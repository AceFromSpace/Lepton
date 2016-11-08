#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QMessageBox>

#include <QColor>
#include <QSlider>
#include <QLabel>
#include <QtDebug>
#include <QString>
#include <QPushButton>
#include <QCheckBox>
#include <QMenu>
#include <QMenuBar>

#include "LeptonThread.h"
#include "MyLabel.h"
#include <iostream>
#include <opencv2/opencv.hpp>

         
using namespace cv;
void create_buttons(QWidget *mywig,QThread *thr);
void create_sliders(QWidget *mywig,QThread *thr);
void create_checkboxes(QWidget *mywig,QThread *thr);
void create_screen(QWidget *mywig,QThread *thr);
void create_labels(QWidget *mywig,QThread *thr);
int main( int argc, char **argv )
{

	QApplication a( argc, argv );
	
	QWidget *myWidget = new QWidget;
    myWidget->setGeometry(10, 50, 800, 350);
    LeptonThread *thread = new LeptonThread();

    QImage myImage= QImage(320, 240, QImage::Format_RGB888);
    QRgb green = qRgb(0,255,0);
    for(int i=0;i<320;i++) {
        for(int j=0;j<240;j++) {
            myImage.setPixel(i, j, green);
        }
    }

    MyLabel myLabel(myWidget);
    myLabel.setGeometry(10, 10, 320, 240);
    myLabel.setPixmap(QPixmap::fromImage(myImage));
    QObject::connect(thread, SIGNAL(updateImage(QImage)), &myLabel, SLOT(setImage(QImage)));

    create_screen(myWidget,thread);
    create_buttons(myWidget,thread);
    create_sliders(myWidget,thread);
    create_checkboxes(myWidget,thread);
    create_labels(myWidget,thread);

	thread->start();
	
	myWidget->show();

	return a.exec();
}
void create_buttons(QWidget *mywig, QThread *thr)
{
    QPushButton *button_FFC = new QPushButton("Perform FFC", mywig);
    button_FFC->setGeometry(130, 290-35, 100, 30);
    QPushButton *button_rainbow = new QPushButton("Rainbow Scale", mywig);
    button_rainbow->setGeometry(10, 290-35, 100, 30);
    QPushButton *button_grayscale = new QPushButton("Gray Scale", mywig);
    button_grayscale->setGeometry(10, 290, 100, 30);
    QPushButton *button_iron = new QPushButton("Iron Scale", mywig);
    button_iron ->setGeometry(10, 320, 100, 30);


    QPushButton *button_binarization = new QPushButton("Binarization", mywig);
    button_binarization ->setGeometry(130, 290, 100, 30);
    QPushButton *button_normal = new QPushButton("Normal Diplay", mywig);
    button_normal ->setGeometry(130, 325, 100, 30);

    QPushButton *button_dilatation = new QPushButton("Dilatation", mywig);
    button_dilatation ->setGeometry(260, 255, 100, 30);
    QPushButton *button_erosion = new QPushButton("Erosion", mywig);
    button_erosion ->setGeometry(260, 290, 100, 30);
    QPushButton *button_open = new QPushButton("Open", mywig);
    button_open ->setGeometry(260, 320, 100, 30);

    QPushButton *button_close = new QPushButton("Close", mywig);
    button_close ->setGeometry(390, 255, 100, 30);
    QPushButton *button_sobel = new QPushButton("Sobel", mywig);
    button_sobel ->setGeometry(390, 290, 100, 30);
    QPushButton *button_skeleton = new QPushButton("Skeleton", mywig);
    button_skeleton ->setGeometry(390, 320, 100, 30);


    QPushButton *button_mediane = new QPushButton("Mediane",mywig);
    button_mediane -> setGeometry(520,320,100,30);

    QPushButton *button_agc_en = new QPushButton("AGC ON",mywig);
    button_agc_en-> setGeometry(650,255,100,30);
    QPushButton *button_agc_dis = new QPushButton("AGC OFF",mywig);
    button_agc_dis-> setGeometry(650,290,100,30);
    QPushButton *button_snap_shot = new QPushButton("Snapshot",mywig);
    button_snap_shot -> setGeometry(650,320,100,30);

    QPushButton *button_separate_hand = new QPushButton("Separate",mywig);
    button_separate_hand ->setGeometry(650,40,100,30);

    QObject::connect(button_FFC, SIGNAL(clicked()), thr, SLOT(performFFC()));
    QObject::connect(button_rainbow, SIGNAL(clicked()),thr,SLOT(change_colormap_rainbow()));
    QObject::connect(button_grayscale, SIGNAL(clicked()),thr,SLOT(change_colormap_gray()));
    QObject::connect(button_iron, SIGNAL(clicked()),thr,SLOT(change_colormap_iron()));
    QObject::connect(button_normal, SIGNAL(clicked()),thr,SLOT(set_normal_mode()));
    QObject::connect(button_binarization, SIGNAL(clicked()),thr,SLOT(set_binary_mode()));
    QObject::connect(button_dilatation, SIGNAL(clicked()),thr,SLOT(switchon_dilatation()));
    QObject::connect(button_erosion, SIGNAL(clicked()),thr,SLOT(switchon_erosion()));
    QObject::connect(button_open, SIGNAL(clicked()),thr,SLOT(switchon_open()));
    QObject::connect(button_close, SIGNAL(clicked()),thr,SLOT(switchon_close()));
    QObject::connect(button_sobel, SIGNAL(clicked()),thr,SLOT(switchon_sobel()));
    QObject::connect(button_skeleton, SIGNAL(clicked()),thr,SLOT(switchon_skeleton()));
    QObject::connect(button_mediane, SIGNAL(clicked()),thr,SLOT(switchon_mediane()));
    QObject::connect(button_agc_en, SIGNAL(clicked()),thr,SLOT(enableAGC()));
    QObject::connect(button_agc_dis, SIGNAL(clicked()),thr,SLOT(disableAGC()));
    QObject::connect(button_snap_shot, SIGNAL(clicked()),thr,SLOT(make_snapshot()));
    QObject::connect(button_separate_hand, SIGNAL(clicked()),thr,SLOT(separate_hand()));

}

void create_sliders(QWidget *mywig,QThread *thr)
{
    QSlider *binarization_threshold = new QSlider(Qt::Horizontal,mywig);
    binarization_threshold -> setGeometry(375,10,175,30);
    binarization_threshold -> setMinimum(0);
    binarization_threshold -> setMaximum(255);
    binarization_threshold -> setTickPosition(QSlider::TicksBelow);
    binarization_threshold -> setTickInterval(10);
    binarization_threshold -> setValue(150);

    QSlider *Canny_threshold_lower = new QSlider(Qt::Horizontal,mywig);
    Canny_threshold_lower -> setGeometry(375,40,175,30);
    Canny_threshold_lower -> setMinimum(0);
    Canny_threshold_lower -> setMaximum(255);
    Canny_threshold_lower -> setTickPosition(QSlider::TicksBelow);
    Canny_threshold_lower -> setTickInterval(10);
    Canny_threshold_lower -> setValue(150);

    QObject::connect(binarization_threshold, SIGNAL(valueChanged(int)),thr,SLOT(change_slider_value_binary(int)));
    QObject::connect(Canny_threshold_lower, SIGNAL(valueChanged(int)),thr,SLOT(change_slider_value_canny(int)));
}

void create_checkboxes(QWidget *mywig,QThread *thr)
{
    QCheckBox *checkbox_reco = new QCheckBox("Recognize",mywig);
    checkbox_reco ->setGeometry(650,70,100,30);
    QCheckBox *checkbox_line = new QCheckBox("line",mywig);
    checkbox_line ->setGeometry(650,100,100,30);
    QCheckBox *checkbox_hist = new QCheckBox("histogram",mywig);
    checkbox_hist ->setGeometry(650,130,100,30);
    QCheckBox *checkbox_hull = new QCheckBox("hull",mywig);
    checkbox_hull ->setGeometry(650,160,100,30);
    QCheckBox *checkbox_count_contour = new QCheckBox("contour",mywig);
    checkbox_count_contour ->setGeometry(650,190,100,30);

    QObject::connect(checkbox_reco, SIGNAL(toggled(bool)),thr,SLOT(switchon_recognize()));
    QObject::connect(checkbox_line, SIGNAL(toggled(bool)),thr,SLOT(switchon_line()));
    QObject::connect(checkbox_hist, SIGNAL(toggled(bool)),thr,SLOT(switchon_histogram()));
    QObject::connect(checkbox_hull, SIGNAL(toggled(bool)),thr,SLOT(switchon_hull()));
    QObject::connect(checkbox_count_contour, SIGNAL(toggled(bool)),thr,SLOT(switchon_conting_countour()));
}

void create_screen(QWidget *mywig,QThread *thr)
{
}

void create_labels(QWidget *mywig,QThread *thr)
{
    QLabel *label_pixel= new QLabel("pixel",mywig);
    label_pixel-> setGeometry(375,60,200,60);

    QLabel *label_contours = new QLabel("contours",mywig);
    label_contours-> setGeometry(375,110,200,60);

    QLabel *label_contours_hull = new QLabel("hull",mywig);
    label_contours_hull-> setGeometry(375,150,200,60);

    QLabel *label_reco = new QLabel("RSP",mywig);
    label_reco -> setGeometry(375,200,200,60);

    QObject::connect(thr, SIGNAL(updateText(QString)),label_pixel,SLOT(setText(QString)));
    QObject::connect(thr, SIGNAL(updateTextContours(QString)),label_contours,SLOT(setText(QString)));
    QObject::connect(thr, SIGNAL(updateTextContoursHull(QString)),label_contours_hull,SLOT(setText(QString)));
    QObject::connect(thr, SIGNAL(updateTextReco(QString)),label_contours_hull,SLOT(setText(QString)));

}
