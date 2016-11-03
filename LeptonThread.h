#ifndef TEXTTHREAD
#define TEXTTHREAD

#include <ctime>
#include <stdint.h>
#include <iostream>
#include <ctime>
#include <string>

#include <QThread>
#include <QtCore>
#include <QPixmap>
#include <QImage>
#include <QSlider>

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

  void make_snapshot();

signals:

  void updateText(QString);
  void updateImage(QImage);

private:

   Mat opencvmat;
   int width;
   int height;
   const int *colormap;
   int mode;//0-normal , 1- binary
   int slider_value;
   int ppmode;
   bool learn;
   bool mediane_on;


  
  void postprocessing();
  void mr_skeleton(Mat input, Mat &output);
  void finding_edges(Mat input, Mat &output);
  
  uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
  uint16_t *frameBuffer;

};

#endif
