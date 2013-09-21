/*
 * Author Justin Marple 
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
 
//OpenCV
#include <cv.h>
#include "csiopencv.c"

CvFont font;
IplImage *py, *pu, *pv, *pu_big, *pv_big, *image,* dstImage;


void cvCallBack(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	MMAL_BUFFER_HEADER_T *new_buffer;
   PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

   if (pData)
   {
     
      if (buffer->length)
      {

	      mmal_buffer_header_mem_lock(buffer);
 
 		//
		// *** PR : OPEN CV Stuff here !
		//
		int w=pData->pstate->width;	// get image size
		int h=pData->pstate->height;
		int h4=h/4;
		
		memcpy(py->imageData,buffer->data,w*h);	// read Y
		
		if (pData->pstate->graymode==0)
		{
			memcpy(pu->imageData,buffer->data+w*h,w*h4); // read U
			memcpy(pv->imageData,buffer->data+w*h+w*h4,w*h4); // read v
	
			cvResize(pu, pu_big, CV_INTER_NN);
			cvResize(pv, pv_big, CV_INTER_NN);  //CV_INTER_LINEAR looks better but it's slower
			cvMerge(py, pu_big, pv_big, NULL, image);
	
			cvCvtColor(image,dstImage,CV_YCrCb2RGB);	// convert in RGB color space (slow)
			
			//uchar* temp_ptr = &((uchar*)(dstImage->imageData + dstImage->widthStep*50))[50];
			CvScalar s = cvGet2D(dstImage, 50, 50);
	
			char text[255];
			sprintf(text, "RGB = %d , %d, %d", (int)s.val[2], (int)s.val[1], (int)s.val[0]);
			
			cvPutText(dstImage, text, cvPoint(50, 50), &font, cvScalar(255, 255, 255 ,0));
			printf("test1");
			cvShowImage("camcvWin", dstImage );
		}
		else
		{	
			
			cvShowImage("camcvWin", py); // display only gray channel
			
		}
		
		cvWaitKey(1);
		nCount++;		// count frames displayed
		
         mmal_buffer_header_mem_unlock(buffer);
      }
      else vcos_log_error("buffer null");
      
   }
   else
   {
      vcos_log_error("Received a encoder buffer callback with no state");
   }

   // release buffer back to the pool
   mmal_buffer_header_release(buffer);

   // and send one back to the port (if still open)
   if (port->is_enabled)
   {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->video_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the encoder port");
   }
}

int main(int argc, const char **argv)
{
	// Our main data storage vessel..
	RASPIVID_STATE state;
	
	MMAL_STATUS_T status = -1;
	MMAL_PORT_T *camera_video_port = NULL;
	MMAL_PORT_T *camera_still_port = NULL;
	MMAL_PORT_T *preview_input_port = NULL;
	MMAL_PORT_T *encoder_input_port = NULL;
	MMAL_PORT_T *encoder_output_port = NULL;
	
	time_t timer_begin,timer_end;
	double secondsElapsed;
	
	bcm_host_init();
	signal(SIGINT, signal_handler);

	// read default status
	default_status(&state);
	
	//TODO: OpenCV stuff here
	cvNamedWindow("camcvWin", CV_WINDOW_AUTOSIZE); 
	
	int w=state.width;
	int h=state.height;
	
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
    
    //TODO: END
    

	
	// create camera
	if (!create_camera_component(&state, cvCallBack))
	{
	   vcos_log_error("%s: Failed to create camera component", __func__);
	}
	else if ((status = raspipreview_create(&state.preview_parameters)) != MMAL_SUCCESS)
	{
	   vcos_log_error("%s: Failed to create preview component", __func__);
	   destroy_camera_component(&state);
	}
	else
	{			
		PORT_USERDATA callback_data;
		
		camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
		camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
	   
		VCOS_STATUS_T vcos_status;
		
		callback_data.pstate = &state;
		
		vcos_status = vcos_semaphore_create(&callback_data.complete_semaphore, "RaspiStill-sem", 0);
		vcos_assert(vcos_status == VCOS_SUCCESS);
		
		// assign data to use for callback
		camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_data;
        
        // init timer
  		time(&timer_begin); 

       
       // start capture
		if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
		{
		   goto error;
		}
		
		// Send all the buffers to the video port
		
		int num = mmal_queue_length(state.video_pool->queue);
		int q;
		for (q=0;q<num;q++)		
		{
		   MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.video_pool->queue);
		
		   if (!buffer)
		   		vcos_log_error("Unable to get a required buffer %d from pool queue", q);
		
			if (mmal_port_send_buffer(camera_video_port, buffer)!= MMAL_SUCCESS)
		    	vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);
		}
		
		
		// Now wait until we need to stop
		vcos_sleep(state.timeout);
	
error:

		mmal_status_to_int(status);
		
		
		// Disable all our ports that are not handled by connections
		check_disable_port(camera_still_port);
		
		if (state.camera_component)
		{
		   mmal_component_disable(state.camera_component);
		}	
	
		//destroy_encoder_component(&state);
		raspipreview_destroy(&state.preview_parameters);
		destroy_camera_component(&state);
		
	}
	if (status != 0)
	{
		raspicamcontrol_check_configuration(128);
	}	
	
	printf("ohyes");
	
	cvReleaseImage(&dstImage);
	cvReleaseImage(&pu);
	cvReleaseImage(&pv);
	cvReleaseImage(&py);
	cvReleaseImage(&pu_big);
	cvReleaseImage(&pv_big);
	
	return 0;
}


