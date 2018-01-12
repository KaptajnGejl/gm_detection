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

vector<corner> cvtCorner(vector<Point> points){

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
	vector<Point> points;
	Point t;

	for(unsigned int i = 0; i<corners.size(); i++){
		t.x = corners[i].pos.x;
		t.y = corners[i].pos.y;
		points.push_back(t);
		}

	return points;
}

int optimal_threshold(const Mat hist){
	//cout << endl << "Finding optimal threshold..." << endl << endl;
	
	int T_old = 0;
	int max_iter = 10;
	int iter = 0;

	float mu1,mu2;

	float sum_i = 0,sum_p = 0;

	for (int i = 0; i < 256; ++i)
	{
		sum_i += i*hist.at<float>(i);
		sum_p += hist.at<float>(i);
	}

	int T = sum_i/sum_p;
	sum_i = 0;
	sum_p = 0;

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

		//cout << mu1 << "   "  << mu2 << endl;

		T = int((mu1+mu2)/2);
		
	}
	
	return T;
}

Mat hist(Mat img, bool show_image) {

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


corner cross_center(Mat& img_thr, Mat& img_cor, vector<corner> corners, bool print){

	corner center, c_center, test;
	vector<corner>  temp, result;
	unsigned int i = 0;

  center = global_center(corners);


	/* Calculate distances to global center, and sort */
	for(i = 0; i<corners.size(); i++){
		corners[i].dist = sqrt(pow(corners[i].pos.x-center.pos.x,2)+pow(corners[i].pos.y-center.pos.y,2));
	}
	sort(corners.begin(),corners.end(),dist_cmp);

	result = findSquares(img_cor, corners, print);

	if(result.size()>0){
		for(unsigned int j = 0; j<result.size(); j++){
			c_center.pos.x += result[j].pos.x;
			c_center.pos.y += result[j].pos.y;
			circle(img_cor,Point(result[j].pos.x,result[j].pos.y),3,Scalar(0,255,0),-1,0);
		}

		c_center.pos.x /= result.size();
		c_center.pos.y /= result.size();

		if(print) cout << "Found cross center using square at (" << c_center.pos.x << "," << c_center.pos.y << ")..." << endl << endl;

		return c_center;
	}
	
	result = findTriangles(img_thr, img_cor, corners, print);

	if(result.size()>0){
		for(unsigned int j = 0; j<result.size(); j++){
			c_center.pos.x += result[j].pos.x;
			c_center.pos.y += result[j].pos.y;
			circle(img_cor,Point(result[j].pos.x,result[j].pos.y),3,Scalar(0,255,0),-1,0);
		}

		c_center.pos.x /= result.size();
		c_center.pos.y /= result.size();

		if(print) cout << "Found cross center using triangle at (" << c_center.pos.x << "," << c_center.pos.y << ")..." << endl << endl;

		return c_center;
	}

	if(print) cout << "Failed to find cross center..." << endl;

	return c_center;
}

vector<corner> findSquares(Mat& img_cor, vector<corner> corners, bool print){
	unsigned int i = 0, index = 0;
	float avg_err = 0.0, best_err = 100.0;
	char count = 0;
	vector<corner> temp,result;
	corner center;
	bool first = false;

	while(corners.size()>3){
		for(i = 1; i<corners.size(); i++){
			if(abs(corners[i].dist-corners[i-1].dist)<= 40 && first == false){
				count++;
				first = true;
				index = i-1;
				avg_err += abs(corners[i].dist-corners[index].dist);
			}
			else if(abs(corners[i].dist-corners[index].dist)<= 40 && first==true){
				count++;
				avg_err += abs(corners[i].dist-corners[index].dist);
			}
			else
			{
				count=0;
				first = false;
				avg_err = 0;
			}

			if(count==3){
				count = 0;

				for(unsigned int j = index; j<index+4; j++){
						temp.push_back(corners[j]);
				}

				if( (avg_err/4<best_err) && (squareMatch(img_cor,cvtPoint(temp),0.05,print)) ){
					best_err = avg_err/4;
					result.clear();
					for(unsigned int j = index; j<index+4; j++){
						result.push_back(corners[j]);
					}
					if(print){
						cout << endl << "Found new best square: " << endl;
						cout << endl << "Corners vector is size " << corners.size() << "." << endl;
						for(i = 0; i<corners.size(); i++){
							cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
						}			 
						cout << "New best error: " << best_err << ", At position: " << index << endl;
					}
				}

				temp.clear();
				first = false;
				i = index+1; 
				avg_err = 0;
			}
		}

		corners.pop_back();	
		center = global_center(corners);
		for(i = 0; i<corners.size(); i++){
			corners[i].dist = sqrt(pow(corners[i].pos.x-center.pos.x,2)+pow(corners[i].pos.y-center.pos.y,2));
		}

		sort(corners.begin(),corners.end(),dist_cmp);
	}

	if(result.size()>0){
		return result;
	}
	else{
		for(unsigned int j = 0; j<corners.size(); j++){
			temp.push_back(corners[j]);
		}

		if(squareMatch(img_cor,cvtPoint(temp),0.05,print)){
			for(i = 0; i<corners.size();i++){
				circle(img_cor,Point(corners[i].pos.x,corners[i].pos.y),3,Scalar(0,255,0),-1,0);
			}

			if(print) cout << "Defaulting to square consisting of the four corners closest to the total center of mass of all the corners." << endl;
			temp.clear();
			return temp;
		}
		else{
			if(print) cout << "Failed to find square." << endl;
			return result;
		}
	}
}

vector<corner> findTriangles(Mat& img_thr, Mat& img_cor, vector<corner> corners, bool print){
	unsigned int i = 0, index = 0;
	float avg_err = 0.0, best_err = 100.0;
	char count = 0;
	vector<corner> temp,result;
	corner center, test;
	bool first = false;

	while(corners.size()>2){
		for(i = 1; i<corners.size(); i++){
			if(abs(corners[i].dist-corners[i-1].dist)<= 40 && first == false){
				count++;
				first = true;
				index = i-1;
				avg_err += abs(corners[i].dist-corners[index].dist);
				
			}
			else if(abs(corners[i].dist-corners[index].dist)<= 40 && first==true){
				count++;
				avg_err += abs(corners[i].dist-corners[index].dist);
			}
			else
			{
				count=0;
				first = false;
				avg_err = 0;
			}

			if(count==2){
				for(unsigned int j = index; j<index+3; j++){
						temp.push_back(corners[j]);
				}

				if( (avg_err/3<best_err) && (triangleMatch(img_cor,cvtPoint(temp),0.1,print)) ){ 
					for(unsigned int j = index; j<index+3; j++){
						test.pos.x += corners[j].pos.x;
						test.pos.y += corners[j].pos.y;
					}

					test.pos.x /= 3;
					test.pos.y /= 3;

					if(img_thr.at<uchar>(test.pos.y,test.pos.x)==0){
						best_err = avg_err/3;

						result.clear();
						for(unsigned int j = index; j<index+3; j++){
							result.push_back(corners[j]);
						}

						if(print){
							cout << endl << "Found new best triangle: " << endl;
							cout << endl << "Corners vector is size " << corners.size() << "." << endl;
							for(i = 0; i<corners.size(); i++){
								cout << "x: " << corners[i].pos.x << " y: " << corners[i].pos.y << " dist: " << corners[i].dist << endl;
							}
										
							cout << "Best triangle error so far: " << best_err << ", Index:" << index << endl;
						}
					}

					test.pos.x = 0;
					test.pos.y = 0;
				}

				temp.clear();
				avg_err = 0;
				first = false;
				i = index+1;
			}
		}
		corners.pop_back();	
		center = global_center(corners);

		for(i = 0; i<corners.size(); i++){
			corners[i].dist = sqrt(pow(corners[i].pos.x-center.pos.x,2)+pow(corners[i].pos.y-center.pos.y,2));
		}

		sort(corners.begin(),corners.end(),dist_cmp);	
	}

	if(result.size()>0){
		return result;
	}
	else{
		for(unsigned int j = 0; j<corners.size(); j++){
			temp.push_back(corners[j]);
		}

		if(triangleMatch(img_cor,cvtPoint(temp),0.1,print)){
			for(i = 0; i<corners.size();i++){
				circle(img_cor,Point(corners[i].pos.x,corners[i].pos.y),3,Scalar(0,255,0),-1,0);
			}

			if(print) cout << "Defaulting to triangle consisting of the three corners closest to the total center of mass of all the corners." << endl;
			temp.clear();
			return temp;
		}
		else{
			if(print) cout << "Failed to find square." << endl;
			return result;
		}
	}
}

bool squareMatch(Mat& img_cor, vector<Point> points, float limit, bool print){

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
	double match = matchShapes(cnt[0],cnt[1],1,0);

	if(match<limit){
		if(print) cout << "Square match: " << match << endl;
		drawContours(img_cor,cnt,1,Scalar(255,0,255),1,LINE_8,noArray(),INT_MAX,Point(0,0));
		return 1;
	}
	else
		return 0;
}

bool triangleMatch(Mat& img_cor, vector<Point> points, float limit, bool print){

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

	double match = matchShapes(cnt[0],cnt[1],1,0);

	//if(print) cout << endl << "Triangle match: " << match << endl;

	if(match<limit){
		if(print) cout << "Triangle match: " << match << endl;
		drawContours(img_cor,cnt,1,Scalar(255,0,255),1,LINE_8,noArray(),INT_MAX,Point(0,0));
		return 1;
	}
	else
		return 0;
}

corner find_square2(vector<corner> corners, Mat& img_cor, Mat& img_thr, float limit) {
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

						match = squareMatch2(cvtPoint(temp));

						if ( (match < best_match) && (match < limit) && (match!=0) ) {

							corner temp_center = global_center(temp);
							
							if (img_thr.at<uchar>(temp_center.pos.y,temp_center.pos.x)<80) {
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
	/*
	if (c_center.pos.x!=0 && c_center.pos.y!=0){
		cout << "best square match: " << best_match << endl;
	}
	else cout << "failed to find good enough square" << endl;
	*/
	return c_center;
}

corner find_triangle2(vector<corner> corners, Mat& img_cor, Mat& img_thr, float limit) {
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

					match = triangleMatch2(cvtPoint(temp));

					if ( (match < best_match) && (match < limit) && (match!=0) ) {
						corner temp_center = global_center(temp);
						if (img_thr.at<uchar>(temp_center.pos.y,temp_center.pos.x)<80) {
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
			c_center.pos.x += result[j].pos.x;
			c_center.pos.y += result[j].pos.y;
			circle(img_cor,Point(result[j].pos.x,result[j].pos.y),3,Scalar(0,255,0),-1,0);
		}

		c_center.pos.x /= result.size();
		c_center.pos.y /= result.size();
	}
	/*
	if (c_center.pos.x!=0 && c_center.pos.y!=0){
		cout << "best triangle match: " << best_match << endl;
	}
	else cout << "failed to find good enough triangle" << endl;
	*/
	return c_center;
}

double squareMatch2(vector<Point> points){

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
	return matchShapes(cnt[0],cnt[1],1,0);
}

double triangleMatch2(vector<Point> points){

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

	return matchShapes(cnt[0],cnt[1],1,0);

}
