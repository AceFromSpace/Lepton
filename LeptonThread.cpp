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
}

LeptonThread::~LeptonThread()
{
	 
}

void LeptonThread::run()
{
	//create the initial image
	myImage = QImage(80, 60, QImage::Format_RGB888);

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
		std::cout<<"Zmierzona "<<temp_measure(maxValue,lepton_temperature_fpa())<< "  Otoczenia :"<< lepton_temperature_fpa()<< "  "<<lepton_temperature_aux()<<std::endl;
		
	

		switch(mode)
		{
			case 0: //normal display
			{
			float diff = maxValue - minValue;
			float scale = 255/diff;
			QRgb color;
			for(int i=0;i<FRAME_SIZE_UINT16;i++)
				{
				if(i % PACKET_SIZE_UINT16 < 2) {
					continue;
				}
				value = (frameBuffer[i] - minValue) * scale;
				color = qRgb(colormap[3*value], colormap[3*value+1], colormap[3*value+2]);
				column = (i % PACKET_SIZE_UINT16 ) - 2;
				row = i / PACKET_SIZE_UINT16;
				myImage.setPixel(column, row, color);
				}
			}
			break;
			case 1: //binarization display
			{
			float diff = maxValue - minValue;
			float scale = 255/diff;
			QRgb color;
			for(int i=0;i<FRAME_SIZE_UINT16;i++)
				{
				if(i % PACKET_SIZE_UINT16 < 2) {
					continue;
				}
				value = (frameBuffer[i] - minValue) * scale;
				if(value>slider_value)color = qRgb(255,255,255);
				else color = qRgb(0,0,0);
				
				column = (i % PACKET_SIZE_UINT16 ) - 2;
				row = i / PACKET_SIZE_UINT16;
				myImage.setPixel(column, row, color);
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
				dilatation(myImage,myImage,5,1);
			}
			break;
			case 2:
			{
				erosion(myImage,myImage,5,1);
			}
			break;
			case 3:
			{
				erosion(myImage,myImage,5,1);
				dilatation(myImage,myImage,5,1);
				
			}
			break;
			case 4:
			{
				dilatation(myImage,myImage,5,1);
				erosion(myImage,myImage,5,1);
			}
			break;
			case 5:
			{
				sobel(myImage,myImage,3);
			}
			break;
			case 6:
			{
			mr_skeleton(myImage,myImage);	
			}
			break;
			case 7:
			{
			mediane(myImage,myImage,5);	
			}
			break;
			default :
			{
				
			}
			break;
		
		}
		//MEASURMENT CROSS
		if(measure==true)draw_cross_center(myImage);
		
		//lets emit the signal for update
		emit updateImage(myImage);

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

void LeptonThread::performFFC() {
	//perform FFC
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

/*
void duplicate_edges(QImage input ,QImage &output,int frame)
{
	QImage temp =new QImage(input.width()+2*frame,input.height()+2*frame,QImage::Format_RGB888);
	for(int i=0;i<temp.width();i++)
	{
			for(int j=0;i<temp.height();i++)
			{
			if((i<frame)||(j<frame))temp.setPixel(i,j,input.pixel(i,j);
			if else((i>input.width()+frame)||(j>input.height()+frame))temp.setPixel(i,j,input.pixel(i,j);
			else
		
			}
			
				
			}
		
		
	}
	
}
*/
void LeptonThread::dilatation(QImage input, QImage &output,int kernel_size,int iterations)
{
QImage temp =input;
QImage temp2 =input;
for(int t=0;t<iterations;t++)
	{
	for(int i=0;i<input.width();i++) 
		{
			for(int j=0;j<input.height();j++) 
			{	
				QRgb new_value=temp.pixel(i,j);
				for(int kh=-1*kernel_size/2;kh<=kernel_size/2;kh++)
				{
					for(int kv=-1*kernel_size/2;kv<=kernel_size/2;kv++)
					{
						if(i+kh>=0 && i+kh<input.width()&&j+kv>=0&&j+kv<input.height())
						{
							if(temp.pixel(i+kh,j+kv)>new_value)new_value=temp.pixel(i+kh,j+kv);	
						}
					}
				}
				temp2.setPixel(i,j,new_value);
			}
		}	
		temp=temp2;
	}
	output=temp;
	
}
void LeptonThread::cross_dilatation(QImage input, QImage &output,int kernel_size,int iterations)
{
QImage temp =input;
QImage temp2 =input;
for(int t=0;t<iterations;t++)
	{
	for(int i=0;i<input.width();i++) 
		{
			for(int j=0;j<input.height();j++) 
			{	
				QRgb new_value=temp.pixel(i,j);
				for(int kh=-1*kernel_size/2;kh<=kernel_size/2;kh++)
				{
					if(i+kh>=0 && i+kh<input.width())
						{
							if(temp.pixel(i+kh,j)>new_value)new_value=temp.pixel(i+kh,j);	
						}
				}
				for(int kv=-1*kernel_size/2;kv<=kernel_size/2;kv++)
					{
						if(j+kv>=0&&j+kv<input.height())
						{
							if(temp.pixel(i,j+kv)>new_value)new_value=temp.pixel(i,j+kv);	
						}
					}
				temp2.setPixel(i,j,new_value);
			}
		}	
		temp=temp2;
	}
	output=temp;
	
}
void LeptonThread::cross_erosion(QImage input, QImage &output,int kernel_size,int iterations)
{
QImage temp =input;
QImage temp2 =input;
for(int t=0;t<iterations;t++)
	{
	for(int i=0;i<input.width();i++) 
		{
			for(int j=0;j<input.height();j++) 
			{	
				QRgb new_value=temp.pixel(i,j);
				for(int kh=-1*kernel_size/2;kh<=kernel_size/2;kh++)
				{
					if(i+kh>=0 && i+kh<input.width())
						{
							if(temp.pixel(i+kh,j)<new_value)new_value=temp.pixel(i+kh,j);	
						}
				}
				for(int kv=-1*kernel_size/2;kv<=kernel_size/2;kv++)
					{
						if(j+kv>=0&&j+kv<input.height())
						{
							if(temp.pixel(i,j+kv)<new_value)new_value=temp.pixel(i,j+kv);	
						}
					}
				temp2.setPixel(i,j,new_value);
			}
		}	
		temp=temp2;
	}
	output=temp;
}

void LeptonThread::erosion(QImage input, QImage &output,int kernel_size,int iterations)
{
QImage temp =input;
QImage temp2 =input;
for(int t=0;t<iterations;t++)
	{
	for(int i=0;i<input.width();i++) 
		{
			for(int j=0;j<input.height();j++) 
			{	
				QRgb new_value=temp.pixel(i,j);
				for(int kh=-1*kernel_size/2;kh<=kernel_size/2;kh++)
				{
					for(int kv=-1*kernel_size/2;kv<=kernel_size/2;kv++)
					{
						if(i+kh>=0 && i+kh<input.width()&&j+kv>=0&&j+kv<input.height())
						{
							if(temp.pixel(i+kh,j+kv)<new_value)new_value=temp.pixel(i+kh,j+kv);	
						}
					}
				}
				temp2.setPixel(i,j,new_value);
			}
		}	
		temp=temp2;
	}
	output=temp;
}

void LeptonThread::sobel(QImage input, QImage &output,int kernel_size)
{
QImage temp =input;


for(int i=kernel_size/2;i<(input.width()-kernel_size/2);i++) 
	{
		for(int j=kernel_size/2;j<(input.height()-kernel_size/2);j++) 
		{	
			int new_value_v=0;
			int new_value_h=0;
			if(kernel_size==3)
			{
				new_value_v= -1*input.pixel(i-1,j-1)-2*input.pixel(i,j-1)-1*input.pixel(i+1,j-1)+1*input.pixel(i+1,j+1)+2*input.pixel(i,j+1)+1*input.pixel(i-1,j+1);
				if (new_value_v<0)new_value_v= -1*new_value_v;
				new_value_h= -1*input.pixel(i-1,j-1)-2*input.pixel(i-1,j)-1*input.pixel(i-1,j+1)+1*input.pixel(i+1,j+1)+2*input.pixel(i+1,j)+1*input.pixel(i+1,j-1);
				if (new_value_h<0)new_value_h= -1*new_value_h;
			}
			else if (kernel_size==5)
			{
				new_value_v= -1*input.pixel(i-2,j-2)-4*input.pixel(i-1,j-2)- 6*input.pixel(i,j-2)-4*input.pixel(i+1,j-2)-1*input.pixel(i+2,j-2)
							 -2*input.pixel(i-2,j-1)-8*input.pixel(i-1,j-1)-12*input.pixel(i,j-1)-8*input.pixel(i+1,j-1)-2*input.pixel(i+2,j-1)
							 +2*input.pixel(i-2,j+1)+8*input.pixel(i-1,j+1)+12*input.pixel(i,j+1)+8*input.pixel(i+1,j+1)+2*input.pixel(i+2,j+1)
							 +1*input.pixel(i-2,j+2)+4*input.pixel(i-1,j+2)+ 6*input.pixel(i,j+2)+4*input.pixel(i+1,j+2)+1*input.pixel(i+2,j+2);
				if (new_value_v<0)new_value_v= -1*new_value_v;
				new_value_h= -1*input.pixel(i-2,j-2)-4*input.pixel(i-2,j-1)- 6*input.pixel(i-2,j)-4*input.pixel(i-2,j+1)-1*input.pixel(i-2,j+2)
							 -2*input.pixel(i-1,j-2)-8*input.pixel(i-1,j-1)-12*input.pixel(i-1,j)-8*input.pixel(i-1,j+1)-2*input.pixel(i-1,j+2)
							 +2*input.pixel(i-1,j-2)+8*input.pixel(i+1,j-1)+12*input.pixel(i+1,j)+8*input.pixel(i+1,j+1)+2*input.pixel(i+1,j+2)
							 +1*input.pixel(i+2,j-2)+4*input.pixel(i+2,j-1)+ 6*input.pixel(i+2,j)+4*input.pixel(i+2,j+1)+1*input.pixel(i+2,j+2);
				if (new_value_h<0)new_value_h= -1*new_value_h;
				
			
			}
			temp.setPixel(i,j,new_value_v+new_value_h);
		}
	}	
output=temp;
}

void LeptonThread::mr_skeleton(QImage input, QImage &output)
{
	
QImage temp = input;
QImage skel = QImage(input.width(),input.height(),QImage::Format_RGB888);
QImage empty = QImage(input.width(),input.height(),QImage::Format_RGB888);
QImage eroded = QImage(input.width(),input.height(),QImage::Format_RGB888);

get_black(skel);
get_black(empty);
get_black(eroded);

do
	{
	cross_erosion(input,eroded,3,1);
	cross_dilatation(eroded,temp,3,1);
	bit_not(temp,temp);
	bit_and(input,temp,temp);
	bit_or(skel,temp,skel);
	cross_erosion(input,input,3,1);


	}
while(!(empty==input));
output=skel;	

}

void LeptonThread::bit_not(QImage input,QImage &output)
{
for(int i=0;i<input.width();i++) 
		{
			for(int j=0;j<input.height();j++) 
			{	
				
				int temp=~input.pixel(i,j);
				output.setPixel(i,j,temp);
				
			}
		}	
}

void LeptonThread::bit_and(QImage input1,QImage input2,QImage &output)
{
for(int i=0;i<input1.width();i++) 
		{
			for(int j=0;j<input1.height();j++) 
			{
				int temp=input1.pixel(i,j)&input2.pixel(i,j);
				output.setPixel(i,j,temp);	
			}
		}	
}

void LeptonThread::bit_or(QImage input1,QImage input2,QImage &output)
{
for(int i=0;i<input1.width();i++) 
		{
			for(int j=0;j<input1.height();j++) 
			{	
				
				int temp=input1.pixel(i,j)|input2.pixel(i,j);
				output.setPixel(i,j,temp);
			}
		}	
}

void LeptonThread::get_black(QImage &inputoutput)
{
for(int i=0;i<inputoutput.width();i++) 
		{
			for(int j=0;j<inputoutput.height();j++) 
			{	
				inputoutput.setPixel(i,j,0);
			}
		}	
}

void LeptonThread::draw_cross_center(QImage &inputoutput)
{
	int cross_radius=3;
	for(int i=inputoutput.width()/2-cross_radius;i<=inputoutput.width()/2+cross_radius;i++) 
	{
				inputoutput.setPixel(i,inputoutput.height()/2,0);
	}	
	for(int j=inputoutput.height()/2-cross_radius;j<=inputoutput.height()/2+cross_radius;j++) 
	{	
				inputoutput.setPixel(inputoutput.width()/2,j,0);
	}
	
}

double LeptonThread::temp_measure(int val,int amb_temp)
{
	/*//double  temp =0.0465*val-349.44;
	double delta= 346.48 - 4*0.1278*(7372.6-val);
	double sq_delta= sqrt(delta); 
	double temp =(-1* 18.614 -sq_delta)/0.256;*/
	double temp =(0.0217*val)+amb_temp/100-177.77;
	return temp;

}
void LeptonThread::mediane(QImage input, QImage &output,int kernel_size)
{

QImage temp =input;
for(int i=0;i<input.width();i++) 
		{
			for(int j=0;j<input.height();j++) 
			{	
				int values_under_kernel[kernel_size*kernel_size];
				int iter=0;
				
				for(int kh=-1*kernel_size/2;kh<=kernel_size/2;kh++)
				{
					for(int kv=-1*kernel_size/2;kv<=kernel_size/2;kv++)
					{
						if(i+kh>=0 && i+kh<input.width()&&j+kv>=0&&j+kv<input.height())
						{
							values_under_kernel[iter]=input.pixel(i+kh,j+kv);
							iter++;
						}
					}
				}
				iter=0;
				bubble_sort(values_under_kernel,kernel_size*kernel_size);
				temp.setPixel(i,j,values_under_kernel[kernel_size*kernel_size/2]);
			}
		}	
		
	
	output=temp;	
}


void LeptonThread::bubble_sort(int tab[],int size)
{
for(int j=2;j<size;j++)
{
	for(int i =1;i<size;i++)
		{
		if(tab[i-1]>tab[i])
			{
				int temp = tab[i];
				tab[i]=tab[i-1];
				tab[i-1]=temp;


			}
		}
	}
}
int LeptonThread::find_mediane_in_tab(int tab[],int size)
{
	int most_popular=0;
	int temp=0;
	int mp_index=0;
	for(int i =1;i<size;i++)
	{
		if(tab[i-1]==tab[i])
			{
			temp++;
			if(temp>most_popular)
				{
				most_popular=temp;
				mp_index=i;
				}		
			}
		else temp=0;
	
	}
	if(most_popular>1)return tab[mp_index];
	return 0;
}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
