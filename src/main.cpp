#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <algorithm>
#include <ctime>

#include "func.hpp"

using namespace std;
using namespace cv;

struct dirent *drnt;

int main(int argc, char const *argv[])
{
	DIR *dir;
	string path, arg, ext;
	Mat img, img_blr, img_thr, img_cor;
	uint16_t success = 0, total = 0;
	unsigned char thr = 0;
	clock_t start = time(0), end = time(0);

	/* Flags */ 
	bool flag[5] = {false,false,false,false,false}; 													//Has to be same size as number of possible arguments

	/*
	Flag table: 
	0: image path
	1: extension
	2: exit
	3: continue
	4: manual
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
			cout << "Image file extension: -e .extension" << endl;
			cout << "Manual mode: -m" << endl;

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
		else if(arg == "-m"){

			flag[4] = true;

			cout << "Manual mode selected." << endl;
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

	if(!flag[4]) start = time(0);


	/* End of argument handling */ 


	/* Main loop */ 

	cout <<  "Searching path for images with extension " << ext << "..." << endl << endl;

	while((drnt = readdir(dir)) && !flag[2]){

		string fname = drnt->d_name;

		if(fname.find(ext,(fname.length()-ext.length())) != string::npos){

			clrscr();
			cout << "Found image: " << fname << endl;
			cout << "No. of images found: " << total << endl;
			
			img = imread(path+fname,CV_LOAD_IMAGE_GRAYSCALE);

			if(flag[4]){	
				namedWindow(fname,WINDOW_AUTOSIZE);
				moveWindow(fname,1280-img.size().width,20);
				waitKey(100);


				string cmd = "wmctrl -a " + fname + " 2>/dev/null";
				if(system(cmd.c_str()));	
			}

			flag[3] = false;

			while(!flag[3]){
				total ++;

				if(flag[4])imshow(fname,img);	 
				
				/* Blurring */

				if(flag[4]) cout << "Blurring Image..." << endl << endl;
				GaussianBlur(img,img_blr,Size(7,7),0,0);

				/* Thresholding*/

				if(flag[4]) thr = optimal_threshold(hist(img_blr,true));
				else if(!flag[4]) thr = optimal_threshold(hist(img_blr,false));

				if(flag[4]) cout << "Threshold set to: " << int(thr) << endl;
				if(flag[4]) cout << "Thresholding image..." << endl << endl;
				threshold(img_blr,img_thr,thr,255,0);

				if(flag[4]){
					namedWindow("Thresholded",WINDOW_AUTOSIZE);
					moveWindow("Thresholded",1280-img_thr.size().width,53+img.size().height);
					imshow("Thresholded",img_thr);
				}
				/* Corner Detection */

				if(flag[4]) cout << "Detecting corners on thresholded image..." << endl << endl;

				vector<Point> points;
				goodFeaturesToTrack(img_thr,points,0,0.2,10,noArray(),11,true,0.04);


				/* Draw found corners */
				cvtColor(img,img_cor,COLOR_GRAY2BGR,0);		
				for(unsigned int i = 0; i<points.size();i++){
					circle(img_cor,points[i],3,Scalar(0,140,255),-1,0);
				}


				/* Find center of cross, and draw it */
				if(points.size()>0){
					vector<corner> corners = cvtCorner(points);

	 				corner center = global_center(corners);
	 				circle(img_cor,Point(center.pos.x,center.pos.y),3,Scalar(0,0,255),-1,0);

					corner c_center = cross_center(img_cor, corners, flag[4]);

					if(c_center.pos.x!=0 && c_center.pos.y != 0){
						circle(img_cor,Point(c_center.pos.x,c_center.pos.y),3,Scalar(255,0,0),-1,0);
						success++;
					}	

					if(flag[4]){		
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
					
				}
				if(!flag[4]) flag[3]=true;		
			}
			if(flag[4]) destroyAllWindows();
		}
	}

	if(flag[4]) destroyAllWindows();
	if(!flag[4]) end = time(0);
	cout << "Success Rate: " << (float)(success*100.0f/total) << '%' << endl;
	if(!flag[4]) cout << "Time Elapsed: " << difftime(end, start) << " s" << endl;
	cout << "Succesfully exited." << endl;
	return 0;
}
