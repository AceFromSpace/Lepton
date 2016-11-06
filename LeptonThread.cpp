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
	mode = 0; //normal display mode
	colormap = colormap_rainbow;
    slider_value=120;
	ppmode=0;
    learn=false;
	mediane_on=false;
    histogram_on= false;
    mode_hull=false;
    width=80;
    height =60;
    clean_hist();
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
        postprocessing();

        Mat output = Mat::zeros(opencvmat.size(),CV_8UC3);

        if(mode)
        {
            opencvmat_base.copyTo(output);
            threshold(output,output,slider_value,255,THRESH_BINARY);
            cvtColor(output,output,CV_GRAY2BGR);
        }
        else
        {
             opencvmat.copyTo(output);
              cvtColor(output,output,CV_BGR2RGB);
        }

        QImage  myImage(output.data,output.cols,output.rows,output.step,QImage::Format_RGB888);
        emit updateImage(myImage);
	}
	
	//finally, close SPI port just bcuz
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
/*
    Mat output= Mat::zeros(Size(cont.cols+image_histogram.cols+1,image_histogram.rows+1),CV_8UC3);
    Mat roi2(output,Rect(0,0,image_histogram.cols,image_histogram.rows));
    Mat roi1(output,Rect(0,0,cont.cols,cont.rows));
    cont.copyTo(roi1);
    image_histogram.copyTo(roi2);
*/
    imshow(ss.str(),cont);
    imwrite(ss.str()+".jpg",cont);
   // imshow(ss.str(),image_histogram);
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

     //  Sobel(opencvmat,grad_x,opencvmat.depth(),1,0,3,BORDER_DEFAULT);
     //  Sobel(opencvmat,grad_y,opencvmat.depth(),0,1,3,BORDER_DEFAULT);

       Scharr(input, grad_x, input.depth(), 1,0, 1, 0, BORDER_DEFAULT);
       Scharr(input, grad_y, input.depth(), 0,1, 1, 0, BORDER_DEFAULT);

       convertScaleAbs(grad_x,grad_x);
       convertScaleAbs(grad_y,grad_y);

       output=grad_x+grad_y;
}

void LeptonThread::find_countour()
{
   Mat temp=Mat::zeros(opencvmat.size(),CV_8UC1);

   opencvmat.copyTo(temp);
   cvtColor(temp,temp,CV_BGR2GRAY);
   std::vector<std::vector<cv::Point> > contours;
   findContours (temp,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
   Mat cont =Mat::zeros(opencvmat.size(),CV_8UC1);

   int index_biggest=0;

   for(int i =0;i<contours.size();i++)
   {
       if(contours[i].size()>contours[index_biggest].size())index_biggest=i;
   }


   drawContours(cont,contours,index_biggest,Scalar(255,255,255),CV_FILLED);
   imshow("contours",cont);

}

void LeptonThread::show_hist(Mat image)
{
get_hist(image);
int margin=10;
int line_width=2;
image_histogram=Mat::ones(Size(line_width*256+2*margin,900),CV_8UC3);
image_histogram=Scalar(255,255,255);
int bottom=image_histogram.rows*19/20;

if(image.channels()==1)
{
    for(int i=0;i<256;i++)
    {

        rectangle(image_histogram,Point(i*line_width+margin,bottom),Point((i+1)*line_width+margin,bottom-hist[0][i]),44,-1);
    }
}
else
{
    for(int i=0;i<256;i++)
    {
        int value= hist[0][i]/4;
        if(value>270)value=270;
        rectangle(image_histogram,Point(i*line_width+margin,bottom/3),Point((i+1)*line_width+margin,bottom/3-value),Scalar(255,0,0),-1);
    }
    for(int i=0;i<256;i++)
    {
        int value= hist[1][i]/4;
        if(value>270)value=270;
        rectangle(image_histogram,Point(i*line_width+margin,bottom/3*2),Point((i+1)*line_width+margin,bottom/3*2-value),Scalar(0,255,0),-1);
    }
    for(int i=0;i<256;i++)
    {
        int value= hist[2][i]/4;
        if(value>270)value=270;
        rectangle(image_histogram,Point(i*line_width+margin,bottom),Point((i+1)*line_width+margin,bottom-value),Scalar(0,0,255),-1);
    }
}


imshow("histogram",image_histogram);
clean_hist();

}

void LeptonThread::clean_hist()
{
    save_hist();
    for(int i =0;i<256;i++)
    {
        hist[0][i]=0;
        hist[1][i]=0;
        hist[2][i]=0;
    }
}

void LeptonThread::get_hist(Mat image)
{
    if(image.channels()==1)
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            hist[0][image.at<Vec3b>(Point(i,j))[0]]++;
        }
    }
    else
    {
        for(int i=0;i<image.rows;i++)
        {
            for(int j=0;j<image.cols;j++)
            {
                hist[0][image.at<Vec3b>(Point(i,j))[0]]++;
                hist[1][image.at<Vec3b>(Point(i,j))[1]]++;
                hist[2][image.at<Vec3b>(Point(i,j))[2]]++;
            }
        }

    }
}

void LeptonThread::separate_hand()
{
    Mat temp=Mat::ones(opencvmat_base.size(),CV_8UC1);
    opencvmat_base.copyTo(temp);
    threshold(temp,temp,slider_value,255,THRESH_BINARY);

    std::vector<std::vector<cv::Point> > contours;
    findContours (temp,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    cont =Mat::zeros(opencvmat.size(),CV_8UC3);

    int index_biggest=0;

    for(int i =0;i<contours.size();i++)
    {
        if(contours[i].size()>contours[index_biggest].size())index_biggest=i;
    }
    drawContours(cont,contours,index_biggest,Scalar(255,255,255),CV_FILLED);
    cont = opencvmat& cont;



/*
    std::ostringstream ss;
    ss<<cont.type();
    putText(cont,ss.str(),Point(100,100),1,4,Scalar(255,255,255));
*/
    if(mode_hull)draw_convex_hull(cont,contours);
    if(histogram_on)show_hist(cont);
    Size new_size(240,180);
    resize(cont,cont,new_size);
    imshow("contours",cont);
}

void LeptonThread::draw_convex_hull(Mat image,std::vector<std::vector<Point> > conto)
{
    std::vector<std::vector<Point> > hull(conto.size());
    for(int i = 0;i <conto.size();i++)
    {
        convexHull(Mat(conto[i]),hull[i],false);
    }
    Scalar pink(255,0,255);
    for(int i=0;i<conto.size();i++)
    {
        drawContours(cont,hull,i,pink,1,8,std::vector<Vec4i>(),0,Point());
    }
}


void LeptonThread::postprocessing()
{
    switch(ppmode)
    {
        case 0:
        {

        }
        break;
        case 1:
        {
            dilate(opencvmat,opencvmat,Mat());
        }
        break;
        case 2:
        {
            erode(opencvmat,opencvmat,Mat());
        }
        break;
        case 3:
        {
            erode(opencvmat,opencvmat,Mat());
            dilate(opencvmat,opencvmat,Mat());

        }
        break;
        case 4:
        {
            dilate(opencvmat,opencvmat,Mat());
            erode(opencvmat,opencvmat,Mat());
        }
        break;
        case 5:
        {
            finding_edges(opencvmat,opencvmat);

        }
        break;
        case 6:
        {
        mr_skeleton(opencvmat,opencvmat);
        }
        break;
        case 7:
        {
        medianBlur(opencvmat,opencvmat,3);
        }
        break;
        default :
        {
        std::cout<<"Error"<<std::endl;
        }
        break;

    }
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
void LeptonThread::change_slider_value(int value)
{
    slider_value=value;
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
void LeptonThread::switchon_learn()
{
    learn=true;
}
void LeptonThread::switchoff_learn()
{
    learn=false;
}
void LeptonThread::switchon_histogram()
{
    histogram_on=!histogram_on;
}
void LeptonThread::switchon_hull()
{
    mode_hull=!mode_hull;
}
	
	
	
	
	
	
	
	
	
	
	
