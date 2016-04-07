#include<stdio.h>
#include<string>
#include<iostream>
#include<stdlib.h>
#include<math.h>
#include<vector>
#include<algorithm>
#include<float.h>
#include"libconvert.h"

using namespace std;

// main functions
void convertXY(int x, int y, int &X, int &Y, int &t);
double findHoppingReal(string &content, int X, int Y, int Z, int u, int v);
string findHoppingRealImag(string &content, int X, int Y, int Z, int u, int v);
int convertHR2Hopping(int argc, char** argv);
int combineHR2Flex(int argc, char** argv);
int calLatticeWithAngle(int argc, char** argv);
int calLatticeWithAngle2(int argc, char** argv);
int calLatticeWithAngleFeSe(int argc, char** argv);
int transGnuplot(int argc, char** argv);
int findANumber(int argc, char** argv);
int setWannierFrozen(int argc, char** argv);
int replaceStringInSingleFile(int argc, char** argv);
int replaceAllStringInSingleFile(int argc, char** argv);
int replaceStringInSingleFile2(int argc, char** argv);
int convertBands(int argc, char** argv);
int convert10to5(int argc, char** argv);
int cgChem(int argc, char** argv);
int renamefiles(int argc, char** argv);
int setProperty(int argc, char** argv);


int main(int argc, char* argv[])
{
	/*if (argc < 4)
	{
		printf("not enough parameters.");
		return 1;
	}*/

	string option;
	
	if (argc > 1)
		option = argv[1];

	int ret = 0;

	if (option == "-h" || argc == 1)
	{
		cout << "\nfunctions of this program:\n";
		cout << " -f\tconvert the hr data into hopping file.\n";
		cout << "   n_hr.dat hopping.out\n";
		//cout << "-c combine the data into the flex input file\n.";
		//cout << "\t-c inputfile newfile \n";
		cout << " -i\tmake the lattice parameter with the Fe-As-Fe bond angle.\n";
		cout << "   angle\n";
		cout << " -g\ttrans the '.gnu' to '.plt' for gnuplot, and add a line for fermi level.\n";
		cout << "   old.gun new.plt fermivalue\n";
		cout << " -n\tfind a number for output in a string.\n";
		cout << "   string\n";
		cout << " -w\tchange set frozen window near fermi level.\n";
		cout << "   seed.win fermivalue -1 1\n";
		cout << " -r\treplace string in a file.\n";
		cout << "   filename oldstring newstring\n";
		cout << " -ra\treplace all string in a file.\n";
		cout << "   filename oldstring newstring\n";
		cout << " -b\tconvert the bands data.\n";
		cout << "   input output fermi weight0 weight1 weight2\n";
		cout << " -5\tconvert 10 bands into 5 bands.\n";
		cout << "   input output edge1 edge2\n";
		cout << " -s\treset a property of a fortran input file.\n";
		cout << "   filename property content\n";
		cout << "\n";
	}

	if (option == "-f")		ret = convertHR2Hopping(argc, argv);
	if (option == "-c")		ret = combineHR2Flex(argc, argv);
	if (option == "-i")		ret = calLatticeWithAngle(argc, argv);
	if (option == "-ifs")		ret = calLatticeWithAngleFeSe(argc, argv);
	if (option == "-2")		ret = calLatticeWithAngle(argc, argv);
	if (option == "-g")		ret = transGnuplot(argc, argv);
	if (option == "-n")		ret = findANumber(argc, argv);
	if (option == "-w")		ret = setWannierFrozen(argc, argv);
	if (option == "-r")		ret = replaceStringInSingleFile(argc, argv);
	if (option == "-ra")		ret = replaceAllStringInSingleFile(argc, argv);
	if (option == "-rf")		ret = replaceStringInSingleFile2(argc, argv);
	if (option == "-b")		ret = convertBands(argc, argv);
	if (option == "-5")		ret = convert10to5(argc, argv);
	if (option == "-cg")		ret = cgChem(argc, argv);
	if (option == "-i2")		ret = calLatticeWithAngle2(argc, argv);
	if (option == "-rename")		ret = renamefiles(argc, argv);
	if (option == "-s")		ret = setProperty(argc, argv);

#ifdef WIN32
	printf("\nPress and key to exit.");
	getchar();
#endif
	return ret;
}


void convertXY(int x, int y, int &X, int &Y, int &t)
{
	t = (x + y) % 2;
	if (t != 0)
	{
		X = (x - y + 1) / 2;
		Y = (x + y - 1) / 2;
	}
	else
	{
		X = (x - y) / 2;
		Y = (x + y) / 2;
	}
	return;
}

double findHoppingReal(string &content, int X, int Y, int Z, int u, int v)
{
	char s[256];
	double r = 0;
	double i = 0;
	sprintf(s, "%5d%5d%5d%5d%5d", X, Y, Z, u, v);
	int pos = content.find(s) + 26;
	r += atof(content.substr(pos, 12).c_str());
	i += atof(content.substr(pos + 12, 12).c_str());
	sprintf(s, "%11.8lfD0", r);
	string str = s;
	return r;
}

string findHoppingRealImag(string &content, int X, int Y, int Z, int u, int v)
{
	char s[256];
	double r = 0;
	double i = 0;
	sprintf(s, "%5d%5d%5d%5d%5d", X, Y, Z, u, v);
	int pos = content.find(s) + 26;
	r += atof(content.substr(pos, 12).c_str());
	i += atof(content.substr(pos + 12, 12).c_str());
	if (X == 0 && Y == 0 && v < 6)
	{
		//r = 0;
		//i = 0;
	}
	sprintf(s, "(%11.7lfD0, %11.7lfD0)", r,i);
	string str = s;
	return str;
}

int convertHR2Hopping(int argc, char** argv)
{
	//string outf1 = "energies.out", outf2 = "hopping.out";
	string infile2 = argv[2];
	string outfile2 = argv[3];
	double fermi = 0;
	if (argc > 4) fermi = atof(argv[4]);
	//cout << fermi;
	char s[256];

	//printf("Input files: %s, %s\n", inf1.c_str(), inf2.c_str());
	//printf("Output files: %s, %s\n\n", outf1.c_str(), outf2.c_str());

	// it seems that the energies of each band should be read from the hr file at (0,0,0)

	/*string s1 = readStringFromFile(inf1);
	string out1 = "";
	for (int i = 1; i <= 5; i++)
	{
	sprintf(s, "Wannier#  %d", i);
	int pos = s1.find(s);
	pos = s1.find("energy:", pos);
	int pos2 = s1.find("eV", pos);
	string sub = s1.substr(pos + 8, pos2 - pos - 8);
	double f = atof(sub.c_str());
	sprintf(s, "%lfd0 ", f);
	out1 += s;
	}
	printf("The enegy of 5 wannier functions have been recorded.\n%s\n\n", out1.c_str());
	*/

	//content of the hopping integral file
	string s2 = readStringFromFile(infile2);

	//find the energies
	/*
	string out1;
	for (int i = 1; i <= 5; i++)
	{
		formatAppendString(out1, "  %d   ", i);
		formatAppendString(out1,"%6.3lfD0\n", findHoppingReal(s2, 0, 0, 0, i, i)-fermi);
	}

	//format hopping
	*/
	string out2;
	/*string out2 = "--------------flux values --------------\n 0.0E0  0.0E0  0.0E0\n\n";
	out2 += " --------------orbital level-------------\n";
	out2 += out1 + "\n";
	*/
	/*
	int x[] = { 1, 1, 2, 2, 2 }, y[] = { 0, 1, 0, 1, 2 };
	//for (int x = -2; x <= 2;x++)
	//for (int y = -2; y <= 2; y++)
	for (int i = 0; i < 5; i++)
	{
		sprintf(s, "------------------- [%d,%d] hopping matrix elements (1 eV) -------------------\n", x[i], y[i]);
		out2 += s;
		int X, Y, Z = 0;
		convertXY(x[i], y[i], X, Y);

		for (int i1 = 1; i1 <= 5; i1++)
		{
			string line;
			for (int i2 = i1; i2 <= 5; i2++)
			{
				line += findHoppingReal(s2, X, Y, Z, i1, i2);
			}
			line += "\n";
			out2 += line;
		}
	}
	*/
	for (int x = -2; x <= 2; x++)
	{
		for (int y = -2; y <= 2; y++)
		{
			sprintf(s, "------------------- [ %d, %d] hopping------------------------\n", x, y);
			out2 += s;
			int X, Y, Z = 0, t = 0;
			convertXY(x, y, X, Y, t);

			for (int u = 1; u <= 5; u++)
			{
				for (int v = 1; v <= 5; v++)
				{
					string line;
					int uu = u, vv = v;
					if (t != 0)
						vv = v + 5;
					formatAppendString(line, "%2d, %2d  ", u, v);
					line += findHoppingRealImag(s2, X, Y, Z, u, vv);
					line += "\n";
					out2 += line;
					//printf("%d, %d, %s", X, Y, line.c_str());
				}
			}
			//out2 += "\n";
		}
	}

	writeStringToFile(out2, outfile2);
	//printf("%s", out2.c_str());
	printf("\n\nSave the results into %s.\n\n", outfile2.c_str());
	return 0;
}

int combineHR2Flex(int argc, char** argv)
{
	replaceStringInFile("n.flex.model", argv[3], "[insert here]", readStringFromFile(argv[2]));
	return 0;
}

int calLatticeWithAngle(int argc, char** argv)
{
	double a = 7.4593, b = a, c = a * 1.7711;
	string Rname = "Na";

	double angle = atof(argv[2])*M_PI / 180;

	if (argc > 3) a = atof(argv[3]);
	if (argc > 4) c = a*atof(argv[4]);
	if (argc > 5) Rname = argv[5];

	vec lattice = vec(a, b, c);
	printf("\nOld lattice parameters are\n%lf %lf %lf\n", lattice.x, lattice.y, lattice.z);

	vec unit = vec(1, 1, 1);
	vec halfz_ = vec(0, 0, 0.5);
	vec halfz = halfz_.productBase(lattice);

	//names with "_" are relative
	vec Fe1_ = vec(0.75, 0.25, 0), Fe2_ = unit - Fe1_, Fe3_ = Fe2_;
	Fe3_.z = 0;
	vec Na1_ = vec(0.25, 0.25, 0.64673), Na2_ = unit - Na1_;
	vec As1_ = vec(0.25, 0.25, 0.20234), As2_ = unit - As1_;

	//the original point is located at z=0.5
	vec Fe1 = Fe1_.productBase(lattice) - halfz;
	vec Fe3 = Fe3_.productBase(lattice) - halfz;
	//vec Fe2_ = Fe2.dotBase(lattice);
	vec As1 = As1_.productBase(lattice);
	double height = As1.z;
	As1 = As1 - halfz;
	//vec As2_ = As2_.dotBase(lattice);
	vec Na1 = Na1_.productBase(lattice) - halfz;
	//vec Na2_ = Na2.dotBase(lattice);

	double bond_length = As1.distance(Fe1);
	printf("Fe-As bond length is %f\n", bond_length);
	printf("Fe-As-Fe angle is %lf", (Fe1_ - As1_).productBase(lattice).angle((Fe3_ - As1_).productBase(lattice)));
	printf(" and %lf\n", 2 * 180 / M_PI*acos(height / bond_length));

	//two kinds of bond angles.

	double half_diag = 2 * bond_length * sin(angle / 2);
	half_diag /= sqrt(2);  //this is the difference between two bond angles.
	height = sqrt(pow(bond_length, 2) - pow(half_diag / sqrt(2), 2));
	//printf("height of As is %lf\n\n", height);

	//vec lattice;
	lattice.x = half_diag*sqrt(2);
	lattice.y = lattice.x;
	//lattice.z = 2 * (height - As1.z);
	As1.z = -lattice.z / 2 + height;

	printf("\nNew lattice parameters are\n%lf %lf %lf\n", lattice.x, lattice.y, lattice.z);

	string pw, wannier90, atom;

	printf("\nInput data for pw.x:\n");
	formatAppendString(pw, "celldm(1) = %1.10lf, ", lattice.x);
	formatAppendString(pw, "celldm(3) = %1.10lf,\n", lattice.z / lattice.x);
	printf("%s", pw.c_str());
	replaceStringInFile("pwbase.scf", "n.scf", "[celldm]", pw);
	replaceStringInFile("pwbase.nscf", "n.nscf", "[celldm]", pw);

	printf("\nInput data for wannier90.x:\n");
	double xf = roundf(lattice.x*1e10) / 1e10;
	double zxf = roundf(lattice.z / lattice.y*1e10) / 1e10;
	double zf = xf*zxf;
	formatAppendString(wannier90, "%1.10lf 0 0\n", lattice.x);
	formatAppendString(wannier90, "0 %1.10lf 0\n", lattice.y);
	formatAppendString(wannier90, "0 0 %1.10lf\n", zf);
	printf("%s", wannier90.c_str());
	replaceStringInFile("wannierbase.win", "n.win", "[celldm]", wannier90);

	printf("\nPositions of atoms:\n");
	formatAppendString(atom, "Fe 0.75 0.25 0\n");
	formatAppendString(atom, "Fe 0.25 0.75 1\n");
	formatAppendString(atom, "Na 0.25 0.25 %1.10lf\n", (Na1.z + lattice.z / 2) / lattice.z);
	formatAppendString(atom, "Na 0.75 0.75 %1.10lf\n", 1 - (Na1.z + lattice.z / 2) / lattice.z);
	formatAppendString(atom, "As 0.25 0.25 %1.10lf\n", height / lattice.z);
	formatAppendString(atom, "As 0.75 0.75 %1.10lf\n", 1 - (As1.z + lattice.z / 2) / lattice.z);
	printf("%s", atom.c_str());
	replaceStringInFile("n.scf", "n.scf", "[atom]", atom);
	replaceStringInFile("n.win", "n.win", "[atom]", atom);

	As1_ = vec(0.25, 0.25, height / lattice.z);
	printf("\nCheck the result:\n");
	printf("New Fe-As bond length is %lf\n", Fe1_.productBase(lattice).distance(As1_.productBase(lattice)));
	printf("New Fe-As-Fe angle is %lf", (Fe1_ - As1_).productBase(lattice).angle((Fe3_ - As1_).productBase(lattice)));
	printf(" and %lf\n", 2 * 180 / M_PI*acos(height / bond_length));

	printf("\nPlease copy n.scf, n.nscf, n.win into your folder.\n");
	return 0;
}

int calLatticeWithAngle2(int argc, char** argv)
{
	double a = 7.626, b = a, c = a * 2.16559;
	string Rname = "Na";

	double angle = atof(argv[2])*M_PI / 180;

	if (argc > 3) a = atof(argv[3]);
	if (argc > 4) c = a*atof(argv[4]);
	if (argc > 5) Rname = argv[5];

	vec lattice = vec(a, b, c);
	vec unit = vec(1, 1, 1);
	vec halfz_ = vec(0, 0, 0.5);
	vec halfz = halfz_.productBase(lattice);

	//names with "_" are relative
	vec Fe1_ = vec(0.75, 0.25, 0), Fe2_ = unit - Fe1_, Fe3_ = Fe2_;
	Fe3_.z = 0;
	vec Na1_ = vec(0.25, 0.25, 0.64673), Na2_ = unit - Na1_;
	vec As1_ = vec(0.25, 0.25, 0.20234), As2_ = unit - As1_;

	//the original point is located at z=0.5
	vec Fe1 = Fe1_.productBase(lattice) - halfz;
	vec Fe3 = Fe3_.productBase(lattice) - halfz;
	//vec Fe2_ = Fe2.dotBase(lattice);
	vec As1 = As1_.productBase(lattice);
	double height = As1.z;
	As1 = As1 - halfz;
	//vec As2_ = As2_.dotBase(lattice);
	vec Na1 = Na1_.productBase(lattice) - halfz;
	//vec Na2_ = Na2.dotBase(lattice);

	double bond_length = As1.distance(Fe1);
	printf("Fe-As bond length is %f\n", bond_length);
	printf("Fe-As-Fe angle is %lf", (Fe1_ - As1_).productBase(lattice).angle((Fe3_ - As1_).productBase(lattice)));
	printf(" and %lf\n", 2 * 180 / M_PI*acos(height / bond_length));

	//two kinds of bond angles.

	double half_diag = 2 * bond_length * sin(angle / 2);
	half_diag /= sqrt(2);  //this is the difference between two bond angles.
	height = sqrt(pow(bond_length, 2) - pow(half_diag / sqrt(2), 2));
	//printf("height of As is %lf\n\n", height);

	//vec lattice;
	lattice.x = half_diag*sqrt(2);
	lattice.y = lattice.x;
	lattice.z = 2 * (height - As1.z);

	printf("\nNew lattice parameters are\n%lf %lf %lf\n", lattice.x, lattice.y, lattice.z);

	string pw, wannier90, atom;

	printf("\nInput data for pw.x:\n");
	formatAppendString(pw, "celldm(1) = %1.4lf, ", lattice.x);
	formatAppendString(pw, "celldm(3) = %1.4lf,\n", lattice.z / lattice.x);
	printf("%s", pw.c_str());
	replaceStringInFile("pwbase.scf", "n.scf", "[celldm]", pw);
	replaceStringInFile("pwbase.nscf", "n.nscf", "[celldm]", pw);

	printf("\nInput data for wannier90.x:\n");
	double xf = roundf(lattice.x*1e4) / 1e4;
	double zxf = roundf(lattice.z / lattice.y*1e4) / 1e4;
	double zf = xf*zxf;
	formatAppendString(wannier90, "%1.4lf 0 0\n", lattice.x);
	formatAppendString(wannier90, "0 %1.4lf 0\n", lattice.y);
	formatAppendString(wannier90, "0 0 %1.8lf\n", zf);
	printf("%s", wannier90.c_str());
	replaceStringInFile("wannierbase.win", "n.win", "[celldm]", wannier90);

	printf("\nPositions of atoms:\n");
	formatAppendString(atom, "Fe 0.75 0.25 0\n");
	formatAppendString(atom, "Fe 0.25 0.75 1\n");
	formatAppendString(atom, "Na 0.25 0.25 %1.5lf\n", (Na1.z + lattice.z / 2) / lattice.z);
	formatAppendString(atom, "Na 0.75 0.75 %1.5lf\n", 1 - (Na1.z + lattice.z / 2) / lattice.z);
	formatAppendString(atom, "As 0.25 0.25 %1.5lf\n", height / lattice.z);
	formatAppendString(atom, "As 0.75 0.75 %1.5lf\n", 1 - (As1.z + lattice.z / 2) / lattice.z);
	printf("%s", atom.c_str());
	replaceStringInFile("n.scf", "n.scf", "[atom]", atom);
	replaceStringInFile("n.win", "n.win", "[atom]", atom);

	As1_ = vec(0.25, 0.25, height / lattice.z);
	printf("\nCheck the result:\n");
	printf("New Fe-As bond length is %lf\n", Fe1_.productBase(lattice).distance(As1_.productBase(lattice)));
	printf("New Fe-As-Fe angle is %lf", (Fe1_ - As1_).productBase(lattice).angle((Fe3_ - As1_).productBase(lattice)));
	printf(" and %lf\n", 2 * 180 / M_PI*acos(height / bond_length));

	printf("\nPlease copy n.scf, n.nscf, n.win into your folder.\n");
	return 0;
}

int calLatticeWithAngleFeSe(int argc, char** argv)
{
	double changeXY = atof(argv[2]);
	int singlelayer = atoi(argv[3]);
	string output = argv[4]; 

	double celldm1 = 7.11481885898746;
	vec a = vec(1, 0, 0)*changeXY;
	vec b = vec(0, 1, 0)*changeXY;
	vec c = vec(0, 0, 1.46560424966799)*(1/changeXY/changeXY);

	vec Fe1 = vec(0.75, 0.25, 0);
	vec Fe2 = vec(0.25, 0.75, 0);
	vec Se1 = vec(0.75, 0.75, 0.243);
	vec Se2 = vec(0.25, 0.25, -0.243);
	
	//if (singlelayer == 2)
	//Se2 = vec(0.25, 0.25, -0.243);	

	if (singlelayer)
	{
		c = c * 10;
		Se1.z /= 10;
		Se2.z /= 10;
	}

	string s = "";
	s = (a * celldm1).tostring() + "\n" + (b * celldm1).tostring() + "\n" + (c * celldm1).tostring() + "\n";
	writeStringToFile(s, output + ".wan90_c.txt");

	cout << s << endl;

	s = a.tostring() + "\n" + b.tostring() + "\n" + c.tostring() + "\n";
	writeStringToFile(s, output + ".pw_c.txt");

	cout << s << endl;

	s = "Fe\t" + Fe1.tostring() + "\n";
	s += "Fe\t" + Fe2.tostring() + "\n";
	s += "Se\t" + Se1.tostring() + "\n";
	s += "Se\t" + Se2.tostring() + "\n";

	writeStringToFile(s, output + ".a.txt");

	cout << s << endl;

	//string s1 = "CELL_PARAMETERS cubic\n";
	//string s2 = "ATOMIC_POSITIONS {crystal}\n";



	return 0;
}


int transGnuplot(int argc, char** argv)
{
	string s = readStringFromFile(argv[2]);
	replaceString(s, "set data style dots", "clear\nunset arrow\nset style data dots");

	int pos0 = s.find("[0:");
	int pos1 = s.find("]", pos0);
	double xmax = atof(s.substr(pos0 + 3, pos1 - pos0 - 3).c_str());
	double f = atof(argv[4]);
	string line = "";
	formatAppendString(line, "set arrow from 0, %f to %f, %f nohead\n", f, xmax, f);
	int pos = s.find("set xtics");
	s.insert(pos, line);
	writeStringToFile(s, argv[3]);
	return 0;
}

int findANumber(int argc, char** argv)
{
	string s;
	for (int i = 2; i < argc; i++)
	{
		s += " ";
		s += argv[i];
	}

	cout << findANumber(s);
	return 0;
}



int setWannierFrozen(int argc, char** argv)
{
	double f = atof(argv[3]);
	double dif0 = atof(argv[4]);
	double dif1 = atof(argv[5]);
	string filename = argv[2];
	replaceStringInFile(filename, filename, "[minfrozen]", formatString("%lf", f + dif0));
	replaceStringInFile(filename, filename, "[maxfrozen]", formatString("%lf", f + dif1));
	return 0;
}

int replaceStringInSingleFile(int argc, char** argv)
{
	string filename = argv[2];
	int strnum = 2;
	string filename2 = filename;
	if (argc == 6)
	{
		strnum++;
		filename2 = argv[3];
	}
	string oldstr = argv[strnum+1];
	string newstr = argv[strnum+2];
	replaceStringInFile(filename, filename2, oldstr, newstr);
	return 0;
}

int replaceAllStringInSingleFile(int argc, char** argv)
{
	string filename = argv[2];
	int strnum = 2;
	string filename2 = filename;
	if (argc == 6)
	{
		strnum++;
		filename2 = argv[3];
	}
	string oldstr = argv[strnum + 1];
	string newstr = argv[strnum + 2];
	replaceAllStringInFile(filename, filename2, oldstr, newstr);
	return 0;
}

int replaceStringInSingleFile2(int argc, char** argv)
{
	string filename = argv[2];
	int strnum = 2;
	string filename2 = filename;
	if (argc == 6)
	{
		strnum++;
		filename2 = argv[3];
	}
	string oldstr = argv[strnum + 1];
	string newstr = readStringFromFile(argv[strnum + 2]);
	replaceStringInFile(filename, filename2, oldstr, newstr);
	return 0;
}

int convertBands(int argc, char** argv)
{
	string s = readStringFromFile(argv[2]);
	double w0 = 1, w1 = 0, w2 = 0, w3 = 0, fermi = atof(argv[4]);
	if (argc >= 6) w0 = atof(argv[5]);
	if (argc >= 7) w1 = atof(argv[6]);
	if (argc >= 8) w2 = atof(argv[7]);
	//double w0 = atof(argv[4]);
	vector<double> numbers, x0, y0;
	int totalCount = findNumbers(s, numbers);
	cout << "Total numbers is " << totalCount << endl;
	//divide x and y
	for (int i = 0; i < numbers.size(); i++)
	{
		if (i % 2 == 0)
			x0.push_back(numbers.at(i));
		else
			y0.push_back(numbers.at(i));
	}
	//find the real count of x
	int xCount = 0;
	for (int i = 1; i < x0.size(); i++)
	{
		bool finded = false;
		for (int j = 0; j < i; j++)
		{
			if (fabs(x0.at(i) - x0.at(j)) < 1e-5)
			{
				xCount = i;
				finded = true;
				break;
			}
		}
		if (finded)
			break;
	}
	cout << "Count of X is " << xCount << endl;
	//divide y0 into vectors by the same x
	int groupCount = totalCount / 2 / xCount;
	cout << "Count of Group is " << groupCount << endl;
	printf("Weights are %lf, %lf, %lf", w0, w1, w2);

	vector<double> x;
	for (int i = 0; i < xCount; i++)
	{
		x.push_back(x0.at(i));
	}

	sort(x.begin(), x.end());
	vector<vector<double> > y1;
	y1.resize(xCount);
	for (int i = 0; i < y0.size(); i++)
	{
		double xf = x0[i];
		for (int j = 0; j < xCount; j++)
		{
			if (fabs(xf - x[j]) < 1e-5)
			{
				y1[j].push_back(y0[i]);
				break;
			}
		}
	}
	vector<vector<double> > y;
	//divide
	y.resize(groupCount);
	for (int i = 0; i < groupCount; i++)
	{
		y[i].push_back(y1[0][i]);
		y[i].push_back(y1[1][i]);
		y[i].push_back(y1[2][i]);
		double deltax = x[1] - x[0];
		for (int j = y[i].size(); j < xCount; j++)
		{
			int mink = -1;
			double mindif = DBL_MAX;
			for (int k = 0; k < y1[j].size(); k++)
			{
				double d0, d1;
				double l0 = 0, l1 = 0, l2 = 0, l3 = 0;
				//only delta y
				d1 = y1[j][k] - y[i][j - 1];
				l0 = pow(d1, 2);
				//1 order differential
				d0 = diff1(y[i][j - 2], x[j - 2], y[i][j - 1], x[j - 1]);
				d1 = diff1(y[i][j - 1], x[j - 1], y1[j][k], x[j]);
				l1 = pow((d0 - d1)*deltax, 2);
				//2 order differential
				d0 = diff2(y[i][j - 3], x[j - 3], y[i][j - 2], x[j - 2], y[i][j - 1], x[j - 1]);
				d1 = diff2(y[i][j - 2], x[j - 2], y[i][j - 1], x[j - 1], y1[j][k], x[j]);
				l2 = pow((d0 - d1)*deltax*deltax, 2);
				//try the weights of them
				double dif = w0*l0 + w1*l1 + w2*l2;
				if (dif < mindif)
				{
					mindif = dif;
					mink = k;
				}
			}
			y[i].push_back(y1[j][mink]);
			y1[j].erase(y1[j].begin() + mink);
		}
	}
	string out;
	for (int i = 0; i < xCount; i++)
	{
		formatAppendString(out, "%1.4lf", x[i]);
		for (int j = 0; j < groupCount; j++)
		{
			formatAppendString(out, "\t%1.4lf", y[j][i]-fermi);
		}
		formatAppendString(out, "\n");
	}
	writeStringToFile(out, argv[3]);
	return 0;
}

int convert10to5(int argc, char** argv)
{
	string s = readStringFromFile(argv[2]);
	vector<double> numbers, x0, y0[10];
	//region where 10 overlap to 5
	double x1 = atof(argv[4]);
	double x2 = atof(argv[5]);
	int totalCount = findNumbers(s, numbers);
	cout << "Total numbers is " << totalCount << endl;
	//divide x and y
	for (int i = 0; i < numbers.size(); i++)
	{
		if (i % 11 == 0)
			x0.push_back(numbers.at(i));
		else
			y0[i%11-1].push_back(numbers.at(i));
	}

	bool used[10][2] = { false };
	int first[] = { 0, 1, 3, 9, 6};
	//used[0] = true
	vector<double> y[5];
	int xCount = totalCount / 11;
	
	for (int i = 0; i < 5; i++)
	{
		int yindex1 = -1, yindex2 = -1, yindex3 = -1, yindex4 = -1;
		double mindif = 1e5;
		//find the first part in unused
		/*for (int j = 0; j < 10; j++)
		{
			if (!used[j][0])
			{
				yindex1 = j;
				used[yindex1][0] = true;
				break;
			}
		}*/
		int j = first[i];
		if (!used[j][0])
		{
			yindex1 = j;
			used[yindex1][0] = true;
		}
		if (yindex1 < 0)
		{
			cout << "cannot connect!";
			return 0;
		}
		cout << "1st part: " << yindex1 << ", ";
		int k = 0;
		while (x0[k] < x1)
		{
			y[i].push_back(y0[yindex1][k]);
			k++;
		}
		mindif = 1e5;
		for (int j = 0; j < 10; j++)
		{
			double dif = fabs(y0[yindex1][k] - y0[j][k]);
			if (!used[j][0] &&  dif < mindif)
			{
				mindif = dif;
				yindex2 = j;
			}
		}
		used[yindex2][0] = true;
		cout << "2nd part: " << yindex2 << ", ";
		for (k--; k >= 0; k--)
		{
			y[i].push_back(y0[yindex2][k]);
		}

		k = xCount - 1;
		mindif = 1e5;
		for (int j = 0; j < 10; j++)
		{
			double dif = fabs(y0[yindex2][0] - y0[j][k]);
			if (!used[j][1] && dif < mindif)
			{
				mindif = dif;
				yindex3 = j;
			}
		}
		k--;
		used[yindex3][1] = true;
		cout << "3rd part: " << yindex3 << ", ";
		while (x0[k] > x2)
		{
			y[i].push_back(y0[yindex3][k]);
			k--;
		}
		mindif = 1e5;
		for (int j = 0; j < 10; j++)
		{
			double dif = fabs(y0[yindex3][k] - y0[j][k]);
			if (!used[j][1] && dif < mindif)
			{
				mindif = dif;
				yindex4 = j;
			}
		}
		used[yindex4][1] = true;
		cout << "4th part: " << yindex4 << ". " << endl;
		//k++;
		k--;
		while (k < xCount)
		{
			y[i].push_back(y0[yindex4][k]);
			k++;
		}
	}
	cout << "Convert ended! The count of row is "<< y[4].size() << endl;
	string out;
	for (int i = 0; i < y[0].size(); i++)
	{
		for (int j = 0; j < 5; j++)
		{
			formatAppendString(out, "%1.5lf\t", y[j][i]);
		}
		formatAppendString(out, "\n");
	}
	writeStringToFile(out, argv[3]);

	return 0;
}

double findFinalEnergy(const string& str)
{
	double ryd = 13.60569253;
	int pos = findTheLast(str, "!    total energy");
	//find the last
	int posln = str.find("Ry", pos);

	double energy = atof(findANumber(str.substr(pos, posln - pos + 1)).c_str());
	return energy;
}

string findAtomFinal(const string& s)
{
	int pos = findTheLast(s, "Begin final coordinates");
	pos = s.find("ATOMIC_POSITIONS", pos);
	pos = s.find("\n", pos);
	int posln = s.find("End final coordinates", pos);
	string str = s.substr(pos + 1, posln - pos - 1);
	return str;
}

int cgChem(int argc, char** argv)
{
	double ryd = 13.60569253;
	string option = argv[2];
	int beginop = 3;

	//pure c - only one time
	if (option == "pc")
	{
		double x = atof(argv[3]);
		double y = atof(argv[4]);
		string prefix = formatString("c-%1.3lf-%1.3lf", x, y);
		string a1 = formatString("%1.12lf", 3.0*x);
		string b1 = formatString("%1.12lf", -1.5 * x);
		string b2 = formatString("%1.12lf", 1.5*sqrt(3.0) * y);

		string filename = prefix + ".scf";
		replaceStringInFile("c.scf.base", filename, "[prefix]", prefix);
		replaceStringInFile(filename, filename, "[a1]", a1);
		replaceStringInFile(filename, filename, "[b1]", b1);
		replaceStringInFile(filename, filename, "[b2]", b2);
		cout << filename;
	}

	if (option == "sg" || option == "re" || option == "re2" || option == "scf2")
	{
		
		string name = argv[3];
		string weight = argv[4];
		string pp = argv[5];
		string charge = argv[6];
		double x=0, y=0;
		if (argc > 8)
		{
			x = atof(argv[7]);
			y = atof(argv[8]);
		}

		string a1 = formatString("%1.12lf", 3.0*x);
		string b1 = formatString("%1.12lf", -1.5 * x);
		string b2 = formatString("%1.12lf", 1.5*sqrt(3.0) * y);
		
		string atom = name + "    " + weight + "    " + pp;

		string filename, prefix;

		if (option == "sg")
		{
			prefix = formatString("%s-%1.3lf-%1.3lf", name.c_str(), x, y);
			filename = prefix + ".scf";
			
			replaceStringInFile("scf0.base", filename, "[prefix]", prefix);
		}

		if (option == "re")
		{
			string position = argv[7];

			if (position[0] >= 'a' && position[0] <= 'z')
			{
				position[0] += 'A' - 'a';
			}

			string pos = "";
			if (position == "T"){ pos = "   0   0   0.1"; };
			if (position == "B"){ pos = "   0.0555555555   0.111111111  0.1"; };
			if (position == "H"){ pos = "   0.222222222  0.444444444   0.1"; };

			prefix = name + position;

			string atompos = name + "    " + pos;

			filename = name + "-" + position + ".relax";
			replaceStringInFile("relax.base", filename, "[prefix]", prefix);
			replaceStringInFile(filename, filename, "[atompos]", atompos);
		}

		if (option == "re2")
		{
			string filerelax = argv[9];

			prefix = formatString("%s-c-%1.3lf-%1.3lf", name.c_str(), x, y);

			filename = prefix + ".relax";

			string atompos = findAtomFinal(readStringFromFile(filerelax));

			replaceStringInFile("strainrelax.base", filename, "[prefix]", prefix);
			replaceStringInFile(filename, filename, "[atompos]", atompos);
		}

		if (option == "scf2")
		{
			prefix = formatString("%s-c-%1.3lf-%1.3lf", name.c_str(), x, y);
			string filerelax = prefix + ".relax.out";
			filename = prefix + ".scf";

			string atompos = findAtomFinal(readStringFromFile(filerelax));

			replaceStringInFile("strainscf.base", filename, "[prefix]", prefix);
			replaceStringInFile(filename, filename, "[atompos]", atompos);
		}

		if (option == "re2b")
		{
			string filerelax = argv[9];

			prefix = formatString("%s-cB2-%1.3lf-%1.3lf", name.c_str(), x, y);

			filename = prefix + ".relax";

			string atompos = findAtomFinal(readStringFromFile(filerelax));

			replaceStringInFile("strainrelax.base", filename, "[prefix]", prefix);
			replaceStringInFile(filename, filename, "[atompos]", atompos);

			replaceStringInFile(filename, filename, "0.055555555   0.111111111", "0.055555555   0.277777778");
		}

		if (option == "scf2b")
		{
			prefix = formatString("%s-cB2-%1.3lf-%1.3lf", name.c_str(), x, y);
			string filerelax = prefix + ".relax.out";
			filename = prefix + ".scf";

			string atompos = findAtomFinal(readStringFromFile(filerelax));

			replaceStringInFile("strainscf.base", filename, "[prefix]", prefix);
			replaceStringInFile(filename, filename, "[atompos]", atompos);
		}

		replaceStringInFile(filename, filename, "[a1]", a1);
		replaceStringInFile(filename, filename, "[b1]", b1);
		replaceStringInFile(filename, filename, "[b2]", b2);
		replaceStringInFile(filename, filename, "[charge]", charge);
		replaceStringInFile(filename, filename, "[atom]", atom);
		replaceStringInFile(filename, filename, "[atomname]", name);
		cout << filename;
	}




	if (option == "fe")
	{
		printf("%1.8lf", findFinalEnergy(readStringFromFile(argv[3])));
	}

	if (option == "fm")
	{
		double e[3];
		for (int i = 0; i < 3; i++)
		{
			e[i] = findFinalEnergy(readStringFromFile(argv[3 + i]));
			//cout << e[i];
		}

		int index = 0;
		double min = e[0];
		for (int i = 1; i < 3; i++)
		{
			if (e[i] < min)
			{
				min = e[i];
				index = i;
			}
		}
		//cout << argv[3 + index] << " " << min << "\n";

		string filename = argv[3 + index];
		
		string pos;
		do
		{
			pos = "-T.";
			if (filename.find(pos) != string::npos) break;
			pos = "-H.";
			if (filename.find(pos) != string::npos) break;
			pos = "-B.";
			if (filename.find(pos) != string::npos) break;
		} 
		while (false);

		cout << pos[1];
	}
	return 0;
}

int renamefiles(int argc, char** argv)
{
	string s = readStringFromFile(argv[2]);
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
				string s = t.substr(k+2, t.length() - k);
				//cout << "rename \""<<t<<"\" \""<<s<<"\"\n";
				//formatAppendString(r, "rename \"%s\" \"%s\"\n", t.c_str(), s.c_str());
				printf("rename \"%s\" \"%s\"\n", t.substr(1,t.length()-1).c_str(), s.c_str());
				break;
			}
		}
			}
	//cout << r;
	return 0;
}

int setProperty(int argc, char** argv)
{
	string filename = argv[2];
	int strnum = 2;
	string filename2 = filename;
	if (argc == 6)
	{
		strnum++;
		filename2 = argv[3];
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
				c1 = f.at(pos0 - 1);
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
	cout << "\nresult:\n"<<f;
#endif
	return 0;
}

