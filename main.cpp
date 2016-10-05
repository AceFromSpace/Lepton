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

#include "LeptonThread.h"
#include "MyLabel.h"
#include <iostream>
#include <opencv2/opencv.hpp>

         
using namespace cv;

int main( int argc, char **argv )
{
	//create the app
	QApplication a( argc, argv );
	
	QWidget *myWidget = new QWidget;
	myWidget->setGeometry(400, 300, 800, 350);

	//create an image placeholder for myLabel
	//fill the top left corner with red, just bcuz
	QImage myImage;
    cv::Mat temp= cv::Mat::zeros(240,320,CV_8UC3);

	myImage = QImage(320, 240, QImage::Format_RGB888);
	QRgb green = qRgb(0,255,0);
	for(int i=0;i<320;i++) {
		for(int j=0;j<240;j++) {
			myImage.setPixel(i, j, green);

		}
	}

	//create a label, and set it's image to the placeholder
	MyLabel myLabel(myWidget);
	myLabel.setGeometry(10, 10, 320, 240);
	myLabel.setPixmap(QPixmap::fromImage(myImage));

	QPushButton *button_rainbow = new QPushButton("Rainbow Scale", myWidget);
	button_rainbow->setGeometry(10, 290-35, 100, 30);
	QPushButton *button_grayscale = new QPushButton("Gray Scale", myWidget);
	button_grayscale->setGeometry(10, 290, 100, 30);
	QPushButton *button_iron = new QPushButton("Iron Scale", myWidget);
	button_iron ->setGeometry(10, 320, 100, 30);
	
	QPushButton *button1 = new QPushButton("Perform FFC", myWidget);
	button1->setGeometry(130, 290-35, 100, 30);
	QPushButton *button_binarization = new QPushButton("Binarization", myWidget);
	button_binarization ->setGeometry(130, 290, 100, 30);
	QPushButton *button_normal = new QPushButton("Normal Diplay", myWidget);
	button_normal ->setGeometry(130, 325, 100, 30);
	
	QPushButton *button_dilatation = new QPushButton("Dilatation", myWidget);
	button_dilatation ->setGeometry(260, 255, 100, 30);
	QPushButton *button_erosion = new QPushButton("Erosion", myWidget);
	button_erosion ->setGeometry(260, 290, 100, 30);
	QPushButton *button_open = new QPushButton("Open", myWidget);
	button_open ->setGeometry(260, 320, 100, 30);
	
	QPushButton *button_close = new QPushButton("Close", myWidget);
	button_close ->setGeometry(390, 255, 100, 30);
	QPushButton *button_sobel = new QPushButton("Sobel", myWidget);
	button_sobel ->setGeometry(390, 290, 100, 30);
	QPushButton *button_skeleton = new QPushButton("Skeleton", myWidget);
	button_skeleton ->setGeometry(390, 320, 100, 30);
	
	QPushButton *button_measureon = new QPushButton("Measure on", myWidget);
	button_measureon->setGeometry(520, 255, 100, 30);
	QPushButton *button_measureoff = new QPushButton("Measure off", myWidget);
	button_measureoff->setGeometry(520, 290, 100, 30);
	QPushButton *button_mediane = new QPushButton("Mediane",myWidget);
	button_mediane -> setGeometry(520,320,100,30);
	
	QPushButton *button_agc_en = new QPushButton("AGC ON",myWidget);
	button_agc_en-> setGeometry(650,255,100,30);
	QPushButton *button_agc_dis = new QPushButton("AGC OFF",myWidget);
	button_agc_dis-> setGeometry(650,290,100,30);
	
	QSlider *binarization_threshold = new QSlider(Qt::Horizontal,myWidget);
	binarization_threshold -> setGeometry(375,10,175,30);
	binarization_threshold -> setMinimum(0);
	binarization_threshold -> setMaximum(255);
	binarization_threshold -> setTickPosition(QSlider::TicksBelow);
	binarization_threshold -> setTickInterval(10);
	
	QLabel *temperature_label= new QLabel("text",myWidget);
	temperature_label-> setGeometry(375,70,30,30);
	
	//create a thread to gather SPI data
	//when the thread emits updateImage, the label should update its image accordingly
	LeptonThread *thread = new LeptonThread();
	QObject::connect(thread, SIGNAL(updateImage(QImage)), &myLabel, SLOT(setImage(QImage)));
	
	//connect ffc buttons to the thread's actions
	QObject::connect(button1, SIGNAL(clicked()), thread, SLOT(performFFC()));
	QObject::connect(button_rainbow, SIGNAL(clicked()),thread,SLOT(change_colormap_rainbow()));
	QObject::connect(button_grayscale, SIGNAL(clicked()),thread,SLOT(change_colormap_gray()));
	QObject::connect(button_iron, SIGNAL(clicked()),thread,SLOT(change_colormap_iron()));
	QObject::connect(button_normal, SIGNAL(clicked()),thread,SLOT(set_normal_mode()));
	QObject::connect(button_binarization, SIGNAL(clicked()),thread,SLOT(set_binary_mode()));
	QObject::connect(button_dilatation, SIGNAL(clicked()),thread,SLOT(switchon_dilatation()));
	QObject::connect(button_erosion, SIGNAL(clicked()),thread,SLOT(switchon_erosion()));
	QObject::connect(button_open, SIGNAL(clicked()),thread,SLOT(switchon_open()));
	QObject::connect(button_close, SIGNAL(clicked()),thread,SLOT(switchon_close()));
	QObject::connect(button_sobel, SIGNAL(clicked()),thread,SLOT(switchon_sobel()));
	QObject::connect(button_skeleton, SIGNAL(clicked()),thread,SLOT(switchon_skeleton()));
	QObject::connect(button_measureoff, SIGNAL(clicked()),thread,SLOT(switchoff_measure()));
	QObject::connect(button_measureon, SIGNAL(clicked()),thread,SLOT(switchon_measure()));
	QObject::connect(button_mediane, SIGNAL(clicked()),thread,SLOT(switchon_mediane()));
	QObject::connect(button_agc_en, SIGNAL(clicked()),thread,SLOT(enableAGC()));
	QObject::connect(button_agc_dis, SIGNAL(clicked()),thread,SLOT(disableAGC()));
	
	QObject::connect(binarization_threshold, SIGNAL(valueChanged(int)),thread,SLOT(change_slider_value(int)));
	//QObject::connect(temperature_label, SIGNAL(clicked()),thread,SLOT(setText(temp_measure())));	
		
	
	
	thread->start();
	
	myWidget->show();

	return a.exec();
}

