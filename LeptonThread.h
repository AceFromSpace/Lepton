#ifndef TEXTTHREAD
#define TEXTTHREAD

#include <ctime>
#include <stdint.h>
#include <iostream>

#include <QThread>
#include <QtCore>
#include <QPixmap>
#include <QImage>
#include <QSlider>

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
  void switchon_measure();
  void switchoff_measure();
  void switchon_mediane();
	
signals:

  void updateText(QString);
  void updateImage(QImage);

private:

  QImage myImage;
  const int *colormap;
  int mode;//0-normal , 1- binary
  int slider_value;
  int ppmode;
  bool measure;
  bool mediane_on;
  
  void duplicate_edges(QImage input ,QImage &output);
  void dilatation(QImage input, QImage &output,int kernel_size,int iterations);
  void cross_dilatation(QImage input, QImage &output,int kernel_size,int iterations);
  void erosion(QImage input, QImage &output,int kernel_size,int iterations);
  void cross_erosion(QImage input, QImage &output,int kernel_size,int iterations);
  void sobel(QImage input, QImage &output,int kernel_size);//kernel size 5 or 3 only
  void mr_skeleton(QImage input, QImage &output);
  void bit_not(QImage input,QImage &output);
  void bit_and(QImage input1,QImage input2, QImage &output);
  void bit_or(QImage input1,QImage input2, QImage &output);
  void get_black(QImage &inputoutput);
  void draw_cross_center(QImage &inputoutput);//measurment symbol
  double temp_measure(int val,int amb_temp);
  void mediane(QImage input, QImage &output,int kernel_size);
  void bubble_sort(int tab[],int size);
  int find_mediane_in_tab(int tab[],int size);
  
  uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
  uint16_t *frameBuffer;

};

#endif
