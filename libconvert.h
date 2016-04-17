#pragma once
#define _USE_MATH_DEFINES 
#include <string>
#include <vector>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

//string functions
std::string readStringFromFile(const std::string &filename);
void writeStringToFile(const std::string &str, const std::string &filename);
int replaceString(std::string &s, const std::string &oldstring, const std::string &newstring, int pos0 = 0);
int replaceAllString(std::string &s, const std::string &oldstring, const std::string &newstring);
void replaceStringInFile(const std::string &oldfilename, const std::string &newfilename, const std::string &oldstring, const std::string &newstring);
void replaceAllStringInFile(const std::string &oldfilename, const std::string &newfilename, const std::string &oldstring, const std::string &newstring);
std::string formatString(const char *format, ...);
void formatAppendString(std::string &str, const char *format, ...);

//math functions

struct vec
{
public:
	double x, y, z;

	vec()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	vec(double _x, double _y, double _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	vec productBase(const vec &vec2)
	{
		return vec(x*vec2.x, y*vec2.y, z*vec2.z);
	}
	vec divideBase(const vec &vec2)
	{
		return vec(x / vec2.x, y / vec2.y, z / vec2.z);
	}
	double dotProduct(const vec &vec2)
	{
		return x*vec2.x + y*vec2.y + z*vec2.z;
	}
	double distance(const vec &vec2)
	{
		return sqrt(pow(x - vec2.x, 2) + pow(y - vec2.y, 2) + pow(z - vec2.z, 2));
	}
	vec operator+(const vec &vec2)
	{
		return vec(x + vec2.x, y + vec2.y, z + vec2.z);
	}
	vec operator-(const vec &vec2)
	{
		return vec(x - vec2.x, y - vec2.y, z - vec2.z);
	}
	vec operator*(double c)
	{
		return vec(x*c, y*c, z*c);
	}
	double length()
	{
		return distance(vec(0, 0, 0));
	}
	double angle(vec vec2)
	{
		return acos(dotProduct(vec2) / length() / vec2.length()) * 180 / M_PI;
	}
	std::string tostring()
	{
		return formatString("%1.10lf %1.10lf %1.10lf", x, y, z);
	}
};


double diff1(double y1, double x1, double y0, double x0);
double diff2(double y2, double x2, double y1, double x1, double y0, double x0);
int findNumbers(const std::string &s, std::vector<double> &data);
std::string findANumber(const std::string &s);
unsigned findTheLast(const std::string &s, const std::string &content);
std::vector<std::string> splitString(std::string str, std::string pattern);
bool isProChar(char c);

