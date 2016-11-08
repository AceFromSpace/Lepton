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
  void change_slider_value_binary(int value);
  void change_slider_value_canny(int value);

  void switchon_dilatation();
  void switchon_erosion();
  void switchon_open();
  void switchon_close();
  void switchon_sobel();
  void switchon_skeleton();
  void switchon_mediane();
  void switchon_histogram();
  void switchon_hull();
  void switchon_conting_countour();
  void switchon_line();
  void switchon_recognize();

  double draw_convex_hull(Mat image,std::vector<std::vector<Point> > conto,int biggest);
  void make_snapshot();
  void histogram_alternative(Mat image);
  double counting_contour(Mat image,Mat mask);
  void separate_hand();//make mask from the biggest contour and use it with orginal image

signals:

  void updateText(QString);
  void updateTextContours(QString);
  void updateTextContoursHull(QString);
  void updateTextReco(QString);
  void updateImage(QImage);

private:

   Mat opencvmat_base;//values 0-255 1 channel
   Mat opencvmat;
   Mat cont;
   Mat mask;
   Mat image_hull;
   Mat image_histogram;
   Mat image_params;

   int hist[3][256];

   bool mode;//0-normal , 1- binary
   bool mode_hull;
   bool mediane_on;
   bool histogram_on;
   bool counting_contours_on;
   bool draw_line;
   bool recognize;
   int ppmode;

   int width;
   int height;
   const int *colormap;

   int slider_value_binary;
   int slider_value_canny;


   void recognize_gesture(double hull, double conts);
   void postprocessing(Mat image);
   void mr_skeleton(Mat input, Mat &output);
   void finding_edges(Mat input, Mat &output);
   void save_hist();
  
   uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
   uint16_t *frameBuffer;

};

#endif
