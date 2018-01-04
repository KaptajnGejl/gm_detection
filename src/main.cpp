#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
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
	clrscr();
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
	while(cin.get()!='\n'){}


	/* End of argument handling */ 


	/* Main loop */ 

	cout <<  "Searching path for images with extension " << ext << "..." << endl << endl;

	while((drnt = readdir(dir)) && !flag[2]){

		string fname = drnt->d_name;

		if(fname.find(ext,(fname.length()-ext.length())) != string::npos){

			clrscr();
			cout << "Found image: " << fname << endl;

			Mat img = imread(path+fname,CV_LOAD_IMAGE_GRAYSCALE);
			namedWindow(fname,WINDOW_AUTOSIZE);
			moveWindow(fname,1280-img.size().width,20);
			waitKey(100);

			string cmd = "wmctrl -a " + fname + " 2>/dev/null";
			if(system(cmd.c_str()));
			
			flag[3] = false;

			while(!flag[3]){

				imshow(fname,img);	 

				unsigned char thr = optimal_threshold(hist(img,true));

				cout << "Threshold set to: " << int(thr) << endl;
				cout << "Thresholding image..." << endl << endl;

				Mat img_thr; 
				threshold(img,img_thr,thr,255,0);

				namedWindow("Thresholded",WINDOW_AUTOSIZE);
				moveWindow("Thresholded",1280-img_thr.size().width,53+img.size().height);
				imshow("Thresholded",img_thr);

				cout << "Detecting corners on thresholded image..." << endl << endl;

				vector<Point> corners;
				//cornerHarris(img_thr,img_cor,7,5,0.05,BORDER_DEFAULT);
				goodFeaturesToTrack(img_thr,corners,0,0.1,20,noArray(),3,true,0.04);

				Mat img_cor;
				cvtColor(img,img_cor,COLOR_GRAY2BGR,0);
				for(unsigned int i = 0; i<corners.size();i++){
					circle(img_cor,corners[i],3,Scalar(0,255,0),-1,0);
				}

				namedWindow("Corners",WINDOW_AUTOSIZE);
				moveWindow("Corners",1280-img_thr.size().width,2*(53+img.size().height));
				imshow("Corners",img_cor);

				cout << "Press Enter to continue or 'q' to quit..." << endl;  
				char key = waitKey(0);

				switch(key){
					case 'q':
						flag[2] = true;
						flag[3] = true;
					break;

					case 13: //Enter key
						flag[3] = true;
					break;
				}				
			}
			destroyAllWindows();
		}
	}

	destroyAllWindows();
	cout << "Succesfully exited." << endl;
	return 0;
}
