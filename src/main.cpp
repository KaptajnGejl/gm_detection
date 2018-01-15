#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

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
	string path, path_arg, failpath, arg, ext, vid_name;
	Mat img, img_blr, img_thr, img_cor, img_edge, histogram;
	uint16_t success = 0, total = 0;
	clock_t start = time(0), end = time(0), timer = time(0), timer_old=time(0);
	vector<Point> points;
	vector<corner> corners;




	/* Flags */ 
	bool flag[7] = {false,false,false,false,false,false,false}; 													//Has to be same size as number of possible arguments

	/*
	Flag table: 
	0: image path
	1: extension
	2: exit
	3: continue
	4: manual
	5: webcam
	6: save video
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
			cout << "Failure mode: -f" << endl;
			cout << "Use webcam as source: -w" << endl;
			cout << "Save video from webcam to file: -sv" << endl;
      
			return -1;
		}
		else if(arg == "-p"){
			path_arg = argv[i+1];
			failpath = path_arg + "/failures/";
			path = path_arg + "/pic/";
      
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
		else if(arg == "-f"){
			path = path_arg + "/failures/";
			cout << "Path set to: " << path << endl;
		}
		else if(arg == "-w"){
			flag[5] = true;
			cout << "Using webcam as image source" << endl;
		}
		else if(arg == "-sv"){
			flag[6] = true;
			cout << "Please enter filename for video: " ;
			getline(cin, vid_name);
			if(vid_name.length() > 0){
				cout << "Saving video to " << vid_name << ".avi" << endl;
				vid_name += ".avi";
			}
			else{
				cout << "Video name must be longer than zero characters." << endl;
				return -1;
			}
		}
	}

	if(!(flag[5] || flag[6]) && !(flag[0] && flag[1])){															//Check for correct arguments
		cout << "Both path and extension must be set. Use -help for details." << endl;
		return -1;
	}
	else if(flag[6] && !flag[5]){
		cout << "Webcam must be used if video is to be saved" << endl;
		return -1;
	}
	

	if(flag[0] && flag[1]){

		dir = opendir(path.c_str());															//try to open image directory

		if(dir){
			cout << endl << "Opened working directory." << endl;  
		}
		else{
			cout << "Failed to open working directory." << endl;
			return -1;
		}
	}else{
		dir = opendir("bogus");
	}
		

	cout << endl << "Press Enter to coninue..." << endl; 
	while(cin.get()!='\n'){}

	if(!flag[4]) start = time(0);


	/* End of argument handling */ 


	/* Main loop */ 
	if(!flag[5]){
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
				}

				flag[3] = false;
        
				while(!flag[3]){
					total++;

					if(flag[4])imshow(fname,img);	 
					
					/* Blurring */

					if(flag[4]) cout << "Blurring image..." << endl << endl;
					GaussianBlur(img,img_blr,Size(1,1),0,0);

					/* Canny edge detection */

					if(flag[4]) cout << "Finding strong edges in the image..." << endl << endl;
					Canny(img_blr,img_edge, 125, 200, 3,true);
				

					/* Histogram generation, for debugging */


					if(flag[4]) cout << "Generating histogram of pixels in raw image..." << endl << endl;					
					if(flag[4]) histogram = hist(img, true);
		
					/* If in manual mode, show the images in windows */ 

					if(flag[4]){
						namedWindow("Edges",WINDOW_AUTOSIZE);
						moveWindow("Edges",1280-img_edge.size().width,53+img.size().height);
						imshow("Edges",img_edge);

						namedWindow("Blurred",WINDOW_AUTOSIZE);
						moveWindow("Blurred",1280-2*(img_blr.size().width),2*(53+img.size().height));
						imshow("Blurred",img_blr);
					}

					/* Corner Detection */

					if(flag[4]) cout << "Detecting corners in the edge image..." << endl << endl;
					goodFeaturesToTrack(img_edge,points,16,0.2,10,noArray(),11,false,0.04);


					/* Draw found corners */

					cvtColor(img,img_cor,COLOR_GRAY2BGR,0);		
					for(unsigned int i = 0; i<points.size();i++){
						circle(img_cor,points[i],3,Scalar(0,140,255),-1,0);
					}

					/* If some points are found, process them */

					if(points.size()>0){

						corners = cvtCorner(points); 					
						points.clear();
		 				
						/* First, search the image for good squares */ 

						corner c_center = find_square(corners, img_cor, img, 0.02, 0.8);

						/* If no squares are found, search for triangles instead */

						if(c_center.pos.x == 0 && c_center.pos.y == 0) {
							c_center = find_triangle(corners, img_cor, img, 0.3, 0.8);
						}

						/* If either a square or triangle is found, count up no. of successes, and draw the center of the corner */ 

						if(c_center.pos.x != 0 && c_center.pos.y != 0){
							circle(img_cor,Point(c_center.pos.x,c_center.pos.y),3,Scalar(255,0,0),-1,0);
							success++;


						/* If not in manual mode, save images where detection failed */ 
						}else if(!flag[4]){
							imwrite(failpath + fname, img);
						}

						/* Show the result, and prompt the user for input */ 

						if(flag[4]){		
							namedWindow("Corners",WINDOW_AUTOSIZE);
							moveWindow("Corners",1280-img_blr.size().width,2*(53+img.size().height));
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
						corners.clear();
					}
					else{ /* Continue to next image if no points are found */ 
						flag[3]=true;
					} /* If in automatic mode, always continue to next image */ 
					if(!flag[4]) flag[3]=true;		
				}
				if(flag[4]) destroyAllWindows();
			}
		}
	}
	else{
		VideoCapture stream(0);
		VideoWriter video = VideoWriter(vid_name,video.fourcc('M','J','P','G'),15,Size(320,240));
		uint16_t FPS = 0;

		/* Check if camera device has been initialised */
		if (!stream.isOpened()) { 
			cout << "Cannot open camera" << endl;
			return -1;
		}

		/* If save video is enabled, check if video stream can be opened */
		if(flag[6]){
			if (!video.isOpened()) {
			cout << "Cannot open video file, maybe you don't have the MJPG codec?" << endl;
			return -1;
			}
		}
	
		stream.set(CV_CAP_PROP_FRAME_WIDTH,320);
		stream.set(CV_CAP_PROP_FRAME_HEIGHT,240);
	
		while(true){

			/* Calculate and display frames per second*/
			timer = time(0);
			if(timer!=timer_old){
				timer_old = time(0);
				clrscr();
				cout << "FPS: " << FPS << endl;
				FPS = 0;
			}
			FPS++;

			stream.read(img);
			cvtColor(img,img,COLOR_BGR2GRAY,0);	

			namedWindow("Stream",WINDOW_AUTOSIZE);
			moveWindow("Stream",1280-img.size().width,20);
			waitKey(1);
			imshow("Stream",img);	 
			
			/* Blurring */

			GaussianBlur(img,img_blr,Size(3,3),0,0);
			
			/* Thresholding*/


      Canny(img_blr,img_edge, 50, 200, 3,true);
    
			namedWindow("Edges",WINDOW_AUTOSIZE);
			moveWindow("Edges",1280-img_edge.size().width,53+img.size().height);
			imshow("Edges",img_edge);
      
			/* Corner Detection */

			goodFeaturesToTrack(img_edge,points,16,0.2,10,noArray(),11,false,0.04);


			/* Draw found corners */

			cvtColor(img,img_cor,COLOR_GRAY2BGR,0);		
			for(unsigned int i = 0; i<points.size();i++){
				circle(img_cor,points[i],3,Scalar(0,140,255),-1,0);
			}

			/* Find center of cross, and draw it */

			if(points.size()>0){
				corners = cvtCorner(points);
				points.clear();
 				
 				corner c_center = find_square(corners, img_cor, img, 0.02, 0.8);

				if(c_center.pos.x==0 && c_center.pos.y == 0) {

					c_center = find_triangle(corners, img_cor, img, 0.3, 0.8);
				}

				if(c_center.pos.x!=0 && c_center.pos.y != 0){
					circle(img_cor,Point(c_center.pos.x,c_center.pos.y),3,Scalar(255,0,0),-1,0);
					success++;
				}

				namedWindow("Corners",WINDOW_AUTOSIZE);
				moveWindow("Corners",1280-img_cor.size().width,2*(53+img.size().height));
				imshow("Corners",img_cor);

				corners.clear();
			}
			if(flag[6]) video.write(img_cor);

			char key = waitKey(10);
			if(key == 'q') break;	
			
		}
	}

	if(flag[4]) destroyAllWindows();
	if(!flag[4]) end = time(0);
	if(!flag[5])cout << "Success Rate: " << (float)(success*100.0f/(total-1)) << '%' << endl;
	if(!flag[4]) cout << "Time Elapsed: " << difftime(end, start) << " s" << endl;
	cout << "Succesfully exited." << endl;
	return 0;
}
