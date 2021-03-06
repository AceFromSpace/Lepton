#ifndef TEXTTHREAD
#define TEXTTHREAD

#include <ctime>
#include <stdint.h>
#include <iostream>
#include <fstream>
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
  void switchon_rescale();
  void switchon_subbg();
  void switchon_concave();
  void switchon_save_prev();
  void switchon_test();
  void get_BG();

  double draw_convex_hull(Mat image, int biggest);
  void make_snapshot();
  void histogram_alternative(Mat image);
  double counting_contour(Mat image,Mat mask);
  void separate_hand();//make mask from the biggest contour and use it with orginal image
  Mat rescale_hand(Mat image_mask);

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
   Mat Canny_conts;
   Mat opencvmat_values;
   Mat background;
   Mat hand_convex;
   std::vector<Mat> sequence;
   Point top_hand;
   int hist[3][256];
   int hottest_point;
   int coolest_point;

   bool mode;//0-normal , 1- binary
   bool mode_hull;
   bool mediane_on;
   bool histogram_on;
   bool counting_contours_on;
   bool draw_line;
   bool sub_background;
   bool recognize;
   bool rescale;
   bool mode_concave;
   bool save_prev;
   bool mode_test;
   int ppmode;

   int width;
   int height;
   const int *colormap;

   int slider_value_binary;
   int slider_value_canny;
   int depth_thresh;
   double ratioxy;

   Mat                                  cut_wirst(Mat img_hand, std::vector<std::vector<Point> > conto, int biggest);
   int recognize_gesture(int conv_points, double hull);
   void postprocessing(Mat image);
   void                                 mr_skeleton(Mat input, Mat &output);
   void                                 finding_edges(Mat input, Mat &output);
   void                                 save_hist();
   bool                                 find_direction(Mat img_hand, Point point_throught);//false-left ,true -right
   Mat                                  get_mask_classic(); //by threshold +biggest contour
   Mat                                  get_cont_and_mask(Mat image_mask);
   Mat                                  correct_mask(Mat mask_to_correct);
   Vec4f                                draw_fit_line(Mat image_hand, std::vector<cv::Point> contour);
   int                                  calc_wirst(Mat img_hand, int* tab_of_wirsts, int size);//returns index of wirst
   std::vector<std::vector<cv::Point> > get_vector_with_conts(Mat image_mask_and_conts);
   int                                  return_biggest_index(std::vector<std::vector<Point> > conto);
   Mat                                  sub_BG();
   int                                  concave(Mat image_cont, std::vector<std::vector<Point> > conto, int biggest);
   double                               calc_angle(Point point_begin, Point point_end, Point point_conv);
   void                                 manage_history(Mat image);
   void                                 contours_on_fingers(Mat image,Point fingers_begin,Point fingers_end);
   void                                 game();
   bool                                 is_Mat_empty(Mat img);
   void                                 update_depth_threshold(Mat img_hand, std::vector<Point>  conto);
   void                                 test(int number_of_concave, double circle_to_field , double circut_to_field, int reco_sign);
   double                               circut_to_field_ratio(Mat image);
   uint8_t result[PACKET_SIZE*PACKETS_PER_FRAME];
   uint16_t *frameBuffer;

};

#endif
