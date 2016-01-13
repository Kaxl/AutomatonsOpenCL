#include "Display.h"

Display::Display(unsigned int* data,
                 unsigned int sizex,
                 unsigned int sizey,
                 unsigned int max) : data_(data),sizeX_(sizex),sizeY_(sizey),max_(max),
                                     img_(cv::Mat(cv::Size(sizeX_,sizeY_),CV_8UC3,cv::Scalar(0,0,0))) {
	cv::namedWindow("Window",CV_WINDOW_KEEPRATIO);
}

Display::~Display() { cv::destroyWindow("Window"); }

void Display::show() {
	//static float gradient;
	//static color col;
	for (unsigned int y=0;y<sizeY_;y++ ) {
		for (unsigned int x=0;x<sizeX_;x++) {
			//gradient = (((float)data_[y*sizeX_+x])/((float)max_))*250.0;
			//groundColorMix(col,gradient,0.0,250.0);
			img_.data[(x*3)+y*sizeX_*3]   = (char)(data_[y * sizeX_ + x] * 250.0);
			img_.data[(x*3)+y*sizeX_*3+1] = (char)(data_[y * sizeX_ + x] * 250.0);
			img_.data[(x*3)+y*sizeX_*3+2] = (char)(data_[y * sizeX_ + x] * 250.0);
		}
	}
	cv::imshow("Window",img_);
	cv::waitKey(1);
}

char Display::waitForKey(){ return cv::waitKey(0); }

void Display::groundColorMix(color &color,float x,float min,float max) {
	float posSlope = (max-min)/60;
	float negSlope = (min-max)/60;
	if (x < 60) {
		color.r = max;
		color.g = posSlope*x+min;
		color.b = min;
		return;
	} else if (x < 120) {
		color.r = negSlope*x+2*max+min;
		color.g = max;
		color.b = min;
		return;
	} else if (x < 180) {
		color.r = min;
		color.g = max;
		color.b = posSlope*x-2*max+min;
		return;
	} else if (x < 240) {
		color.r = min;
		color.g = negSlope*x+4*max+min;
		color.b = max;
		return;
	} else if (x < 300) {
		color.r = posSlope*x-4*max+min;
		color.g = min;
		color.b = max;
		return;
	} else {
		color.r = max;
		color.g = min;
		color.b = negSlope*x+6*max;
		return;
	}
}
