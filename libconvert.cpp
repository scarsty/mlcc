#include "libconvert.h"

string readStringFromFile(const string &filename)
{
	FILE *fp = fopen(filename.c_str(), "rb");
	if (!fp)
	{
		printf("Can not open file %s\n", filename.c_str());
		return "";
	}
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0, 0);
	char* s = new char[length + 1];
	for (int i = 0; i <= length; s[i++] = '\0');
	fread(s, length, 1, fp);
	string str(s);
	fclose(fp);
	delete[] s;
	return str;
}

void writeStringToFile(const string &str, const string &filename)
{
	FILE *fp = fopen(filename.c_str(), "wb");
	int length = str.length();
	fwrite(str.c_str(), length, 1, fp);
	fclose(fp);
}

int replaceString(string &s, const string &oldstring, const string &newstring)
{
	int pos = s.find(oldstring);
	if (pos >= 0)
	{
		s.erase(pos, oldstring.length());
		s.insert(pos, newstring);
	}
	return pos;
}

void replaceStringInFile(const string &oldfilename, const string &newfilename, const string &oldstring, const string &newstring)
{
	string s = readStringFromFile(oldfilename);
	replaceString(s, oldstring, newstring);
	writeStringToFile(s, newfilename);
}

void replaceAllStringInFile(const string &oldfilename, const string &newfilename, const string &oldstring, const string &newstring)
{
	string s = readStringFromFile(oldfilename);
	while (replaceString(s, oldstring, newstring) >= 0);
	writeStringToFile(s, newfilename);
}

string formatString(const char *format, ...)
{
	char s[1000];
	va_list arg_ptr;
	va_start(arg_ptr, format);
	vsprintf(s, format, arg_ptr);
	va_end(arg_ptr);
	return s;
}

void formatAppendString(string &str, const char *format, ...)
{
	char s[1000];
	va_list arg_ptr;
	va_start(arg_ptr, format);
	vsprintf(s, format, arg_ptr);
	va_end(arg_ptr);
	str += s;
}

double diff1(double y1, double x1, double y0, double x0)
{
	return (y1 - y0) / (x1 - x0);
}

double diff2(double y2, double x2, double y1, double x1, double y0, double x0)
{
	return (diff1(y2, x2, y1, x1) - diff1(y1, x1, y0, x0)) / (x1 - x0);
}

int findNumbers(const string &s, vector<double> &data)
{
	int n = 0;
	string str = "";
	for (int i = 0; i < s.length(); i++)
	{
		char c = s[i];
		if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'E' || c == 'e')
		{
			str += c;
		}
		else
		{
			if (str != "")
			{
				double f = atof(str.c_str());
				data.push_back(f);
				n++;
			}
			str = "";
		}
	}
	return n;
}

string findANumber(const string &s)
{
	bool findPoint = false;
	bool findNumber = false;
	bool findE = false;
	string n;
	for (int i = 0; i < s.length(); i++)
	{
		char c = s[i];
		if (c >= '0' && c <= '9' || c=='-' || c == '.' || c=='e' || c=='E')
		{
			if (c >= '0' && c <= '9' || c == '-')
			{
				findNumber = true;
				n += c;
			}
			if (c == '.')
			{
				if (!findPoint)
					n += c;
				findPoint = true;
			}
			if (c == 'e' || c == 'E')
			{
				if (findNumber && !(findE))
				{
					n += c;
					findE = true;
				}
			}
		}
		else
		{
			if (findNumber)
				break;
		}
	}
	return n;
}

unsigned findTheLast(const string &s, const string &content)
{
	int pos = 0, prepos = 0;
	while (pos >= 0)
	{
		prepos = pos;
		pos = s.find(content, prepos + 1);
		//printf("%d\n",pos);
	}
	return prepos;
}

std::vector<std::string> splitString(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern; //扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}