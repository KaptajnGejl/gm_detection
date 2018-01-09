#ifndef FUNC_HPP_INCLUDED
#define FUNC_HPP_INCLUDED

#include <opencv2/opencv.hpp>
#include <vector> 

// Namespaces
using namespace cv;
using namespace std;

//Structs
struct corner{
	Point pos = Point(0,0);
	uint16_t dist = 0;
};

/* Prototypes */

void clrscr();
bool dist_cmp(corner a, corner b);
int optimal_threshold(Mat hist);
Mat hist(Mat img, bool show_image);
corner global_center(vector<corner> corners);
corner cross_center(Mat& img_cor, vector<corner> corners);

#endif