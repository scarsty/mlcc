#pragma once
#include "PotConv.h"
#include "ft2build.h"

#include "freetype/freetype.h"
#include "opencv2/opencv.hpp"
#include <string>

class DrawStringFT
{
private:
    FT_Library library{ nullptr };
    FT_Face face{ nullptr };
    FT_GlyphSlot slot{ nullptr };
    //FT_Matrix matrix{nullptr};
    FT_Vector pen{ 0, 0 };
    int fontsize_ = 0;

public:
    DrawStringFT()
    {
        FT_Init_FreeType(&library);
    }

    ~DrawStringFT()
    {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void openFont(const std::string& fontname, int fontsize)
    {
        if (FT_New_Face(library, fontname.c_str(), 0, &face))
        {
            fprintf(stderr, "Cannot open font file: %s \n", fontname.c_str());
        }
        FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        FT_Set_Char_Size(face, 64 * fontsize, 0, 100, 0);
        slot = face->glyph;
        fontsize_ = fontsize;
    }

    int drawString(std::string text, cv::Mat& mat, int x, int y, cv::Vec3b color, int fusion = 0, const std::string& code = "utf-8")
    {
        pen.x = 0;
        pen.y = 0;
        int width = 0;
        auto text1 = PotConv::conv(text, code, "utf-16le");
        for (int n = 0; n < text1.size(); n += 2)
        {
            FT_Set_Transform(face, nullptr, &pen);
            wchar_t v = *(wchar_t*)&text1[n];
            FT_Load_Char(face, v, FT_LOAD_RENDER);
            draw_bitmap(&slot->bitmap, slot->bitmap_left + x, fontsize_ * 1 - slot->bitmap_top + y, mat, color, fusion);
            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
            width = slot->bitmap.width + slot->bitmap_left;
        }
        return width;
    }

private:
    void draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, cv::Mat& image, cv::Vec3b color, int fusion)
    {
        FT_Int x_max = x + bitmap->width;
        FT_Int y_max = y + bitmap->rows;
        for (int i = x, p = 0; i < x_max; i++, p++)
        {
            for (int j = y, q = 0; j < y_max; j++, q++)
            {
                if (i < 0 || j < 0 || i >= image.cols || j >= image.rows)
                {
                    continue;
                }
                auto& v0 = bitmap->buffer[q * bitmap->width + p];
                if (v0 == 0)
                {
                    continue;
                }
                if (image.channels() == 1)
                {
                    auto& a = image.data[j * image.step + i];
                    auto v = v0;
                    if (fusion == 0)
                    {
                        a = v;
                    }
                    else if (fusion == -1)
                    {
                        a = std::min(v, a);
                    }
                    else if (fusion == 1)
                    {
                        a = std::max(v, a);
                    }
                }
                else if (image.channels() == 3)
                {
                    for (int c = 0; c < image.channels(); c++)
                    {
                        auto& a = image.data[j * image.step + i * 3 + c];
                        auto v = uchar(v0 * color[c] / 255);
                        if (fusion == 0)
                        {
                            a = v;
                        }
                        else if (fusion == -1)
                        {
                            a = std::min(v, a);
                        }
                        else if (fusion == 1)
                        {
                            a = std::max(v, a);
                        }
                    }
                }
            }
        }
    }
};
