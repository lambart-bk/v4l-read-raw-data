#include"opencv/cv.h"
#include"opencv2/opencv.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv/highgui.h"
#include"opencv2/core/core.hpp"
#include"opencv2/videoio.hpp"
#include"opencv/cxcore.h"

#include"iostream"
#include<vector>


using namespace std;


int main(int argc,char **argv){
 
 
    cv::VideoCapture cap(0);
    if (cap.isOpened())
      cout << "camera is opened"<< endl;
    else 
      {
      	cout << "camera is not opened" << endl;
      	exit(0);
      	}

	
    
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CV_CAP_PROP_FPS, 30.0);
 


    cv::Mat videoPlay;
    int count = 15000;

    while (1)
    {
	    cap >> videoPlay;
	    cv::imshow("videoplay", videoPlay);
	    cv::waitKey(30);
	    //count --;
    }

    cap.release();


 
return 0;
  }