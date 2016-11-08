#include "LeptonThread.h"

#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"
#include <math.h>
#include <string>
#include <iostream>

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27;



LeptonThread::LeptonThread() : QThread()
{
    mode = false; //normal display mode
    mediane_on=false;
    histogram_on= false;
    counting_contours_on=false;
    mode_hull=false;
    draw_line =false;
    recognize =false;
    colormap = colormap_rainbow;
    ppmode=0;
    slider_value_binary=150;
    slider_value_canny=80;
    width=80;
    height =60;
    image_params=Mat::zeros(Size(width,height),CV_8UC1);
    image_params=255;
    cont=Mat::zeros(opencvmat.size(),CV_8UC1);
    mask=Mat::zeros(opencvmat.size(),CV_8UC1);
    image_hull=Mat::zeros(opencvmat.size(),CV_8UC1);
}

LeptonThread::~LeptonThread()
{
}

void LeptonThread::run()
{

    //open spi port
	SpiOpenPort(1);

	while(true) {

		//read data packets from lepton over SPI
		int resets = 0;
		for(int j=0;j<PACKETS_PER_FRAME;j++) 
			{
				//if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
				read(spi_cs1_fd, result+sizeof(uint8_t)*PACKET_SIZE*j, sizeof(uint8_t)*PACKET_SIZE);
				int packetNumber = result[j*PACKET_SIZE+1];
				if(packetNumber != j) {
					j = -1;
					resets += 1;
					usleep(1000);
					//Note: we've selected 750 resets as an arbitrary limit, since there should never be 750 "null" packets between two valid transmissions at the current poll rate
					//By polling faster, developers may easily exceed this count, and the down period between frames may then be flagged as a loss of sync
					if(resets == 750) {
						SpiClosePort(0);
						usleep(750000);
						SpiOpenPort(0);
					}
				}
			}
		if(resets >= 30) {
			qDebug() << "done reading, resets: " << resets;
		}

		frameBuffer = (uint16_t *)result;
		int row, column;
		uint16_t value;
		uint16_t minValue = 65535;
		uint16_t maxValue = 0;

		
		for(int i=0;i<FRAME_SIZE_UINT16;i++)
		 {
			//skip the first 2 uint16_t's of every packet, they're 4 header bytes
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			
			//flip the MSB and LSB at the last second
			int temp = result[i*2];
			result[i*2] = result[i*2+1];
			result[i*2+1] = temp;
			
			value = frameBuffer[i];
			if(value > maxValue) {
				maxValue = value;
			}
			if(value < minValue) {
				minValue = value;
			}
			column = i % PACKET_SIZE_UINT16 - 2;
			row = i / PACKET_SIZE_UINT16 ;
		}

		
        opencvmat=Mat::zeros(Size(width,height),CV_8UC3);
        opencvmat_base=Mat::zeros(Size(width,height),CV_8UC1);

			float diff = maxValue - minValue;
			float scale = 255/diff;


			for(int i=0;i<FRAME_SIZE_UINT16;i++)
				{
                if(i % PACKET_SIZE_UINT16 < 2)continue;

                    value = (frameBuffer[i] - minValue) * scale;
                    column = (i % PACKET_SIZE_UINT16 ) - 2;
                    row = i / PACKET_SIZE_UINT16;
                    Scalar color(colormap[3*value],colormap[3*value+1],colormap[3*value+2]);
                    opencvmat.at<Vec3b>(Point(column,row))[0]=color[2];
                    opencvmat.at<Vec3b>(Point(column,row))[1]=color[1];
                    opencvmat.at<Vec3b>(Point(column,row))[2]=color[0];

                    opencvmat_base.at<uchar>(Point(column,row))=value;

                }

        postprocessing(opencvmat);
        Mat output;
        opencvmat.copyTo(output);
        cvtColor(output,output,CV_BGR2RGB);
        QImage  myImage(output.data,output.cols,output.rows,output.step,QImage::Format_RGB888);
        emit updateImage(myImage);

	}

    SpiClosePort(1);
}

//voids

void LeptonThread::make_snapshot()
{
    time_t t = time(0);
    struct tm * now =localtime(& t);
    std::ostringstream ss;
    ss<<now->tm_hour*3600 +now->tm_min*60+now->tm_sec;
    namedWindow(ss.str(),CV_WINDOW_NORMAL);
    setWindowProperty(ss.str(),CV_WND_PROP_FULLSCREEN,CV_WINDOW_KEEPRATIO);

    imshow(ss.str(),opencvmat);

    imwrite(ss.str()+"t"+".jpg",opencvmat);
    imwrite(ss.str()+"m" +".jpg",mask);
    imwrite(ss.str()+".jpg",cont);
    imwrite(ss.str()+"hu"+".jpg",image_hull);
    imwrite(ss.str()+"d"+".jpg",image_params);
    imwrite(ss.str()+"h"+".jpg",image_histogram);

}

void LeptonThread::mr_skeleton(Mat input, Mat &output)
{

Mat skel=Mat::zeros(Size(width,height),CV_8UC3);
Mat eroded=Mat::zeros(Size(width,height),CV_8UC3);
Mat temp=Mat::zeros(Size(width,height),CV_8UC3);
Mat empty=Mat::zeros(input.size(),input.type());
input.copyTo(temp);

Mat element = getStructuringElement(MORPH_CROSS,Size(3,3));

do
    {
    erode(input,eroded,element);
    dilate(eroded,temp,element);
    temp= ~temp;
    temp= temp & input;
    skel = skel | temp;
    erode(input,input,element);
    }
while(input.empty());
output=skel;

}

void LeptonThread::finding_edges(Mat input, Mat &output)
{
     Mat grad_x;
     Mat grad_y;
     Scharr(input, grad_x, input.depth(), 1,0, 1, 0, BORDER_DEFAULT);
     Scharr(input, grad_y, input.depth(), 0,1, 1, 0, BORDER_DEFAULT);
     convertScaleAbs(grad_x,grad_x);
     convertScaleAbs(grad_y,grad_y);
     output=grad_x+grad_y;
}

void LeptonThread::separate_hand()
{
    Mat temp=Mat::ones(opencvmat_base.size(),CV_8UC1);
    cont=Mat::zeros(opencvmat.size(),CV_8UC1);
    opencvmat_base.copyTo(temp);
    threshold(temp,temp,slider_value_binary,255,THRESH_BINARY);

    std::vector<std::vector<cv::Point> > contours;
    findContours (temp,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_SIMPLE , CV_CHAIN_APPROX_TC89_L1, CV_CHAIN_APPROX_TC89_KCOS,CV_CHAIN_APPROX_NONE
    mask = Mat::zeros(opencvmat.size(),CV_8UC3);

    int index_biggest=0;
    for(int i =0;i<contours.size();i++)
    {
        if(contours[i].size()>contours[index_biggest].size())index_biggest=i;
    }

    drawContours(mask,contours,index_biggest,Scalar(255,255,255),CV_FILLED);
    cont = opencvmat& mask;

    double hull_coverage;
    double contour_ratio;
    if(mode_hull) hull_coverage = draw_convex_hull(cont,contours,index_biggest);
    if(histogram_on) histogram_alternative(cont);
    if(counting_contours_on) contour_ratio=counting_contour(cont,mask);
    if(recognize)recognize_gesture(hull_coverage,contour_ratio);

}

double LeptonThread::draw_convex_hull(Mat image, std::vector<std::vector<Point> > conto, int biggest)
{
    Mat temp;
    image.copyTo(temp);
    std::vector<std::vector<Point> > hull(conto.size());

    convexHull(Mat(conto[biggest]),hull[biggest],false);
    Scalar pink(255,0,255);
    drawContours(temp,hull,biggest,pink,1,8,std::vector<Vec4i>(),0,Point());

    Mat temp_gray;
    temp.copyTo(temp_gray);
    Size new_size(320,240);
    resize(temp,temp,new_size);
    imshow("hull",temp);
    image_hull =temp;

    Mat field(temp_gray.size(),CV_8UC1);
    Scalar white(255,255,255);
    drawContours(field,hull,biggest,white,-1,8,std::vector<Vec4i>(),0,Point());
    int field_under_hull =countNonZero(field);
    cvtColor(temp_gray,temp_gray,CV_BGR2GRAY);
    int field_hand =countNonZero(temp_gray);
    double ratio = (double)field_hand/field_under_hull;

    Vec4f vec_line;
    if(draw_line) fitLine(conto[biggest] ,vec_line,CV_DIST_L2,0,0.01,0.01); //

    int m =30;
    line(temp_gray,Point(vec_line[2]-vec_line[0]*m,vec_line[3]-vec_line[1]*m),Point(vec_line[2] +vec_line[0]*m, vec_line[3]+vec_line[1]*m),255);


    resize(temp_gray,temp_gray,new_size);
    imshow("fdsf",temp_gray);

    std::ostringstream ss;
    ss<<hull[biggest].size() ;
    putText(image_params,"Hull:"+ ss.str()+"/n" ,Point(5,30),1,2,0);
    imshow("bal",image_params);
    emit updateTextContours("Hull:"+ QString::number(hull[biggest].size())+"/n"
                            + "Ratio h/f"+ QString::number(ratio,'f',2));
    return ratio;
}

void LeptonThread::save_hist()
{
    std::fstream file;
    file.open("hist",std::ios::app);
    file<<"nazwa(zaraz ustale)"<<std::endl;

    for(int i=0;i<256;i++) file<<hist[0][i]<<" ";
    file<<std::endl;
    for(int i=0;i<256;i++) file<<hist[1][i]<<" ";
    file<<std::endl;
    for(int i=0;i<256;i++) file<<hist[2][i]<<" ";
    file<<std::endl;

    file.close();

}

void LeptonThread::histogram_alternative(Mat image)
{
std::vector<Mat> bgr;
split(image,bgr);
int hist_size = 256;
float range[] = {0,256};
const float* histRange = {range};

bool uniform =true; bool accumulate = false;

Mat b_hist, g_hist, r_hist;

calcHist(&bgr[0],1,0, Mat(),b_hist,1,&hist_size,&histRange,uniform,accumulate);
calcHist(&bgr[1],1,0, Mat(),g_hist,1,&hist_size,&histRange,uniform,accumulate);
calcHist(&bgr[2],1,0, Mat(),r_hist,1,&hist_size,&histRange,uniform,accumulate);

int hist_w = 512; int hist_h = 300*3;
int bin_w = cvRound ((double)hist_w/hist_size);

Mat histImage(hist_h,hist_w,CV_8UC3, Scalar(255,255,255));
int bottom = hist_h/10*9;

for(int i =1;i <hist_size;i++)
{
    rectangle(histImage,Point(bin_w*i, bottom/3),Point(bin_w*(i+1) ,bottom/3-cvRound(b_hist.at<float>(i))), Scalar(255,0,0),-1);
    rectangle(histImage,Point(bin_w*i, bottom*2/3),Point(bin_w*(i+1) ,bottom*2/3-cvRound(g_hist.at<float>(i))), Scalar(0,255,0),-1);
    rectangle(histImage,Point(bin_w*i, bottom),Point(bin_w*(i+1) ,bottom-cvRound(r_hist.at<float>(i))), Scalar(0,0,255),-1);
}
imshow("CalcHist",histImage);
histImage.copyTo(image_histogram);
}

double LeptonThread::counting_contour(Mat image, Mat mask)
{
    Mat temp;
    image.copyTo(temp);
    Mat temp_mask;
    mask.copyTo(temp_mask);
    cvtColor(temp_mask,temp_mask,CV_BGR2GRAY);
    cvtColor(temp,temp,CV_BGR2GRAY);
    Canny(temp,temp,slider_value_canny,slider_value_canny*3/2,3);
    imshow("hej",temp);

    int number_of_pixels_countour = countNonZero(temp);
    int number_of_pixels_hand = countNonZero(temp_mask);
    double ratio = (double)number_of_pixels_hand/number_of_pixels_countour;
    emit updateText("countur:"+ QString::number(number_of_pixels_countour)+ "\n"
                    +"Field:"+QString::number(number_of_pixels_hand)+"\n"
                    +"Radio:"+QString::number(ratio,'f',2));

    Canny(temp,temp,slider_value_canny,slider_value_canny*3/2,3);
    std::vector<std::vector<cv::Point> > contours;
    findContours (temp,contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_SIMPLE , CV_CHAIN_APPROX_TC89_L1, CV_CHAIN_APPROX_TC89_KCOS,CV_CHAIN_APPROX_NONE
    int all_contours=0;
    for(int i = 0; i<contours.size();i++ )
    {
        all_contours+=contours[i].size();
    }

    emit updateTextContoursHull("All cont:" + QString::number(all_contours));
    return ratio;
}

void LeptonThread::postprocessing(Mat image)
{
    if(mode)
    {
        Mat temp;
        opencvmat_base.copyTo(temp);
        threshold(temp,temp,slider_value_binary,255,THRESH_BINARY);
        cvtColor(temp,temp,CV_GRAY2BGR);
        temp.copyTo(image);
    }

    switch(ppmode)
    {
        case 0:
        {

        }
        break;
        case 1:
        {
            dilate(image,image,Mat());
        }
        break;
        case 2:
        {
            erode(image,image,Mat());
        }
        break;
        case 3:
        {
            erode(image,image,Mat());
            dilate(image,image,Mat());

        }
        break;
        case 4:
        {
            dilate(image,image,Mat());
            erode(image,image,Mat());
        }
        break;
        case 5:
        {
            finding_edges(image,image);
        }
        break;
        case 6:
        {
        mr_skeleton(image,image);
        }
        break;
        case 7:
        {
        medianBlur(image,image,3);
        }
        break;
        default :
        {
        std::cout<<"Error"<<std::endl;
        }
        break;

    }
}

void LeptonThread::recognize_gesture(double hull,double conts)
{
    if(hull<0.90)
    {
        emit updateTextReco("Scizors");
    }
    else
    {
        emit updateTextReco("Stone or paper");
    }

}
//////////////////////////
bool LeptonThread::return_mode()
{
    if(mode==1)return true;
    return false;
}
void LeptonThread::change_colormap_rainbow()
{
    colormap=colormap_rainbow;
}
void LeptonThread::change_colormap_gray()
{
    colormap=colormap_grayscale;
}
void LeptonThread::change_colormap_iron()
{
     colormap=colormap_ironblack;
}
void LeptonThread::performFFC()
{
    lepton_perform_ffc();
}
void LeptonThread::enableAGC()
{
    lepton_enable_agc();
}
void LeptonThread::disableAGC()
{
    lepton_disable_agc();
}
void LeptonThread::set_binary_mode()
{
    mode=1;
}
void LeptonThread::set_normal_mode()
{
    mode=0;
    ppmode=0;
    mediane_on=false;
}
void LeptonThread::change_slider_value_binary(int value)
{
    slider_value_binary=value;
}
void LeptonThread::change_slider_value_canny(int value)
{
    slider_value_canny=value;
}
void LeptonThread::switchon_dilatation()
{
    ppmode=1;
    }
void LeptonThread::switchon_erosion()
{
    ppmode=2;
}
void LeptonThread::switchon_open()
{
    ppmode=3;
}
void LeptonThread::switchon_close()
{
    ppmode=4;
}
void LeptonThread::switchon_sobel()
{
    ppmode=5;
}
void LeptonThread::switchon_skeleton()
{
    ppmode=6;
}
void LeptonThread::switchon_mediane()
{
    ppmode=7;
}
void LeptonThread::switchon_conting_countour()
{
    counting_contours_on = !counting_contours_on;
}
void LeptonThread::switchon_histogram()
{
    histogram_on=!histogram_on;
}
void LeptonThread::switchon_hull()
{
    mode_hull=!mode_hull;
}
void LeptonThread::switchon_line()
{
    draw_line=!draw_line;
}
void LeptonThread::switchon_recognize()
{
    recognize=!recognize;
}
	
	
	
	
	
	
	
	
