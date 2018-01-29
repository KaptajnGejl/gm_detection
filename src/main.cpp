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
#include <fstream>
#include <cctype>

#include "func.hpp"

using namespace std;
using namespace cv;

struct dirent *drnt;

int ksz = 1, qlv = 20, mxc = 16, bsz = 11;

static void ksz_cb(int , void*){
	if(ksz==0) ksz = 1;
	else if(ksz%2 == 0) ksz--;
}

static void qlv_cb(int , void*){
	if(qlv==0) qlv = 1;	
}

static void mxc_cb(int , void*){
	if(mxc==0) mxc = 1;
}

static void bsz_cb(int , void*){
	if(bsz==0) ksz = 1;
	else if(bsz%2 == 0) bsz--;
}


int main(int argc, char const *argv[])
{
	DIR *dir; 
	string path, path_arg, failpath, arg, ext, vid_name;
	Mat img, img_blr, img_thr, img_cor, img_edge, histogram;
	int success = 0, total = 0, ct1 = 125, ct2 = 200, mds = 10, sql = 20, trl = 300, ctc = 20, t_start = 0, t_end = 0, dev_id = 0;
	float t = 0.0;
	clock_t start = time(0), end = time(0), timer = time(0), timer_old=time(0);
	vector<Point> points;
	vector<corner> corners;

	/* Flags */ 
	bool flag[8] = {false,false,false,false,false,false,false,false}; 													//Has to be same size as number of possible arguments

	/*
	Flag table: 
	0: image path
	1: extension
	2: exit
	3: continue
	4: manual
	5: webcam
	6: save video
	7: test
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

			cout << "#################################################" << endl;
			cout << "################### Help Menu ###################" << endl;
			cout << "#################################################" << endl << endl;

			string help;
			help = "Current path:";
			cout << help << setw(49-help.length()) << "-p $PWD" << endl;
			help = "Image file extension:";
			cout << help << setw(49-help.length()) << "-e .extension" << endl;
			help = "Manual mode:";
			cout << help << setw(49-help.length()) << "-m" << endl;
			help = "Failure mode:";  
			cout << help << setw(49-help.length()) << "-f" << endl;
			help = "Use camera with device id # as source:";
			cout << help << setw(49-help.length()) << "-w #" << endl;
			help = "Save video to file:";
			cout << help << setw(49-help.length()) << "-sv" << endl;
			help = "Test mode:";
			cout << help << setw(49-help.length()) << "-t" << endl;

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
			dev_id = stoi(argv[i+1]);
			cout << "Using camera " << dev_id << " as image source" << endl;
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
		else if(arg == "-t"){
			flag[7] = true;
			cout << "Test mode selected" << endl;
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
	else if(flag[7] && !flag[5]){
		cout << "Test mode must use webcam. Use -help for details." << endl;
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
	}
	else{
		dir = opendir("bogus");
	}

	/* Read parameters */

	cout << endl << "Reading parameters from params.txt..." << endl << endl;

	string search, line;
	fstream params;
	uint16_t l = 0;

	params.open("params.txt",ios::in);
	if(!params){
		cout << "Unable to open parameter file." << endl;
		return -1;
	} 

	while(params.good()){
		getline(params,line);	

		if(line.substr(0,3) == "ksz"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			ksz = stoi(line.substr(5,l-5));
			cout << "Setting kernel size to " << ksz <<  endl;
		}
		if(line.substr(0,3) == "ct1"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			ct1 = stoi(line.substr(5,l-5));
			cout << "Setting lower canny threshold to " << ct1 <<  endl;
		}
		if(line.substr(0,3) == "ct2"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			ct2 = stoi(line.substr(5,l-5));
			cout << "Setting upper canny threshold to " << ct2 <<  endl;
		}
		if(line.substr(0,3) == "mxc"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			mxc = stoi(line.substr(5,l-5));
			cout << "Setting maximum number of corners to " << mxc <<  endl;
		}
		if(line.substr(0,3) == "qlv"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			qlv = stoi(line.substr(5,l-5));
			cout << "Setting quality level to " << float(qlv)/100 <<  endl;
		}
		if(line.substr(0,3) == "mds"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			mds = stoi(line.substr(5,l-5));
			cout << "Setting minimum distance between corners to " << mds <<  endl;
		}
		if(line.substr(0,3) == "bsz"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			bsz = stoi(line.substr(5,l-5));
			cout << "Setting corner blocksize to " << bsz <<  endl;
		}
		if(line.substr(0,3) == "sql"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			sql = stoi(line.substr(5,l-5));
			cout << "Setting square match limit to " << float(sql)/1000 <<  endl;
		}
		if(line.substr(0,3) == "trl"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			trl = stoi(line.substr(5,l-5));
			cout << "Setting triangle match limit to " << float(trl)/1000 <<  endl;
		}
		if(line.substr(0,3) == "ctc"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			ctc = stoi(line.substr(5,l-5));
			cout << "Setting CtC ratio to " << float(ctc)/10 <<  endl;
		}
		/*if(line.substr(0,3) == "exp"){
			for(l = 0; l < line.length(); l++){
				if(isspace(line.at(l))) 
					break;
			}
			exp = stoi(line.substr(5,l-5));
			cout << "Setting camera exposure time to: " << exp <<  endl;
		}*/
	}
	params.close(); 

	cout << endl << "Press Enter to coninue..." << endl; 
	while(cin.get()!='\n'){}

	if(!flag[4]) start = time(0);


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
					GaussianBlur(img,img_blr,Size(ksz,ksz),0,0);

					/* Canny edge detection */

					if(flag[4]) cout << "Finding strong edges in the image..." << endl << endl;
					Canny(img_blr,img_edge, ct1, ct2, 3,true);
				

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
					goodFeaturesToTrack(img_edge,points,mxc,float(qlv)/100,mds,noArray(),bsz,false,0.04);


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

						corner c_center = find_square(corners, img_cor, img, float(sql)/1000, float(ctc)/100);

						/* If no squares are found, search for triangles instead */

						if(c_center.pos.x == 0 && c_center.pos.y == 0) {
							c_center = find_triangle(corners, img_cor, img, float(trl)/1000, float(ctc)/100);
						}

						/* If either a square or triangle is found, count up no. of successes, and draw the center of the corner */ 

						if(c_center.pos.x != 0 && c_center.pos.y != 0){
							circle(img_cor,Point(c_center.pos.x,c_center.pos.y),3,Scalar(255,0,0),-1,0);
							success++;

						/* If not in manual mode, save images where detectionn failed */ 

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
		VideoCapture stream(dev_id);
		VideoWriter video = VideoWriter(vid_name,video.fourcc('M','J','P','G'),15,Size(320,240));
		uint16_t FPS = 0;

		stream.set(CV_CAP_PROP_FRAME_WIDTH,320);
		stream.set(CV_CAP_PROP_FRAME_HEIGHT,240);

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
	
		while(true){

			if(flag[7]){
				namedWindow("Params",WINDOW_AUTOSIZE);
				moveWindow("Params",1280-2*img.size().width,20);
				createTrackbar("Kernel size","Params",&ksz,31,ksz_cb);
				createTrackbar("Lower canny threshold","Params",&ct1,255,NULL);
				createTrackbar("Upper canny threshold","Params",&ct2,255,NULL);
				createTrackbar("Max corners","Params",&mxc,20,mxc_cb);
				createTrackbar("Quality level","Params",&qlv,100,qlv_cb);
				createTrackbar("Min. distance","Params",&mds,50,NULL);
				createTrackbar("Block size","Params",&bsz,31,bsz_cb);
				createTrackbar("Square limit","Params",&sql,200,NULL);
				createTrackbar("Triangle limit","Params",&trl,500,NULL);
				createTrackbar("CtC ratio","Params",&ctc,100,NULL);

			}

			/* Calculate and display frames per second*/
			timer = time(0);
			if(timer!=timer_old){
				timer_old = time(0);
				clrscr();
				cout << "FPS: " << FPS << endl;
				cout << "Average execution time: " << t/FPS << endl;
				cout << "Max. possible FPS: " << FPS/t << endl;
				t = 0;
				FPS = 0;
			}
			FPS++;

			stream.read(img);

			t_start = getTickCount();

			cvtColor(img,img,COLOR_BGR2GRAY,0);	

			namedWindow("Stream",WINDOW_AUTOSIZE);
			moveWindow("Stream",1280-img.size().width,20);
			waitKey(1);
			imshow("Stream",img);	 
			
			/* Blurring */

			GaussianBlur(img,img_blr,Size(ksz,ksz),0,0);
			
			/* Thresholding*/

      		Canny(img_blr,img_edge, ct1, ct2, 3,true);
    
			namedWindow("Edges",WINDOW_AUTOSIZE);
			moveWindow("Edges",1280-img_edge.size().width,53+img.size().height);
			imshow("Edges",img_edge);
			
			/* Corner Detection */

			goodFeaturesToTrack(img_edge,points,mxc,float(qlv)/100,mds,noArray(),bsz,false,0.04);


			/* Draw found corners */

			cvtColor(img,img_cor,COLOR_GRAY2BGR,0);		
			for(unsigned int i = 0; i<points.size();i++){
				circle(img_cor,points[i],3,Scalar(0,140,255),-1,0);
			}

			/* Find center of cross, and draw it */

			if(points.size()>0){
				corners = cvtCorner(points);
				points.clear();
 				
 				corner c_center = find_square(corners, img_cor, img, float(sql)/1000, float(ctc)/10);

				if(c_center.pos.x==0 && c_center.pos.y == 0) {

					c_center = find_triangle(corners, img_cor, img, float(trl)/1000, float(ctc)/10);
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

			t_end = getTickCount();
			t += float(t_end - t_start)/getTickFrequency();	
			
		}
	}

	if(flag[7]){
		string outline;
		params.open("params.txt",ios::out|ios::trunc);
		if(!params){
		cout << "Unable to open parameter file." << endl;
		} 
		outline = "ksz:=" + to_string(ksz);
		params << outline  << setw(80-outline.length()) << "#size of kernel for gaussian blurring\n";
		outline = "ct1:=" + to_string(ct1);
		params << outline  << setw(80-outline.length()) <<  "#lower threshold for canny edge detector\n";
		outline = "ct2:=" + to_string(ct2);
		params << outline  << setw(80-outline.length()) <<  "#upper threshold for canny edge detector\n";
		outline = "mxc:=" + to_string(mxc);
		params << outline  << setw(80-outline.length()) <<  "#maximum number of corners return from goodFeaturesToTrack\n";
		outline = "qlv:=" + to_string(qlv);
		params << outline  << setw(80-outline.length()) <<  "#quality level of goodFeaturesToTrack (qlv/100)\n";
		outline = "mds:=" + to_string(mds);
		params << outline  << setw(80-outline.length()) <<  "#minimum distance of goodFeaturesToTrack\n";		
		outline = "bsz:=" + to_string(bsz);
		params << outline  << setw(80-outline.length()) <<  "#blocksize of goodFeaturesToTrack\n";	
		outline = "sql:=" + to_string(sql);
		params << outline  << setw(80-outline.length()) <<  "#square search match limit (sql/1000)\n";
		outline = "trl:=" + to_string(trl);
		params << outline  << setw(80-outline.length()) <<  "#triangle search match limit (trl/1000)\n";
		outline = "ctc:=" + to_string(ctc);
		params << outline  << setw(80-outline.length()) <<  "#Corner to center intensity ratio\n";
		//outline = "exp:=" + to_string(exp);
		//params << outline  << setw(80-outline.length()) <<  "#Exposure time of camera in ms\n";

		params.close();
	}

	if(flag[4]) destroyAllWindows();
	if(!flag[4]) end = time(0);
	if(!flag[5])cout << "Success Rate: " << (float)(success*100.0f/(total-1)) << '%' << endl;
	if(!flag[4]) cout << "Time Elapsed: " << difftime(end, start) << " s" << endl;
	cout << "Succesfully exited." << endl;
	return 0;
}
