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
    // Clears out terminal window
	cout << "\033[2J \033[1;1H";

	return;
} 

bool dist_cmp(corner a, corner b){
	// vector compare function
	return a.dist < b.dist;
}

vector<corner> cvtCorner(vector<Point> points){
	// Convert from OpenCV Point type to proprietary corner type
	vector<corner> corners;
	corner t;

	for(unsigned int i = 0; i<points.size(); i++){
		t.pos.x = points[i].x;
		t.pos.y = points[i].y;
		corners.push_back(t);
		}

	return corners;
}

vector<Point> cvtPoint(vector<corner> corners){
	// Convert from proprietary corner type to OpenCV Point type 

	vector<Point> points;
	Point t;

	for(unsigned int i = 0; i<corners.size(); i++){
		t.x = corners[i].pos.x;
		t.y = corners[i].pos.y;
		points.push_back(t);
		}

	return points;
}

Mat hist(Mat img, bool show_image) {

	// Computes histogram of a given image

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
	// Finds the mean center from a vector of corner type

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


corner find_square(vector<corner> corners, Mat& img_cor, Mat& img, float match_limit, float center_limit) {

	// Finds the best square from a given vector of corners
	// the function loops through every combination of 4 points and determines how well these match a square
	// the best combination with a match under a given limit is determined to be four corners in a cross
	// that is, if the center of the found square is black thus is in the cross.
	// the center of the found square is returned. If no square is found, the center (0,0) is returned

	double match = 0,best_match = 10;
	vector<corner> result, temp;
	corner c_center;

	if (corners.size() >= 4) {

		for (unsigned int i = 0; i < corners.size(); i++)
		{	
			temp.push_back(corners[i]);
			for (unsigned int j = i+1; j < corners.size(); j++)
			{
				temp.push_back(corners[j]);
				for (unsigned int k = j+1; k < corners.size(); k++)
				{
					temp.push_back(corners[k]);
					for (unsigned int l = k+1; l < corners.size(); l++)
					{
						temp.push_back(corners[l]);

						match = squareMatch(cvtPoint(temp));

						if ((match < best_match) && (match < match_limit) && (match != 0))  {

							corner temp_center = global_center(temp);

							if (center_check(temp_center,hist(img,false),img,center_limit)) { //&& similar_point_check(img_thr,temp)) {
								best_match = match;
								result = temp;
							}
						}

						temp.pop_back();	
					}

					temp.pop_back();
				}	

				temp.pop_back();	
			}		

			temp.pop_back();
		}
	}
	if(result.size()>0){
		for(unsigned int j = 0; j<result.size(); j++){
			c_center.pos.x += result[j].pos.x;
			c_center.pos.y += result[j].pos.y;
			circle(img_cor,Point(result[j].pos.x,result[j].pos.y),3,Scalar(0,255,0),-1,0);
		}

		c_center.pos.x /= result.size();
		c_center.pos.y /= result.size();
	}
	
	return c_center;
}

corner find_triangle(vector<corner> corners, Mat& img_cor, Mat& img, float match_limit, float center_limit) {

	// Finds the best right-angled isosceles triangle from a given vector of corners
	// the function loops through every combination of 3 points and determines how well these match a right-angled isosceles triangle
	// the best combination with a match under a given limit is determined to be three corners from a cross
	// that is, if the center point between the two corners furthest apart in the found triangle is black, thus is in the cross.
	// the center of the found triangle is returned. If no triangle is found, the center (0,0) is returned



	double match = 0,best_match = 10;
	vector<corner> result, temp;
	corner c_center;

	if (corners.size() >= 3) {

		for (unsigned int i = 0; i < corners.size(); i++)
		{	
			temp.push_back(corners[i]);
			for (unsigned int j = i+1; j < corners.size(); j++)
			{
				temp.push_back(corners[j]);
				for (unsigned int k = j+1; k < corners.size(); k++)
				{
				
					temp.push_back(corners[k]);

					match = triangleMatch(cvtPoint(temp));

					if ((match < best_match) && (match < match_limit) && (match != 0)) {
						corner temp_center = triangle_center(temp);
						if (center_check(temp_center,hist(img,false),img,center_limit)) { 
							best_match = match;
							result = temp;
						}
					}


					temp.pop_back();
				}	

				temp.pop_back();	
			}		

			temp.pop_back();
		}
	}
	if(result.size()>0){
		for(unsigned int j = 0; j<result.size(); j++){
			circle(img_cor,Point(result[j].pos.x,result[j].pos.y),3,Scalar(0,255,0),-1,0);
		}

		c_center = triangle_center(result);
	}
	
	return c_center;
}

double squareMatch(vector<Point> points){

	//returns the match between the points in a given vector and a square using the OpenCV function matchShapes.

	vector<Point> hull,poly;
	vector<vector<Point>> cnt;

	poly.push_back(Point(0,0));
	poly.push_back(Point(0,30));
	poly.push_back(Point(30,30));
	poly.push_back(Point(30,0));
	cnt.push_back(poly);
	poly.clear();

	convexHull(points,hull,false);
	approxPolyDP(hull,poly,0.01*arcLength(hull,true),true);

	cnt.push_back(poly);
	poly.clear();
	return matchShapes(cnt[0],cnt[1],CONTOURS_MATCH_I1,0);
}

double triangleMatch(vector<Point> points){

	//returns the match between the points in a given vector and a right-angled isosceles triangle, using the OpenCV function matchShapes.

	vector<Point> hull,poly;
	vector<vector<Point>> cnt;

	poly.push_back(Point(0,0));
	poly.push_back(Point(0,30));
	poly.push_back(Point(30,0));
	cnt.push_back(poly);
	poly.clear();

	convexHull(points,hull,false);
	approxPolyDP(hull,poly,0.01*arcLength(hull,true),true);

	cnt.push_back(poly);
	poly.clear();

	return matchShapes(cnt[0],cnt[1],CONTOURS_MATCH_I1,0);

}


corner triangle_center(vector<corner> corners){
	
	// finds the center point between the two points furthers apart in a triangle. 
	// the distance and middle point between each of the 3 points is determined and sorted with respect to the distance
	// the middle point of the pair furthest apart is returned

	vector<corner> result_vc;
	corner temp;

	temp.pos.x = (corners[0].pos.x+corners[1].pos.x)/2;
	temp.pos.y = (corners[0].pos.y+corners[1].pos.y)/2;
	temp.dist = (corners[0].pos.x-corners[1].pos.x)*(corners[0].pos.x-corners[1].pos.x)+(corners[0].pos.y-corners[1].pos.y)*(corners[0].pos.y-corners[1].pos.y);

	result_vc.push_back(temp);

	temp.pos.x = (corners[0].pos.x+corners[2].pos.x)/2;
	temp.pos.y = (corners[0].pos.y+corners[2].pos.y)/2;
	temp.dist = (corners[0].pos.x-corners[2].pos.x)*(corners[0].pos.x-corners[2].pos.x)+(corners[0].pos.y-corners[2].pos.y)*(corners[0].pos.y-corners[2].pos.y);

	result_vc.push_back(temp);

	temp.pos.x = (corners[1].pos.x+corners[2].pos.x)/2;
	temp.pos.y = (corners[1].pos.y+corners[2].pos.y)/2;
	temp.dist = (corners[1].pos.x-corners[2].pos.x)*(corners[1].pos.x-corners[2].pos.x)+(corners[1].pos.y-corners[2].pos.y)*(corners[1].pos.y-corners[2].pos.y);

	result_vc.push_back(temp);

	sort(result_vc.begin(),result_vc.end(),dist_cmp);

	return result_vc.back();
}

bool center_check(corner center, Mat histogram, Mat& img, float limit_factor) {

	// checks if a given point in an image has a intensity less than the mean intensity multiplied with a given factor.
	// used to check if a found center lies inside a cross

	int sum_i = 0,sum_p = 0;

	for (int i = 0; i < 256; ++i)
		{
			sum_i += i*histogram.at<float>(i);
			sum_p += histogram.at<float>(i);
		}

		float mean = sum_i/sum_p;

		if (img.at<uchar>(center.pos.y,center.pos.x) < mean*limit_factor) return true;

		else return false;

}


