#ifndef Display_H_
#define Display_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

struct color{
	float r;
	float g;
	float b;
};

class Display {
public:
	Display(unsigned int* data,unsigned int sizex,unsigned int sizey,unsigned int max);
	~Display();
	void show();
	char waitForKey();
	static void groundColorMix(color &col,float x,float min,float max);
private:
	unsigned int *data_;
	unsigned int sizeX_;
	unsigned int sizeY_;
	unsigned int max_;
	cv::Mat img_;
};

#endif /* Display_H_ */
