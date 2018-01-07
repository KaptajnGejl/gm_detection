#ifndef FUNC_HPP_INCLUDED
#define FUNC_HPP_INCLUDED

#include <opencv2/opencv.hpp>

// Namespaces
using namespace cv;

/* Prototypes */

void clrscr();
int optimal_threshold(Mat hist);
Mat hist(Mat img, bool show_image);

#endif