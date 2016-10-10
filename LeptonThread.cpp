#include "LeptonThread.h"

#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"
#include <math.h>

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27;

LeptonThread::LeptonThread() : QThread()
{
	mode = 0; //normal display mode
	colormap = colormap_rainbow;
	slider_value=0;
	ppmode=0;
	measure=false;
	mediane_on=false;
    width=80;
    height =60;
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
        //std::cout<<"Zmierzona "<<temp_measure(maxValue,lepton_temperature_fpa())<< "  Otoczenia :"<< lepton_temperature_fpa()<< "  "<<lepton_temperature_aux()<<std::endl;
		
        Mat opencvmat=Mat::zeros(Size(width,height),CV_8UC3);


		switch(mode)
		{
			case 0: //normal display
			{
			float diff = maxValue - minValue;
			float scale = 255/diff;


			for(int i=0;i<FRAME_SIZE_UINT16;i++)
				{
				if(i % PACKET_SIZE_UINT16 < 2) {
					continue;
				}
				value = (frameBuffer[i] - minValue) * scale;
				column = (i % PACKET_SIZE_UINT16 ) - 2;
				row = i / PACKET_SIZE_UINT16;
                Scalar color(colormap[3*value],colormap[3*value+1],colormap[3*value+2]);
                opencvmat.at<Vec3b>(Point(column,row))[0]=color[2];
                opencvmat.at<Vec3b>(Point(column,row))[1]=color[1];
                opencvmat.at<Vec3b>(Point(column,row))[2]=color[0];
				}
			}
			break;
			case 1: //binarization display
			{
			float diff = maxValue - minValue;
			float scale = 255/diff;
            Scalar color;
            for(int i=0;i<FRAME_SIZE_UINT16;i++)
				{
				if(i % PACKET_SIZE_UINT16 < 2) {
					continue;
				}
				value = (frameBuffer[i] - minValue) * scale;
                if(value>slider_value) color = Scalar(255,255,255);
                else color = Scalar(0,0,0);
				
				column = (i % PACKET_SIZE_UINT16 ) - 2;
				row = i / PACKET_SIZE_UINT16;
                opencvmat.at<Vec3b>(Point(column,row))[0]=color[0];
                opencvmat.at<Vec3b>(Point(column,row))[1]=color[1];
                opencvmat.at<Vec3b>(Point(column,row))[2]=color[2];
				}
			}
			break;
			
		}


		//POSTPROCESSING
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
             Mat grad_x;
             Mat grad_y;

              //  Sobel(opencvmat,grad_x,opencvmat.depth(),1,0,3,BORDER_DEFAULT);
              //  Sobel(opencvmat,grad_y,opencvmat.depth(),0,1,3,BORDER_DEFAULT);

                Scharr(opencvmat, grad_x, opencvmat.depth(), 1,0, 1, 0, BORDER_DEFAULT);
                Scharr(opencvmat, grad_y, opencvmat.depth(), 0,1, 1, 0, BORDER_DEFAULT);


                convertScaleAbs(grad_x,grad_x);
                convertScaleAbs(grad_y,grad_y);

                opencvmat=grad_x+grad_y;


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
		//MEASURMENT CROSS
        //if(measure==true)draw_cross_center(myImage);

        imshow("picture from opencv",opencvmat);
        cvtColor(opencvmat,opencvmat,CV_BGR2RGB);
        QImage  myImage2(opencvmat.data,opencvmat.cols,opencvmat.rows,opencvmat.step,QImage::Format_RGB888);
        emit updateImage(myImage2);

	}
	
	//finally, close SPI port just bcuz
	SpiClosePort(0);
}

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
void LeptonThread::switchon_measure()
{
    measure=true;
}
void LeptonThread::switchoff_measure()
{
    measure=false;
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

    imshow("",input);
    }
while(input.empty());
output=skel;

}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
