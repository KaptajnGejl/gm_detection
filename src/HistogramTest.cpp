#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;


int const max_BINARY_value = 255;

Mat image,img_thr,img_cor,img_cor_thr,img_cor_norm;
int threshold_type = 0;


int optimal_threshold(Mat hist);

Mat hist(Mat img, bool show_image);

int main(int argc, char const *argv[])
{


	image = imread("/Users/FrederiksMac/Documents/GitHub/pic/img1498004642_093301.bmp", CV_LOAD_IMAGE_GRAYSCALE);


	namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", image );     

	
	int t = optimal_threshold(hist(image,false));

	threshold(image, img_thr, t, max_BINARY_value, threshold_type );

	namedWindow( "Threshold", CV_WINDOW_AUTOSIZE );
  	imshow("Threshold", img_thr);

  	cornerHarris(img_thr,img_cor,7, 5, 0.05, BORDER_DEFAULT );

  	normalize( img_cor, img_cor_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );

  	t = optimal_threshold(hist(img_cor_norm,false));

	threshold(img_cor_norm, img_cor_thr, t, max_BINARY_value, threshold_type );

	namedWindow( "Corners thresholded", CV_WINDOW_AUTOSIZE );
  	imshow("Corners thresholded", img_cor_thr);

  	vector<vector<Point> > contours;
 	vector<Vec4i> hierarchy;

 	findContours(img_cor_thr, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  	waitKey(0);

	return 0;
}


int optimal_threshold(const Mat hist)
{
	cout << "Finding optimal threshold.." << endl << endl;
	
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
		cout << "Current threshold: " << T << endl;
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
	    imshow( "Histogram", hist_image ); 
	}

	return hist;

}


