/*
 * Author: Justin Marple 
 * Contact: jmarple@umass.edu
 * 
 * Date Created: September 18th 2013
 * 
 * Description:
 * This file is designed to process data from the CSI camera built into
 * the raspberry pi and have it be useable by the opencv library. 
 * 
 * Credits:
 * This file originated from an edited version of RaspiVid.c
 * Copyright (c) 2012, Broadcom Europe Ltd
 * Edits originally by Pierre Raufast, pierre.raufest@gmail.com
 * 	-- thinkrpi.wordpress.com
 */
 
//OpenCV
#include <cv.h>
#include "../src/csiopencv.c"

CvFont font;
IplImage *dstImage;

void myCallBack(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	
	dstImage = cvQueryRPiFrame(port, buffer);
   
	//uchar* temp_ptr = &((uchar*)(dstImage->imageData + dstImage->widthStep*50))[50];
	CvScalar s = cvGet2D(dstImage, 50, 50);

	char text[255];
	sprintf(text, "RGB = %d , %d, %d", (int)s.val[2], (int)s.val[1], (int)s.val[0]);
	
	cvPutText(dstImage, text, cvPoint(50, 50), &font, cvScalar(255, 255, 255 ,0));
	
	cvShowImage("camcvWin", dstImage );

	cvWaitKey(1);  
}

int main(int argc, const char **argv)
{	
	//Create New Window
	cvNamedWindow("camcvWin", CV_WINDOW_AUTOSIZE); 
	
	//Setup
	int w=320;
	int h=240;	

	dstImage = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);
    	
    //Start RPi Camera call back
    if(cvStartRPiCAM(myCallBack, w, h) == 1)
    {    		
		//Run Until..
		while(-1 == -1){}
	}

    cvCloseRPiCAM();

	cvReleaseImage(&dstImage);	
	
	return 0;
}


