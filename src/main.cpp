#include <opencv2/opencv.hpp>

#include <iostream>
//#include <fstream>
#include <string>
#include <vector>
//#include <bitset>
#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <algorithm>

#include "func.hpp"

using namespace std;
using namespace cv;

struct dirent *drnt;

int main(int argc, char const *argv[])
{
	DIR *dir;
	string path, arg, ext;

	/* Flags */ 
	bool flag[4] = {false,false,false,false}; 													//Has to be same size as number of possible arguments

	/*
	Flag table: 
	0: image path
	1: extension
	2: exit
	3: continue
	*/


	/* Argument Handling */

	cout <<  endl << "Checking arguments to main... " << endl << endl;

	if(argc < 2){
		cout << "Incorrect number of arguments given, try ./gm_detect -help" << endl;			//Check for correct number of arguments
		return -1;
	}

	for(int i = 1; i<argc; i++){																//Reads arguments											

		arg = argv[i];

		if(arg == "-help"){

			cout << "#####################################" << endl;
			cout << "############# Help Menu #############" << endl;
			cout << "#####################################" << endl << endl;

			cout << "Current path: -p $PWD" << endl;
			cout << "Image file extension: -e extension" << endl;

			return -1;
		}
		else if(arg == "-p"){

			path = argv[i+1];
			path = path + "/pic/";

			flag[0] = true;

			cout << "Path set to: " << path << endl;
		}
		else if(arg == "-e"){

			ext = argv[i+1];
			flag[1] = true;

			cout << "Extension set to: " << ext << endl;
		}
	}

	if((flag[0] || flag[1]) == false){															//Check for correct arguments

		cout << "Both current and extension must be set. Use -help for details." << endl;
		return -1;

	}
	else{

		dir = opendir(path.c_str());															//try to open image directory

		if(dir){

			cout << endl << "Opened working directory." << endl;  
	
		}
		else{

			cout << "Failed to open working directory" << endl;
			return -1;

		}

	}


	cout << endl << "Press Enter to coninue..." << endl; 

	//unsigned char c = cin.get();

	//cout << "Char is: " << char(c) << endl;
 
	while(cin.get()!='\n'){}

	return 0;
}
