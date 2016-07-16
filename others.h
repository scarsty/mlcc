#pragma once
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <float.h>

#include "libconvert.h"

using namespace std;

void convertXY(int x, int y, int& X, int& Y, int& t);
double findHoppingReal(const string& content, int X, int Y, int Z, int u, int v);
string findHoppingRealImag(const string& content, int X, int Y, int Z, int u, int v);
int convertHR2Hopping(int argc, char** argv);
int combineHR2Flex(int argc, char** argv);
int calLatticeWithAngle(int argc, char** argv);
int calLatticeWithAngle2(int argc, char** argv);
int calLatticeWithAngleFeSe(int argc, char** argv);
int transGnuplot(int argc, char** argv);
int setWannierFrozen(int argc, char** argv);
int convertBands(int argc, char** argv);
int convert10to5(int argc, char** argv);
int cgChem(int argc, char** argv);


