/*
 *@author Justin Marple 
 * Contact: jmarple@umass.edu
 * 
 * Date Created: September 18th 2013
 * 
 * Description:
 * This file is designed to process data from the CSI camera built into
 * the raspberry pi and have it be useable by the opencv library. 
 * Camcv.c is the main file that is a modified version of of the 
 * original open source code for the camera.  
 * 
 */

CvFont font;
IplImage *py, *pu, *pv, *pu_big, *pv_big, *image,* dstImage;

/* This function will process the image from the CSI port of the raspberry pi */
void openCVProcess(IplImage *image)
{
	//uchar* temp_ptr = &((uchar*)(dstImage->imageData + dstImage->widthStep*50))[50];
	CvScalar s = cvGet2D(image, 50, 50);

	char text[255];
	sprintf(text, "RGB = %d , %d, %d", (int)s.val[2], (int)s.val[1], (int)s.val[0]);
	
	cvPutText(image, text, cvPoint(50, 50), &font, cvScalar(255, 255, 255 ,0));
	
	cvShowImage("camcvWin", image );
}

/* This function will be what initates everything within opencv */
void openCVInit()
{
	/* Windows */
	cvNamedWindow("camcvWin", CV_WINDOW_AUTOSIZE); 
	
	int w = 320;
	int h = 240;
	
	/* Images */
	dstImage = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);
	py = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);		// Y component of YUV I420 frame
	pu = cvCreateImage(cvSize(w/2,h/2), IPL_DEPTH_8U, 1);	// U component of YUV I420 frame
	pv = cvCreateImage(cvSize(w/2,h/2), IPL_DEPTH_8U, 1);	// V component of YUV I420 frame
	pu_big = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	pv_big = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	image = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);	// final picture to display
	
	/* Fonts */
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);
}

/* When the program closes */
void openCVClose()
{
	cvReleaseImage(&dstImage);
	cvReleaseImage(&pu);
	cvReleaseImage(&pv);
	cvReleaseImage(&py);
	cvReleaseImage(&pu_big);
	cvReleaseImage(&pv_big);
}
