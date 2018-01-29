#ifndef FUNC_HPP_INCLUDED
#define FUNC_HPP_INCLUDED

#include <opencv2/opencv.hpp>
#include <vector> 

/* Namespaces */
using namespace cv;
using namespace std;

/* Structs */

struct corner{
	Point pos = Point(0,0);
	uint16_t dist = 0;
};

/* Prototypes */

void clrscr();
bool dist_cmp(corner a, corner b);
vector<corner> cvtCorner(vector<Point> points);
vector<Point> cvtPoint(vector<corner>corners);
Mat hist(Mat img, bool show_image);
corner global_center(vector<corner> corners);
corner find_square(vector<corner> corners, Mat& img_cor ,Mat& img, float match_limit, float center_limit); 
corner find_triangle(vector<corner> corners, Mat& img_cor, Mat& img, float match_limit, float center_limit); 
double squareMatch(vector<Point> points);
double triangleMatch(vector<Point> points);
corner triangle_center(vector<corner> corners);
bool center_check(corner center, Mat histogram, Mat& img, float limit_factor);
bool point_check(corner center, vector<corner> corners, Mat& img, float limit_factor);

#endif