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
vector<corner> cvtCorner(vector<Point> points);
vector<Point> cvtPoint(vector<corner>corners);
int optimal_threshold(Mat hist);
Mat hist(Mat img, bool show_image);
corner global_center(vector<corner> corners);
corner cross_center(Mat& img_thr, Mat& img_cor, vector<corner> corners, bool print);
vector<corner> findSquares(Mat& img_cor, vector<corner> corners, bool print);
corner find_square2(vector<corner> corners,Mat& img_cor,Mat& img_thr, float limit); 
corner find_triangle2(vector<corner> corners,Mat& img_cor, Mat& img_thr, float limit); 
vector<corner> findTriangles(Mat& img_thr, Mat& img_cor, vector<corner> corners, bool print);
bool squareMatch(Mat& img_cor, vector<Point> points, float limit, bool print);
bool triangleMatch(Mat& img_cor, vector<Point> points, float limit, bool print);

double squareMatch2(vector<Point> points);
double triangleMatch2(vector<Point> points);
bool similar_point_check(Mat& img,vector<corner> p);
corner triangle_center(vector<corner> corners);
#endif