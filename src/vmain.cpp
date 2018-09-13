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
  /*   cv::VideoCapture cap(0);
    
    if (cap.isOpened())
      cout << "camera is opened"<< endl;
    else 
      {
      	cout << "camera is not opened" << endl;
      	exit(0);
      	}
    //cap.set(16,0);
    cout<<cap.get(atoi(argv[1]))<<endl;
	
    
    //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    //cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    //cap.set(CV_CAP_PROP_FPS, 30.0);
 
    cv::Mat frame;//(512,480,CV_16UC1);
    cv::Mat fr;
    
    cout<<frame.depth()<<endl;
    for (int i = 1; ;i++)
    {
	    cap >> frame;
	    std::vector<cv::Mat> channel;
	    cv::split(frame,channel);
	    cv::imwrite ("./c1.jpeg",channel[0]);
	    cv::imwrite ("./c2.jpeg",channel[1]);
	    cv::imwrite ("./c3.jpeg",channel[2]);
	    //cap.read(frame);
	    //cvtColor(frame,fr,COLOR_BGR2YUV);
	    if(frame.empty())
		    continue;
	    cout<<frame.rows<<","<<frame.cols<<endl;
	    cout<<frame.depth()<<endl;
	    cout<<frame.channels()<<endl;
	    if(i % 5 == 0)
	    {
	    int max=0;
	    for(int i=0;i<2;i++)
	    	for(int j=0;j<2;j++)
	    		for(int k=0;k<frame.channels();k++)
	    {
	    int t=(int)frame.at<cv::Vec3b>(i,j)[k];
	    //cout<<t<<endl;
	    if(t>max)
	    	max=t;
	    }
	    cout<<max<<endl;
	      cv::imwrite ("./1.jpeg",frame);
	      break;
	    }
	}
    cap.release();
 
	//system("pause");

 
return 0;*/