#include <stdio.h>
#include <string>
#include <iostream>

#include "libconvert.h"
#include "sf.h"
#include "others.h"

using namespace std;

// main functions


int main(int argc, char* argv[])
{
    /*if (argc < 4)
    {
        printf("not enough parameters.");
        return 1;
    }*/

    string option;

    if (argc > 1)
    { option = argv[1]; }

    int ret = 0;

    if (option == "-h" || argc == 1)
    {
        cout << "\nfunctions of this program:\n";
        cout << "\npart 1: string and file\n";
        cout << " -n\tfind a number for output in a string.\n";
        cout << "   string\n";
        cout << " -na\tfind all number for summary and average.\n";
        cout << "   string\n";
        cout << " -r\treplace string in a file.\n";
        cout << "   filename oldstring newstring\n";
        cout << " -ra\treplace all string in a file.\n";
        cout << "   filename oldstring newstring\n";
        cout << " -s\treset a property of a fortran input file.\n";
        cout << "   filename property content\n";

        cout << "\npart 2: others\n";
        cout << " -f\tconvert the hr data into hopping file.\n";
        cout << "   n_hr.dat hopping.out\n";
        //cout << "-c combine the data into the flex input file\n.";
        //cout << "\t-c inputfile newfile \n";
        cout << " -i\tmake the lattice parameter with the Fe-As-Fe bond angle.\n";
        cout << "   angle\n";
        cout << " -g\ttrans the '.gnu' to '.plt' for gnuplot, and add a line for fermi level.\n";
        cout << "   old.gun new.plt fermivalue\n";
        cout << " -w\tchange set frozen window near fermi level.\n";
        cout << "   seed.win fermivalue -1 1\n";
        cout << " -b\tconvert the bands data.\n";
        cout << "   input output fermi weight0 weight1 weight2\n";
        cout << " -5\tconvert 10 bands into 5 bands.\n";
        cout << "   input output edge1 edge2\n";

        cout << "\n";
    }

    argc -= 2;
    argv += 2;

    if (option == "-n")
    { ret = findANumber(argc, argv); }
    if (option == "-na")
    { ret = findAllNumbers(argc, argv); }
    if (option == "-r")
    { ret = replaceStringInSingleFile(argc, argv); }
    if (option == "-ra")
    { ret = replaceAllStringInSingleFile(argc, argv); }
    if (option == "-rf")
    { ret = replaceStringInSingleFile2(argc, argv); }
    if (option == "-rename")
    { ret = renamefiles(argc, argv); }
    if (option == "-s")
    { ret = setProperty(argc, argv); }

    if (option == "-f")
    { ret = convertHR2Hopping(argc, argv); }
    if (option == "-c")
    { ret = combineHR2Flex(argc, argv); }
    if (option == "-i")
    { ret = calLatticeWithAngle(argc, argv); }
    if (option == "-ifs")
    { ret = calLatticeWithAngleFeSe(argc, argv); }
    if (option == "-2")
    { ret = calLatticeWithAngle(argc, argv); }
    if (option == "-g")
    { ret = transGnuplot(argc, argv); }
    if (option == "-w")
    { ret = setWannierFrozen(argc, argv); }
    if (option == "-b")
    { ret = convertBands(argc, argv); }
    if (option == "-5")
    { ret = convert10to5(argc, argv); }
    if (option == "-cg")
    { ret = cgChem(argc, argv); }
    if (option == "-i2")
    { ret = calLatticeWithAngle2(argc, argv); }


#ifdef WIN32
    printf("\nPress and key to exit.");
    getchar();
#endif
    return ret;
}
