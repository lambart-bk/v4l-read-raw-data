#include"opencv/cv.h"
#include"opencv2/opencv.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv/highgui.h"
#include"opencv2/core/core.hpp"
#include"opencv2/video/video.hpp"
#include"opencv2/imgproc/imgproc.hpp"

#include"opencv/cxcore.h"

#include"iostream"
#include<vector>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include<time.h>
//#include <jpeglib.h>

#include"fcntl.h"
#include"termios.h"
#include"iomanip"


#define FALSE false
#define TRUE true

using namespace std;
using namespace cv;


void ttyconfig(int ttyid)
{
  struct termios tios;
  tios.c_oflag=0;
  tios.c_iflag=0;
  tios.c_lflag=0;
  cfsetispeed(&tios,0010002);
  cfsetospeed(&tios,0010002);
  
  //data bits
  tios.c_cflag&=~0x30;
  tios.c_cflag|=CS8;
  
  //stop bits
  tios.c_cflag&=~CSTOPB;
  
  //parity
  tios.c_cflag&=~PARENB;
  
  tios.c_cc[VMIN]=0;
  tios.c_cc[VTIME]=50;
  
  tcsetattr(ttyid,TCSANOW,&tios);
  tcflush(ttyid,TCIOFLUSH);
  
}

void setMode(std::string ttypath,int mode=1)
{
  unsigned char buf1[9]={0xAA,0x05,0x00,0x15,0x01,0x00,0xC5,0xEB,0xAA}; 
  unsigned char buf2[9]={0xAA,0x05,0x00,0x15,0x01,0x01,0xC6,0xEB,0xAA};
  unsigned char buf3[9]={0xAA,0x05,0x00,0x16,0x01,0x00,0xC6,0xEB,0xAA};
  unsigned char buf4[9]={0xAA,0x05,0x00,0x16,0x01,0x02,0xC8,0xEB,0xAA};
  unsigned char *buf;
  
  if(mode==1) //auto rectify off
    buf=buf1; 
  else if(mode==2) //auto rectify on
    buf=buf2; 
  else if(mode==3) //shutter rectify
    buf=buf3; 
  else if(mode==4) //background rectify
    buf=buf4; 
  
  int ttyid=open(ttypath.c_str(),O_RDWR|O_NOCTTY|O_NONBLOCK);
  if(ttyid<0)
  {
    //sduo usermod -aG dialout currentusername
    std::cout<<"tty open failed\n"<<std::endl;
    exit(-1);
  }
  ttyconfig(ttyid);
  
  int sz=sizeof(buf1);
  unsigned char res_buf[9];
  while(write(ttyid,buf,sz)<0)
    std::cout<<"<0"<<std::endl;;
  //int res=write(ttyid,buf,sz) ;
  //std::cout<<"   "<<res<<std::endl;
  while(1)
  {
    if(read(ttyid,res_buf,sizeof(res_buf))>0)
    {
      //for(int i=0;i<sizeof(res_buf);i++)
	//std::cout<<std::setbase(16)<<"0x"<<(int)res_buf[i]<<std::endl;
      break;
    }
  }
  
  close(ttyid);
  
}


void quit(const char * msg)
{
  fprintf(stderr, "[%s] %d: %s\n", msg, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

int xioctl(int fd, int request, void* arg)
{
  for (int i = 0; i < 100; i++) {
    int r = ioctl(fd, request, arg);
    if (r != -1 || errno != EINTR) return r;
  }
  return -1;
}

typedef struct {
  uint8_t* start;
  size_t length;
} buffer_t;

typedef struct {
  int fd;
  uint32_t width;
  uint32_t height;
  size_t buffer_count;
  buffer_t* buffers;
  buffer_t head;
} camera_t;
char * getnamefromtime()
{
  time_t t;
  time(&t);
  char *str=(char*)malloc(30*sizeof(char));
  struct tm *gt=localtime(&t);
  struct timeval us;
  gettimeofday(&us,NULL);
  sprintf(str,"./data/%d-%d-%d-%d-%d-%d-%d.png",gt->tm_year+1900,gt->tm_mon+1,gt->tm_mday,gt->tm_hour,gt->tm_min,gt->tm_sec,us.tv_usec%100);
  //cout<<asctime(gmtime(&t))<<endl;
  return str;

}

camera_t* camera_open(const char * device, uint32_t width, uint32_t height)
{
  int fd = open(device, O_RDWR | O_NONBLOCK, 0);
  if (fd == -1) quit("open");
  camera_t* camera = (camera_t*)malloc(sizeof (camera_t));
  camera->fd = fd;
  camera->width = width;
  camera->height = height;
  camera->buffer_count = 0;
  camera->buffers = NULL;
  camera->head.length = 0;
  camera->head.start = NULL;
  return camera;
}


void camera_init(camera_t* camera) {
  struct v4l2_capability cap;
  if (xioctl(camera->fd, VIDIOC_QUERYCAP, &cap) == -1) quit("VIDIOC_QUERYCAP");
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) quit("no capture");
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) quit("no streaming");

  struct v4l2_cropcap cropcap;
  memset(&cropcap, 0, sizeof cropcap);
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_CROPCAP, &cropcap) == 0) {
    struct v4l2_crop crop;
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;
    if (xioctl(camera->fd, VIDIOC_S_CROP, &crop) == -1) {
      // cropping not supported
    }
  }

  struct v4l2_format format;
  memset(&format, 0, sizeof format);
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = camera->width;
  format.fmt.pix.height = camera->height;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  format.fmt.pix.field = V4L2_FIELD_NONE;
  if (xioctl(camera->fd, VIDIOC_S_FMT, &format) == -1) quit("VIDIOC_S_FMT");

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (xioctl(camera->fd, VIDIOC_REQBUFS, &req) == -1) quit("VIDIOC_REQBUFS");
  camera->buffer_count = req.count;
  camera->buffers = (buffer_t*)calloc(req.count, sizeof (buffer_t));

  size_t buf_max = 0;
  for (size_t i = 0; i < camera->buffer_count; i++) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (xioctl(camera->fd, VIDIOC_QUERYBUF, &buf) == -1)
      quit("VIDIOC_QUERYBUF");
    if (buf.length > buf_max) buf_max = buf.length;
    camera->buffers[i].length = buf.length;
    camera->buffers[i].start =(uint8_t*)
      mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
           camera->fd, buf.m.offset);
    if (camera->buffers[i].start == MAP_FAILED) quit("mmap");
  }
  camera->head.start =(uint8_t*) malloc(buf_max);
}


void camera_start(camera_t* camera)
{
  for (size_t i = 0; i < camera->buffer_count; i++) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (xioctl(camera->fd, VIDIOC_QBUF, &buf) == -1) quit("VIDIOC_QBUF");
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_STREAMON, &type) == -1)
    quit("VIDIOC_STREAMON");
}

void camera_stop(camera_t* camera)
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_STREAMOFF, &type) == -1)
    quit("VIDIOC_STREAMOFF");
}

void camera_finish(camera_t* camera)
{
  for (size_t i = 0; i < camera->buffer_count; i++) {
    munmap(camera->buffers[i].start, camera->buffers[i].length);
  }
  free(camera->buffers);
  camera->buffer_count = 0;
  camera->buffers = NULL;
  free(camera->head.start);
  camera->head.length = 0;
  camera->head.start = NULL;
}

void camera_close(camera_t* camera)
{
  if (close(camera->fd) == -1) quit("close");
  free(camera);
}


int camera_capture(camera_t* camera)
{
  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if (xioctl(camera->fd, VIDIOC_DQBUF, &buf) == -1) return FALSE;
  memcpy(camera->head.start, camera->buffers[buf.index].start, buf.bytesused);
  camera->head.length = buf.bytesused;
  if (xioctl(camera->fd, VIDIOC_QBUF, &buf) == -1) return FALSE;
  return TRUE;
}

int camera_frame(camera_t* camera, struct timeval timeout) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(camera->fd, &fds);
  int r = select(camera->fd + 1, &fds, 0, 0, &timeout);
  if (r == -1) quit("select");
  if (r == 0) return FALSE;
  return camera_capture(camera);
}

/*
void jpeg(FILE* dest, uint8_t* rgb, uint32_t width, uint32_t height, int quality)
{
  JSAMPARRAY image;
  image = (JSAMPARRAY)calloc(height, sizeof (JSAMPROW));
  for (size_t i = 0; i < height; i++) {
    image[i] = (JSAMPROW)calloc(width * 3, sizeof (JSAMPLE));
    for (size_t j = 0; j < width; j++) {
      image[i][j * 3 + 0] = rgb[(i * width + j) * 3 + 0];
      image[i][j * 3 + 1] = rgb[(i * width + j) * 3 + 1];
      image[i][j * 3 + 2] = rgb[(i * width + j) * 3 + 2];
    }
  }

  struct jpeg_compress_struct compress;
  struct jpeg_error_mgr error;
  compress.err = jpeg_std_error(&error);
  jpeg_create_compress(&compress);
  jpeg_stdio_dest(&compress, dest);

  compress.image_width = width;
  compress.image_height = height;
  compress.input_components = 3;
  compress.in_color_space = JCS_RGB;
  jpeg_set_defaults(&compress);
  jpeg_set_quality(&compress, quality, TRUE);
  jpeg_start_compress(&compress, TRUE);
  jpeg_write_scanlines(&compress, image, height);
  jpeg_finish_compress(&compress);
  jpeg_destroy_compress(&compress);

  for (size_t i = 0; i < height; i++) {
    free(image[i]);
  }
  free(image);
}*/


int minmax(int min, int v, int max)
{
  return (v < min) ? min : (max < v) ? max : v;
}

int reverse(int val)
{
  int nVal=0;
  for(int i=0;i<8;i++)
  {
    int t=val%2;
    nVal+=t*pow(2,7-i);
    val=val/2;
  }
  return nVal;
} 

Mat enhance_img(Mat src)
{
  double minv = 0.0, maxv = 0.0;
  double *minp = &minv;
  double *maxp = &maxv;
  
  Mat dst(src.rows, src.cols, CV_16UC1);
  minMaxIdx(src, minp, maxp);
  for(int i = 0; i < src.rows; i++)
  {
    for(int j = 0; j < src.cols; j++)
    {
      double vaule = ((double)src.at<unsigned short>(i,j) - minv)/(maxv-minv)*65535;
      dst.at<unsigned short>(i,j) = (unsigned short)vaule;
    }
  }
   //imwrite("enhance_img.png", dst);
   return dst;
}

void yuyv2rgb(uint8_t* yuyv, uint32_t width, uint32_t height,char *name)
{
  cv::Mat img(height,width,CV_16UC1);
  Mat en_img;
  //uint8_t* rgb = (uint8_t*)calloc(width * height * 3, sizeof (uint8_t));
  for (size_t i = 0; i < height; i++) {
    for (size_t j = 0; j < width; j += 1) {
      size_t index = i * width + j;
      uint8_t x=yuyv[index * 2 + 0];      
      uint8_t y=yuyv[index * 2 + 1];
      //x=reverse(x);
      //y=reverse(y);
      //cout<<(int)y<<"......"<<(int)x<<"......"<<y%64<<endl;
      y=y%64;
      
      //printf("%d,%d  %d\n",x,y,(int)((x<<8)+y)/255);
      
      
      /*if(x>255)
      	x=255;
      if(x<0)
      	x=0;*/
      img.at<unsigned short>(i,j)=(int)(y*256+x)*4;
      //printf("****%d\n",(int)y*256+(int)x);
      
      
      
      /*int y0 = yuyv[index * 2 + 0] << 8;
      int u = yuyv[index * 2 + 1] - 128;
      int y1 = yuyv[index * 2 + 2] << 8;
      int v = yuyv[index * 2 + 3] - 128;
      rgb[index * 3 + 0] = minmax(0, (y0 + 359 * v) >> 8, 255);
      rgb[index * 3 + 1] = minmax(0, (y0 + 88 * v - 183 * u) >> 8, 255);
      rgb[index * 3 + 2] = minmax(0, (y0 + 454 * u) >> 8, 255);
      rgb[index * 3 + 3] = minmax(0, (y1 + 359 * v) >> 8, 255);
      rgb[index * 3 + 4] = minmax(0, (y1 + 88 * v - 183 * u) >> 8, 255);
      rgb[index * 3 + 5] = minmax(0, (y1 + 454 * u) >> 8, 255);*/
    }
  }
  cv::imwrite (name,img);
  //en_img = enhance_img(img);
  //imwrite(name, en_img);
  // cout<<img.depth()<<endl;
  //cv::imshow("img",en_img);
  cv::waitKey(33);
  //cout<<(int)img.at<unsigned short>(height/2,width/2)<<endl;
  //return rgb;
}



int main(int argc,char **argv){


  std::string ttypath="/dev/ttyUSB0";
  //setMode(ttypath,1));
  camera_t* camera = camera_open("/dev/video0",640,512);
  camera_init(camera);
  camera_start(camera);

  setMode(ttypath,1);
  
  
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  /* skip 5 frames for booting a cam */
  for (int i = 0; i < 5; i++) {
    camera_frame(camera, timeout);
  }
  

  while(1)
  {
    setMode(ttypath,3);
    usleep(10);
    setMode(ttypath,4);
    usleep(10);
    for(int i=0;i<5;i++)
    {
	  camera_frame(camera, timeout);
	  yuyv2rgb(camera->head.start, camera->width, camera->height,getnamefromtime());
	  //sleep(1);
	  //FILE* out = fopen("result.jpg", "w");
	  //jpeg(out, rgb, camera->width, camera->height, 100);
	  //fclose(out);
	  //free(rgb);
	  cv::waitKey(33);
    }
    std::cout<<"capture 5 pic in data/"<<std::endl;
    sleep(300);
    
  }

  camera_stop(camera);
  camera_finish(camera);
  camera_close(camera);
  return 0;
 
 
  }