// Libraries
#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>

#include "func.hpp"

// Namespaces
using namespace std;
using namespace cv;


void clrscr(){

	cout << "\033[2J \033[1;1H";

	return;
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
	
	return T+10;
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



