// Libraries
#include <iostream>
#include <iomanip>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>

#include "func.hpp"

// Namespaces
using namespace std;
using namespace cv;


void clrscr(){

	cout << "\033[2J \033[1;1H";

	return;
} 

bool dist_cmp(corner a, corner b){
	return a.dist < b.dist;
}

int optimal_threshold(const Mat hist)
{
	cout << endl << "Finding optimal threshold..." << endl << endl;
	
	int T = 130;
	
	int T_old = 0;
	int max_iter = 10;
	int iter = 0;

	float mu1,mu2;

	float sum_i,sum_p;

	while ((T != T_old) && (iter < max_iter)) 
	{	
		T_old = T;
		iter++;
		//cout << "Current threshold: " << T << endl;

		for (int i = 0; i < T; ++i)
		{
			sum_i += i*hist.at<float>(i);
			sum_p += hist.at<float>(i);
		}

		mu1 = sum_i/sum_p;
		sum_i = 0;
		sum_p = 0;

		for (int i = T; i <256; ++i)
		{
			sum_i += i*hist.at<float>(i);
			sum_p += hist.at<float>(i);
		}

		mu2 = sum_i/sum_p;
		sum_i = 0;
		sum_p = 0;

		T = int((mu1+mu2)/2);
		
	}
	
	return T-30;
}

Mat hist(Mat img, bool show_image) 
{

	int histSize[] = {256};
	float range[] = { 0, 256 } ;
  	const float* histRange = { range };

  	int const hist_height = 256;
    Mat3b hist_image = cv::Mat3b::zeros(hist_height, 256);

  	Mat hist;
  	int channels[] = {0};

  	calcHist(&img, 1, channels, Mat(), hist, 1, histSize, &histRange, true, false);

  	double max_val=0;
    minMaxLoc(hist, 0, &max_val);
    if (show_image) {
	    // visualize each bin
	    for(int b = 0; b < 256; b++) {
	        float const binVal = hist.at<float>(b);
	        int   const height = cvRound(binVal*hist_height/max_val);
	        line(hist_image, Point(b, hist_height-height), Point(b, hist_height), Scalar::all(255));
	    }

		namedWindow( "Histogram", WINDOW_AUTOSIZE );// Create a window for display.
	    imshow("Histogram", hist_image ); 
	    moveWindow("Histogram",1280-img.size().width-hist_image.size().width,20);
	    waitKey(1);
	}

	return hist;

}

corner global_center(vector<corner> corners){
	
	corner temp;

	
	if(corners.size() > 0){

		for(unsigned int i = 0; i<corners.size(); i++){

			temp.pos.x += corners[i].pos.x;
			temp.pos.y += corners[i].pos.y; 

		}

		temp.pos.x /= corners.size();
		temp.pos.y /= corners.size();
	}

	return temp;
}

corner cross_center(Mat& img_cor, vector<corner> corners){

	corner center, c_center;
	vector<corner> result;
	char count = 0;
	bool first=false;
	unsigned int i = 0,index = 0;
	float avg_err = 0, best_err = 10000;

	center = global_center(corners);

	for(i = 0; i<corners.size(); i++){

		corners[i].dist = sqrt(pow(corners[i].pos.x-center.pos.x,2)+pow(corners[i].pos.y-center.pos.y,2));

		//cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;

	}
	cout << endl;

	sort(corners.begin(),corners.end(),dist_cmp);
	/*
	for(i = 0; i<corners.size(); i++){
		cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
	}
	*/

	while(corners.size()>4){

		cout << endl << "Corners(" << corners.size() << "): " << endl;
		for(i = 0; i<corners.size(); i++){
			cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
		}

		for(i = 1; i<corners.size(); i++){
			if(abs(corners[i].dist-corners[i-1].dist)<=10 && first == false){
				count++;
				first = true;
				index = i-1;
				avg_err += abs(corners[i].dist-corners[i-1].dist);
			}
			else if(abs(corners[i].dist-corners[index].dist)<=10 && first==true){
				count++;
				avg_err += abs(corners[i].dist-corners[i-1].dist);
			}
			else
			{
				count=0;
				first = false;
				avg_err = 0;
			}

			if(count==3){
				avg_err /= 4; 
				if(avg_err<best_err){
					best_err = avg_err;

					result.clear();
					for(unsigned int j = index; j<index+4; j++){
						result.push_back(corners[j]);
					}
					
					cout << "Best error: " << best_err << ", Index:" << index << endl;
				}
				first = false; 
			}
		}

		corners.pop_back();	
		center = global_center(corners);

		for(i = 0; i<corners.size(); i++){
			corners[i].dist = sqrt(pow(corners[i].pos.x-center.pos.x,2)+pow(corners[i].pos.y-center.pos.y,2));
			//cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
		}

		sort(corners.begin(),corners.end(),dist_cmp);
	}

	if(result.size()>0){

		for(unsigned int j = 0; j<result.size(); j++){
			c_center.pos.x += result[j].pos.x;
			c_center.pos.y += result[j].pos.y;
			circle(img_cor,Point(result[j].pos.x,result[j].pos.y),3,Scalar(0,255,0),-1,0);
		}

		c_center.pos.x /= 4;
		c_center.pos.y /= 4;

		//for(i = 0; i<corners.size();i++){
			
		//}

		cout << endl << "Corners(" << corners.size() << "): " << endl;
		for(i = 0; i<corners.size(); i++){
			cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
		}

		cout << "Done!" << endl;

		return c_center;
	}
	else
	{

		for(i = 0; i<corners.size();i++){
			circle(img_cor,Point(corners[i].pos.x,corners[i].pos.y),3,Scalar(0,255,0),-1,0);
		}

		cout << endl << "Corners(" << corners.size() << "): " << endl;
		for(i = 0; i<corners.size(); i++){
			cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
		}

		return center;

	}
}


