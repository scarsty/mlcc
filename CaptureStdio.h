#pragma once

// CaptureStdio - Reliably capture stdout from function calls (e.g. DLL) via fd-level redirection.
//
// Mechanism: pipe + dup2 + reader thread
//   1. dup(1) saves original stdout fd
//   2. dup2(pipe_write, 1) redirects stdout to pipe
//   3. Reader thread reads pipe, optionally tees to original console fd (preserving colors)
//   4. After func() returns, restores stdout, reader drains remaining data
//
// Features:
//   - Thread-safe (global mutex serializes captures)
//   - Tee mode: real-time console output with color preservation
//   - No fixed buffer size limit (pipe-based, dynamically accumulated)
//   - Cross-platform (Windows / POSIX)
//
// Console color preservation:
//   Windows console colors are set via SetConsoleTextAttribute on the console HANDLE,
//   which is independent of C runtime fd redirection (_dup2). The reader thread writes
//   captured data back to the saved console fd, so the current text attributes apply.

#include <cstdio>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#define NOMINMAX
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

class CaptureStdio
{
public:
    /// Execute func(), capture its stdout output, return as string.
    /// @param func  The function to execute while capturing stdout
    /// @param tee   If true, output is also written to console in real-time (color-preserving)
    /// @return      Captured stdout content (empty if stdout is already redirected to pipe, or pipe failure)
    static std::string capture(std::function<void()> func, bool tee = true)
    {
        std::lock_guard<std::mutex> lock(mutex());

        std::string captured;

        // Create pipe
        int pipe_fds[2];
#ifdef _WIN32
        if (_pipe(pipe_fds, 65536, _O_BINARY) != 0)
#else
        if (pipe(pipe_fds) != 0)
#endif
        {
            func();
            return {};
        }

        int read_fd = pipe_fds[0];
        int write_fd = pipe_fds[1];

        // Save original stdout fd
#ifdef _WIN32
        int saved_fd = _dup(1);
#else
        int saved_fd = dup(STDOUT_FILENO);
#endif
        if (saved_fd == -1)
        {
            close_fd(read_fd);
            close_fd(write_fd);
            func();
            return {};
        }

        fflush(stdout);

        // Redirect fd 1 to pipe write end
#ifdef _WIN32
        _dup2(write_fd, 1);
#else
        dup2(write_fd, STDOUT_FILENO);
#endif
        close_fd(write_fd);    // fd 1 now holds the only reference to pipe write end

        // Reader thread: drain pipe -> accumulate string, optionally tee to original console fd
        // NOTE: saved_fd must NOT be closed until after reader.join()!
        std::thread reader([&captured, read_fd, saved_fd, tee]()
        {
            char buf[4096];
            for (;;)
            {
#ifdef _WIN32
                int n = _read(read_fd, buf, sizeof(buf));
#else
                ssize_t n = read(read_fd, buf, sizeof(buf));
#endif
                if (n <= 0) break;
                captured.append(buf, static_cast<size_t>(n));
                if (tee)
                {
#ifdef _WIN32
                    _write(saved_fd, buf, static_cast<unsigned>(n));
#else
                    (void)write(saved_fd, buf, static_cast<size_t>(n));
#endif
                }
            }
        });

        // Execute the target function
        func();
        fflush(stdout);

        // Restore stdout - _dup2 closes fd 1 (pipe write end), reader sees EOF
#ifdef _WIN32
        _dup2(saved_fd, 1);
#else
        dup2(saved_fd, STDOUT_FILENO);
#endif

        // IMPORTANT: join BEFORE closing saved_fd; reader thread uses saved_fd for tee writes
        reader.join();
        close_fd(read_fd);

#ifdef _WIN32
        _close(saved_fd);
#else
        close(saved_fd);
#endif

        return captured;
    }

    /// Returns true if stdout is currently redirected to a pipe (OS-level check).
    /// Works across DLL boundaries unlike thread_local flags.
    static bool isRedirected()
    {
#ifdef _WIN32
        return GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_PIPE;
#else
        return !isatty(STDOUT_FILENO);
#endif
    }

    /// Check if stdout is connected to a console / terminal
    static bool isConsole()
    {
#ifdef _WIN32
        return _isatty(_fileno(stdout)) != 0;
#else
        return isatty(fileno(stdout)) != 0;
#endif
    }

private:
    static std::mutex& mutex()
    {
        static std::mutex m;
        return m;
    }

    static void close_fd(int fd)
    {
#ifdef _WIN32
        _close(fd);
#else
        close(fd);
#endif
    }
};