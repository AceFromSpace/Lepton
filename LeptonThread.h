#ifndef TEXTTHREAD
#define TEXTTHREAD

#include <ctime>
#include <stdint.h>
#include <iostream>
#include<fstream>
#include <ctime>
#include <string>

#include <QThread>
#include <QtCore>
#include <QPixmap>
#include <QImage>
#include <QSlider>
#include <QMessageBox>

#include <opencv2/opencv.hpp>

using namespace cv;


#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)

class LeptonThread : public QThread
{
  Q_OBJECT;

public:
  LeptonThread();
  ~LeptonThread();

  void run();
  bool return_mode();//for enabling slider
  

public slots:

  void performFFC();
  void enableAGC();
  void disableAGC();
  void change_colormap_rainbow();
  void change_colormap_gray();
  void change_colormap_iron();
  void set_normal_mode();
  void set_binary_mode();
  void change_slider_value(int value);

  void switchon_dilatation();
  void switchon_erosion();
  void switchon_open();
  void switchon_close();
  void switchon_sobel();
  void switchon_skeleton();
  void switchon_learn();
  void switchoff_learn();
  void switchon_mediane();
  void switchon_histogram();
  void switchon_hull();

  void draw_convex_hull(Mat image,std::vector<std::vector<Point> > conto);
  void make_snapshot();
  void find_countour();
  void separate_hand();//make mask from the biggest contour and use it with orginal image

signals:

  void updateText(QString);
  void updateImage(QImage);

private:

   Mat opencvmat_base;//value 0-255 1 channel
   Mat opencvmat;
   int hist[3][256];
   Mat image_histogram;
   int mode_hull;
   Mat cont;
   int width;
   int height;
   const int *colormap;
   int mode;//0-normal , 1- binary
   int slider_value;
   int ppmode;
   bool learn;
   bool mediane_on;
   bool histogram_on;



  
  void postprocessing();
  void mr_skeleton(Mat input, Mat &output);
  void finding_edges(Mat input, Mat &output);
  void show_hist(Mat image);
  void get_hist(Mat image);
  void clean_hist();
  void save_hist();
  
  uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
  uint16_t *frameBuffer;

};

#endif
