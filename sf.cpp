#include "sf.h"

int findANumber(int argc, char** argv)
{
    string s;
    for (int i = 0; i < argc; i++)
    {
        s += " ";
        s += argv[i];
    }

    cout << findANumber(s);
    return 0;
}

int findAllNumbers(int argc, char** argv)
{
    string s;
    for (int i = 0; i < argc; i++)
    {
        s += " ";
        s += argv[i];
    }
    s += " ";
    vector<double> nums;
    int n = findNumbers(s, &nums);
    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += nums.at(i);
    }
    printf("number: %d\tsummary: %lf\taverage:%lf\n", n, sum, sum / n);
    return 0;
}

int replaceStringInSingleFile(int argc, char** argv)
{
    string filename = argv[0];
    int strnum = 0;
    string filename2 = filename;
    if (argc == 4)
    {
        strnum++;
        filename2 = argv[1];
    }
    string oldstr = argv[strnum + 1];
    string newstr = argv[strnum + 2];
    replaceStringInFile(filename, filename2, oldstr, newstr);
    return 0;
}

int replaceAllStringInSingleFile(int argc, char** argv)
{
    string filename = argv[0];
    int strnum = 0;
    string filename2 = filename;
    if (argc == 4)
    {
        strnum++;
        filename2 = argv[1];
    }
    string oldstr = argv[strnum + 1];
    string newstr = argv[strnum + 2];
    replaceAllStringInFile(filename, filename2, oldstr, newstr);
    return 0;
}

int replaceStringInSingleFile2(int argc, char** argv)
{
    string filename = argv[0];
    int strnum = 0;
    string filename2 = filename;
    if (argc == 4)
    {
        strnum++;
        filename2 = argv[1];
    }
    string oldstr = argv[strnum + 1];
    string newstr = readStringFromFile(argv[strnum + 2]);
    replaceStringInFile(filename, filename2, oldstr, newstr);
    return 0;
}

int renamefiles(int argc, char** argv)
{
    string s = readStringFromFile(argv[0]);
    vector<string> s1 = splitString(s, "\r");
    string r;
    for (string t = s1.back(); s1.size() >= 0; t = s1.back(), s1.pop_back())
    {
        int k = 0;
        for (int i = 0; i <= 5; i++)
        {
            char c = t.c_str()[i];
            if (c >= '0' && c <= '9')
            {
                k = k + 1;
                //cout << c;
            }
            if (k > 0 && c == '.')
            {
                string s = t.substr(k + 2, t.length() - k);
                //cout << "rename \""<<t<<"\" \""<<s<<"\"\n";
                //formatAppendString(r, "rename \"%s\" \"%s\"\n", t.c_str(), s.c_str());
                printf("rename \"%s\" \"%s\"\n", t.substr(1, t.length() - 1).c_str(), s.c_str());
                break;
            }
        }
    }
    //cout << r;
    return 0;
}

int setProperty(int argc, char** argv)
{
    string filename = argv[0];
    int strnum = 0;
    string filename2 = filename;
    if (argc == 4)
    {
        strnum++;
        filename2 = argv[1];
    }
    string pro = argv[strnum + 1];
    string content = argv[strnum + 2];

    string f = readStringFromFile(filename);
    int pos = 0;
    while (pos >= 0)
    {
        int pos0 = f.find(pro, pos);
        if (pos0 < 0)
        {
            cout << "cannot find property: " << pro << endl;
            return -1;
        }
        else
        {
            char c1 = ' ';
            if (pos0 > 0)
            { c1 = f.at(pos0 - 1); }
            char c2 = f.at(pos0 + pro.length());
            //cout << "chars:" << pro << c1 << c2 << endl;
            if (!isProChar(c1) && !isProChar(c2))
            {
                pos = pos0;
                //两边都不是合理字符，说明这个是控制字，可以操作
                //这个方法只处理找到的第一个，且不能用于设置数组
                int pos1 = f.find("=", pos) + 1;
                int pos2 = min(min(f.find(",", pos), f.find("\n", pos)), min(f.find(";", pos), f.find("\r", pos)));
                int len = pos2 - pos1;
                if (len > 0)
                {
                    f.erase(pos1, len);
                    f.insert(pos1, content);
                    writeStringToFile(f, filename2);
                    break;
                }
            }
        }
        pos = pos0 + 1;
    }
#ifdef _DEBUG
    cout << "\nresult:\n" << f;
#endif
    return 0;
}
