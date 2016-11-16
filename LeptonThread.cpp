#include "LeptonThread.h"

#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"
#include <math.h>
#include <string>
#include <iostream>

#define PACKET_SIZE 164
#define PACKET_ SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27;
#define PI 3.1415


LeptonThread::LeptonThread() : QThread()
{
    mode = false; //normal display mode
    mediane_on = false;
    mode_concave = false;
    histogram_on = false;
    counting_contours_on = false;
    mode_hull = false;
    draw_line = false;
    recognize = false;
    rescale = false;
    sub_background = false;
    save_prev = false;
    colormap = colormap_rainbow;
    ppmode = 0;
    slider_value_binary = 150;
    slider_value_canny = 80;
    width = 80;
    height = 60;
    image_params = Mat::zeros(Size(width,height),CV_8UC1);
    image_params = 255;
    cont = Mat::zeros(opencvmat.size(),CV_8UC1);
    mask = Mat::zeros(opencvmat.size(),CV_8UC1);
    image_hull = Mat::zeros(opencvmat.size(),CV_8UC1);
    opencvmat_values = Mat::zeros(Size(width,height),CV_16UC1);
    background = Mat::zeros(Size(width,height),CV_16UC1);
    hottest_point = 0;
    coolest_point = 65535;
    Point top_hand = Point(0,0);
    depth_thresh = 10;

    for(int i = 0;i < 6;i++)
    {
        sequence.push_back(Mat::zeros(Size(width,height),CV_8UC1));
    }
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
            int temp = result[i * 2];
            result[i*2] = result[i * 2 + 1];
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


            for(int i = 0;i < FRAME_SIZE_UINT16 ;i++)
				{
                if(i % PACKET_SIZE_UINT16 < 2)continue;

                    value = (frameBuffer[i] - minValue) * scale;
                    column = (i % PACKET_SIZE_UINT16 ) - 2;
                    row = i / PACKET_SIZE_UINT16;
                    Scalar color(colormap[3 * value],colormap[3 * value + 1],colormap[3 * value + 2]);
                    opencvmat.at<Vec3b>(Point(column,row))[0] = color[2];
                    opencvmat.at<Vec3b>(Point(column,row))[1] = color[1];
                    opencvmat.at<Vec3b>(Point(column,row))[2] = color[0];
                    opencvmat_base.at<uchar>(Point(column,row)) = value;
                    opencvmat_values.at<short>(Point(column,row)) = (frameBuffer[i] - minValue);
                }

        postprocessing(opencvmat);
        Mat output;
        opencvmat.copyTo(output);
        cvtColor(output ,output ,CV_BGR2RGB);
        separate_hand();
        QImage  myImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888);
        emit updateImage(myImage);

	}

    SpiClosePort(1);
}

//voids

void LeptonThread::make_snapshot()
{
    time_t t = time(0);
    struct tm * now = localtime(& t);
    std::ostringstream ss;
    ss << now-> tm_hour * 3600 + now -> tm_min * 60 + now -> tm_sec;
   // namedWindow(ss.str(),CV_WINDOW_NORMAL);
    //setWindowProperty(ss.str(),CV_WND_PROP_FULLSCREEN,CV_WINDOW_KEEPRATIO);
   //imwrite(ss.str()+"d"+".jpg",image_params);
   // imshow(ss.str(),opencvmat);
   // imwrite(ss.str()+"t"+".jpg",opencvmat);
   // imwrite(ss.str()+"m" +".jpg",mask);
   // imwrite(ss.str()+".jpg", cont);
   if(mode_hull) imwrite(ss.str() + "hu" + ".jpg",image_hull);
   if(histogram_on) imwrite(ss.str()+ "h" + ".jpg",image_histogram);
   if(counting_contours_on) imwrite(ss.str() + "c" + ".jpg" ,Canny_conts);
   imwrite(ss.str() + "con" + ".jpg", hand_convex);
   if(save_prev)
   {
        for(uint i = 0;i < sequence.size() ;i++)
        {
             ss<<i;
             imwrite(ss.str() + "con"+".jpg",sequence[i]);
        }
   }
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
     Scharr(input, grad_x, input.depth(), 1, 0, 1, 0, BORDER_DEFAULT);
     Scharr(input, grad_y, input.depth(), 0, 1, 1, 0, BORDER_DEFAULT);
     convertScaleAbs(grad_x,grad_x);
     convertScaleAbs(grad_y,grad_y);
     output=grad_x+grad_y;
}

void LeptonThread::separate_hand()
{
    mask = Mat::zeros(opencvmat.size(),CV_8UC3);
    if(!sub_background)
    {
       mask=get_mask_classic();
    }
    else
    {
        mask = sub_BG();
    }
    if(!is_Mat_empty(mask))
    {
        return;
    }

    mask = correct_mask(mask);
    postprocessing(mask);
    cont= get_cont_and_mask(mask);

    std::vector<std::vector<cv::Point> > contours = get_vector_with_conts(cont);
    int index_biggest=ruturn_biggest_index(contours);
    double hull_coverage = 0;
    double conv_points = 0; //becouse of global point


    if(draw_line) cont= cut_wirst(cont,contours,index_biggest);//changes mask also
    if(mode_concave)conv_points =  concave(cont,contours,index_biggest); //conv_points +
    if(rescale) cont = rescale_hand(mask);
    if(mode_hull) hull_coverage = draw_convex_hull(cont,index_biggest);
    if(histogram_on) histogram_alternative(cont);
    //if(counting_contours_on) contour_ratio = counting_contour(cont,mask);
    if(recognize) recognize_gesture(conv_points, hull_coverage);

    Size new_size(400,300);
    resize(cont,cont,new_size);
    imshow("contour",cont);
    imshow("mask",mask);
}

double LeptonThread::draw_convex_hull(Mat image, int biggest)
{
    Mat temp;
    imshow("rasia1",image);
    image.copyTo(temp);
    std::vector<std::vector<Point> > hull;
    Mat mask_temp;
    cvtColor(mask, mask_temp, CV_BGR2GRAY);
    findContours (mask_temp, hull, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    int index_biggest = ruturn_biggest_index(hull);
    Point2f center;
    float radius;
    minEnclosingCircle(Mat(hull[index_biggest]),center,radius);

    Mat temp_gray;
    temp.copyTo(temp_gray);
    image_hull = temp;

    Mat field = Mat::zeros(temp_gray.size(), CV_8UC1);
    Scalar white(255, 255, 255);
    circle(field ,center ,radius, white, -1);

    int field_under_circle = countNonZero(field);
    cvtColor(temp_gray, temp_gray, CV_BGR2GRAY);

    int field_hand = countNonZero(temp_gray);
    double ratio = (double)field_hand / field_under_circle;


    imshow("rasia",temp_gray);
    imshow("circle",field);

    std::ostringstream ss;
    ss<<hull[biggest].size();
    putText(image_params,"Hull:"+ ss.str()+ "/n" ,Point(5,30), 1, 2, 0);
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

    calcHist(&bgr[0], 1, 0, Mat(), b_hist, 1, &hist_size, &histRange, uniform, accumulate);
    calcHist(&bgr[1], 1, 0, Mat(), g_hist, 1, &hist_size, &histRange, uniform, accumulate);
    calcHist(&bgr[2], 1, 0, Mat(), r_hist, 1, &hist_size, &histRange, uniform, accumulate);

    int hist_w = 512; int hist_h = 300 * 3;
    int bin_w = cvRound ((double)hist_w/hist_size);

    Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(255,255,255));

    int bottom = hist_h / 10 * 9;


    if(colormap==colormap_grayscale)
    {
        for(int i =1;i <hist_size;i++)
        {
            rectangle(histImage,Point(bin_w*i, bottom),Point(bin_w*(i+1) ,bottom-4*cvRound(b_hist.at<float>(i))), Scalar(0,0,0),-1);
        }
    }

     else
    {
        for(int i =1;i <hist_size;i++)
        {
            rectangle(histImage,Point(bin_w*i, bottom/3),Point(bin_w*(i+1) ,bottom/3-cvRound(b_hist.at<float>(i))), Scalar(255,0,0),-1);
            rectangle(histImage,Point(bin_w*i, bottom*2/3),Point(bin_w*(i+1) ,bottom*2/3-cvRound(g_hist.at<float>(i))), Scalar(0,255,0),-1);
            rectangle(histImage,Point(bin_w*i, bottom),Point(bin_w*(i+1) ,bottom-cvRound(r_hist.at<float>(i))), Scalar(0,0,255),-1);
        }
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
    cvtColor(temp_mask, temp_mask, CV_BGR2GRAY);
    cvtColor(temp, temp, CV_BGR2GRAY);
    Canny(temp, temp, slider_value_canny ,slider_value_canny*4/3 ,3);

    std::vector<std::vector<cv::Point> > contours1;
    Mat temp_find_con_external;
    temp.copyTo(temp_find_con_external);
    findContours (temp_find_con_external,contours1,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    resize(temp_find_con_external, temp_find_con_external, Size(400,320));
    imshow("hejo2",temp_find_con_external);
    int number_of_pixels_countour = countNonZero(temp);
    int number_of_pixels_hand = countNonZero(temp_mask);


    double ratio = (double)number_of_pixels_hand/number_of_pixels_countour;


    emit updateText("countur:"+ QString::number(number_of_pixels_countour)+ "\n"
                    +"Field:"+QString::number(number_of_pixels_hand)+"\n"
                    +"Radio:"+QString::number(ratio,'f',2));

    Mat temp_find_con;
    temp.copyTo(temp_find_con);
    std::vector<std::vector<cv::Point> > contours;
    findContours (temp_find_con,contours,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);//CV_CHAIN_APPROX_SIMPLE , CV_CHAIN_APPROX_TC89_L1, CV_CHAIN_APPROX_TC89_KCOS,CV_CHAIN_APPROX_NONE

    int all_contours=0;
    for(uint i = 0; i<contours.size();i++ )
    {
       if(contours[i].size()>4) all_contours++;
    }

    double ratio_hull=(double)number_of_pixels_hand/all_contours;
    emit updateTextContoursHull("All cont:" + QString::number(contours.size())+"\n"
                                 +"Ratio:"+QString::number(ratio_hull,'f',2));

    Canny_conts=temp;
    resize(temp,temp,Size(400,320));
    imshow("hejo",temp);

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

void LeptonThread::recognize_gesture(int conv_points, double hull)
{
    double coverage_ratio_threshold = 0.64;
    if(conv_points == 1)
    {
        emit updateTextReco("Scizors");
    }
    else if(conv_points > 1)
    {
        emit updateTextReco("Paper");
    }
    else
    {
        if (hull > coverage_ratio_threshold)
        {
            emit updateTextReco("Stone");
        }
        else
        {
            emit updateTextReco("Paper");
        }

    }

}

Mat LeptonThread::rescale_hand(Mat image_mask)
{

    int minvalue = 65535;
    int maxvalue = 0;
    Mat mask_gray = Mat::zeros(image_mask.size(), CV_8UC1);
    cvtColor(mask, mask_gray, CV_BGR2GRAY);

    for(int i=0;i<mask_gray.cols;i++)
    {
     for(int j=0;j<mask_gray.rows;j++)
        {        
        int pixel_val = opencvmat_values.at<short>(Point(i,j));
        uchar pixel_con = mask_gray.at<uchar>(Point(i,j));
        if(pixel_con == 0) continue;

        if(pixel_val < minvalue) minvalue = pixel_val;
        else if(pixel_val > maxvalue) maxvalue = pixel_val;

        }
    }
    if(maxvalue>hottest_point)hottest_point=maxvalue;
    else maxvalue=hottest_point;

    if(minvalue<coolest_point)coolest_point=minvalue;
    else minvalue= coolest_point;

    emit updateTextContours(QString::number(maxvalue)+" "+QString::number(minvalue));

    float diff = maxvalue - minvalue;
    float scale = 255/diff;

    Mat temp=Mat::zeros(Size(width,height), CV_8UC3);

    for(int i = 0;i < mask_gray.cols;i++)
    {
     for(int j = 0;j < mask_gray.rows;j++)
        {
         uchar pixel_con = mask_gray.at<uchar>(Point(i,j));
         if(pixel_con == 0)continue;
         int pixel = (int)(opencvmat_values.at<short>(Point(i,j)) - minvalue) * scale;
         Scalar color(colormap[3 * pixel], colormap[3 * pixel + 1] ,colormap[3 * pixel + 2]);
         temp.at<Vec3b>(Point(i,j))[0] = color[2];
         temp.at<Vec3b>(Point(i,j))[1] = color[1];
         temp.at<Vec3b>(Point(i,j))[2] = color[0];
        }
    }

  return temp;
}

Mat LeptonThread::cut_wirst(Mat img_hand, std::vector<std::vector<cv::Point> >conto, int biggest)
{
     Mat temp(opencvmat.size(),CV_8UC3);
     img_hand.copyTo(temp);

     Vec4f lines = draw_fit_line(temp,conto[biggest]);

     Mat temp_for_line = Mat::zeros(temp.size(),CV_8UC1);
     Mat temp_mask;
     mask.copyTo(temp_mask);
     cvtColor(temp_mask,temp_mask,CV_BGR2GRAY);

     int lenght = 50;
     int i = -temp.cols/2;
     int wirsts_values [temp.cols];

     while(i < temp.cols/2)
     {
         Point base(lines[2]-i*lines[0],lines[3]-i*lines[1]);
         line(temp_for_line,Point(base.x -1*lines[1]*lenght, base.y +lines[0]*lenght),Point(base.x +lines[1]*lenght, base.y-lines[0]*lenght),Scalar(255,255,255),1);

         int line_lenght = countNonZero(temp_mask&temp_for_line);
         wirsts_values[i+temp.cols/2]=line_lenght;

         i=i+1;
         temp_for_line= Mat::zeros(temp.size(),CV_8UC1);
     }


     int index_of_smallest = calc_wirst(temp, wirsts_values,temp.cols) - temp.cols/2;
     Point base_smallest(lines[2]-index_of_smallest*lines[0],lines[3]-index_of_smallest*lines[1]);
     line(temp,Point(base_smallest.x -1*lines[1]*lenght, base_smallest.y +lines[0]*lenght),Point(base_smallest.x +lines[1]*lenght, base_smallest.y-lines[0]*lenght),Scalar(0,0,0),3);

     std::vector<std::vector<cv::Point> > contours;
     Mat temp_gray;
     cvtColor(temp, temp_gray, CV_BGR2GRAY);

     findContours (temp_gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_SIMPLE , CV_CHAIN_APPROX_TC89_L1, CV_CHAIN_APPROX_TC89_KCOS,CV_CHAIN_APPROX_NONE
     mask = Mat::zeros(opencvmat.size(), CV_8UC3);
     circle(temp, top_hand,2,255);
     imshow("temp", temp);

     int index_of_cont_hand = 0;
     for(uint i = 0;i < contours.size();i++)
     {
         if(pointPolygonTest(contours[i], top_hand, true) >= pointPolygonTest(contours[index_of_cont_hand], top_hand, true))
         {
             index_of_cont_hand = i;
         }
     }

     drawContours(mask, contours, index_of_cont_hand, Scalar(255,255,255), CV_FILLED);
     temp = temp & mask;
     update_depth_threshold(mask,contours[index_of_cont_hand]);
     return temp;
}

bool LeptonThread::find_direction(Mat img_hand, Point point_throught)
{
    Mat temp;
    img_hand.copyTo(temp);
    cvtColor(temp, temp, CV_BGR2GRAY);
    Rect rect_left(Point(0,0) ,Point(point_throught.x, 60));
    Rect rect_right(Point(point_throught.x, 0) ,Point(80, 60));
    Mat left = temp(rect_left);
    Mat right = temp(rect_right);
    if(countNonZero(left) > countNonZero(right))
    {
        return false;
    }
    return true;
}

Vec4f LeptonThread::draw_fit_line(Mat image_hand, std::vector<cv::Point> contour)
{
    Vec4f main_line;
    fitLine(contour, main_line, 2, 0, 0.01, 0.01);
    int lefty=(-main_line[2] * main_line[1] / (main_line[0] + 0.001)) + main_line[3];
    int righty = ((image_hand.cols - main_line[2]) * main_line[1] / main_line[0]) + main_line[3];
    line(image_hand,Point(image_hand.cols - 1,righty),Point(0,lefty),Scalar(255,0,0),1);
    return main_line;
}

int LeptonThread::calc_wirst(Mat img_hand, int* tab_of_wirsts,int size)
{
    int index_biggest = 0;
    for (int i = 0;i < size; i++)
    {
        if(tab_of_wirsts[i] > tab_of_wirsts[index_biggest])
        {
            index_biggest = i;
        }
    }
    bool direction = find_direction(img_hand, top_hand);

    int index_smallest = 0;


    if(!direction)
    {
        int start = index_biggest + tab_of_wirsts[index_biggest];
        if (start < 0)
        {
            start = 0;
        }
        index_smallest = start;
        for(int i = index_biggest;i < start;i++)
        {
            if(tab_of_wirsts[index_smallest] > tab_of_wirsts[i])
            {
                index_smallest = i;
            }
        }

    }
    else
    {
        //circle(img_hand,top_hand,5,255);
        int start = index_biggest - tab_of_wirsts[index_biggest];

        if (start < 0)
        {
            start = 0;
        }
        index_smallest = start;
        /*
        for(int i = start;i < index_biggest;i--)
        {
            if(tab_of_wirsts[index_smallest] > tab_of_wirsts[i])
            {
                index_smallest = i;
            }
        }
       */
    }
    return index_smallest;
}

void LeptonThread::get_BG()
{
    opencvmat_values.copyTo(background);
}

Mat LeptonThread::sub_BG()
{
    Mat temp=Mat::zeros(Size(width,height), CV_16UC1);
    temp= abs(opencvmat_values- background);
    Mat output=Mat::zeros(temp.size(), CV_8UC3);
    for(int i = 0;i < temp.cols;i++)
    {
     for(int j = 0;j < temp.rows;j++)
        {
        int pixel_val = temp.at<short>(Point(i,j));
         if(pixel_val > 40)
         {
            output.at<Vec3b>(Point(i,j))[0] = 255;
            output.at<Vec3b>(Point(i,j))[1] = 255;
            output.at<Vec3b>(Point(i,j))[2] = 255;
         }
        }
    }
    return output;
}

Mat LeptonThread::correct_mask(Mat mask_to_correct)
{
    Mat temp;
    mask_to_correct.copyTo(temp);
    cvtColor(temp, temp, CV_BGR2GRAY);
    temp = temp & opencvmat_base;
    threshold(temp, temp, slider_value_binary, 255, THRESH_BINARY);
    cvtColor(temp, temp, CV_GRAY2BGR);
    return temp;
}

int LeptonThread::concave(Mat image_cont, std::vector<std::vector<Point> > conto, int biggest)
{

    Mat temp(image_cont.size(), CV_8UC3);
    image_cont.copyTo(temp);
    int conter_all = 0;
    int counter_big = 0;
    if(conto.size() > 0)
    {
        std::vector<std::vector<int> > hull(conto.size());
        convexHull(Mat(conto[biggest]), hull[biggest], false);

        std::vector<std::vector<Point> > hull_to_draw(conto.size());
        convexHull(Mat(conto[biggest]), hull_to_draw[biggest], false);

        std::vector<std::vector<Vec4i> > convdef(conto.size());
        std::vector<Point>  def_points;




        if(hull.size() > 0 && hull[biggest].size() > 2)
        {
            drawContours(temp,hull_to_draw, biggest, Scalar(255,0,255), 1, 8, std::vector<Vec4i>(), 0, Point());
            convexityDefects(conto[biggest], hull[biggest], convdef[biggest]);

            int biggest_conv_index = 0;

            for (uint i = 0 ; i < convdef[biggest].size();i++)
            {                  
                conter_all++;
                int ind_0 = convdef[biggest][i][0];//start
                int ind_1 = convdef[biggest][i][1];//end
                int ind_2 = convdef[biggest][i][2];//defect point
                circle(temp, conto[biggest][ind_0], 2, Scalar(0,0,255), -1);
                circle(temp, conto[biggest][ind_1], 2, Scalar(0,255,0), -1);

                double angle = calc_angle(conto[biggest][ind_0],conto[biggest][ind_1],conto[biggest][ind_2]);
                if((convdef[biggest][i][3] > 20 * slider_value_canny) && (angle < 55) && (angle > 10))
                    {
                        circle(temp, conto[biggest][ind_2], 2, Scalar(255,255,0), -1);
                        counter_big++;
                    }
                else
                    {
                        circle(temp, conto[biggest][ind_2], 2, Scalar(255,0,0), -1);
                    }
                def_points.push_back(conto[biggest][ind_1]);

               if(convdef[biggest][i][3] > convdef[biggest][biggest_conv_index][3])
                    {
                    biggest_conv_index = i;
                    }
           }
           Point center(0,0);
           for(uint i = 0;i < def_points.size();i++)
           {
               center.x += def_points[i].x;
               center.y += def_points[i].y;
           }
            center.x = center.x / def_points.size();
            center.y = center.y / def_points.size();
            circle(temp, center, 2, Scalar(150,255,250), -1);

            Point direct((conto[biggest][convdef[biggest][biggest_conv_index][0]].x + conto[biggest][convdef[biggest][biggest_conv_index][1]].x ) / 2,(conto[biggest][convdef[biggest][biggest_conv_index][0]].y + conto[biggest][convdef[biggest][biggest_conv_index][1]].y ) / 2);
            top_hand = center;
            circle(temp,direct,2,Scalar(255,255,250),-1);

            //contours_on_fingers(temp,center,direct);
        }
    }

    resize(temp, temp, Size(400,320));
    std::ostringstream ss1;
    ss1 << conter_all;
    std::ostringstream ss2;
    ss2 << counter_big;
    std::ostringstream ss3;
    Mat temp_gray;
    cvtColor(temp, temp_gray, CV_BGR2GRAY);
    int field = countNonZero(temp_gray);
    double ratio = (double)field / conter_all;
    ss3 << ratio;
    putText(temp, ss1.str(), Point(360,280), 1,1, Scalar(255,255,255));
    putText(temp, ss2.str(), Point(380,280), 1,1, Scalar(255,255,255));
    putText(temp, ss3.str(), Point(340,260), 1,1, Scalar(255,255,255));
    hand_convex = temp;
    manage_history(hand_convex);
    imshow("convex_point", temp);

    return counter_big;
}

double LeptonThread::calc_angle(Point point_begin, Point point_end, Point point_conv)
{
     double c = cv::norm(point_begin - point_end);
     double a = cv::norm(point_begin - point_conv);
     double b = cv::norm(point_end - point_conv);
     double angle = acos(double(b * b + a * a - c * c)/(2 * a * b)) * 180.0 / PI;
     return angle;
}

Mat LeptonThread::get_mask_classic()
{
    Mat temp;
    opencvmat_base.copyTo(temp);
    threshold(temp,temp, slider_value_binary, 255, THRESH_BINARY);
    std::vector<std::vector<cv::Point> > contours;
    int index_biggest = 0;
    findContours (temp, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_SIMPLE , CV_CHAIN_APPROX_TC89_L1, CV_CHAIN_APPROX_TC89_KCOS,CV_CHAIN_APPROX_NONE

    for(uint i = 0;i < contours.size();i++)
    {
        if(contours[i].size() > contours[index_biggest].size())
        {
            index_biggest = i;
        }
    }
    drawContours(mask, contours, index_biggest, Scalar(255,255,255), CV_FILLED);

return mask;

}

Mat LeptonThread::get_cont_and_mask(Mat image_mask)
{
   return image_mask & opencvmat;
}

std::vector<std::vector<cv::Point> > LeptonThread::get_vector_with_conts(Mat image_mask_and_conts)
{
    std::vector<std::vector<cv::Point> > conto;
    cvtColor(image_mask_and_conts, image_mask_and_conts, CV_BGR2GRAY);
    findContours (image_mask_and_conts, conto, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    return conto;
}

int LeptonThread::ruturn_biggest_index(std::vector<std::vector<Point> > conto)
{
    int output= 0;
    for(uint i = 0;i < conto.size();i++)
        if(conto[i].size() > conto[output].size())output = i;
    return output;
}

void LeptonThread::manage_history(Mat image)
{
    if(sequence.size() > 5)
    {
    for(uint i = 0;i < sequence.size() - 1;i++)
    {
        sequence[i + 1].copyTo(sequence[i]);
    }
        image.copyTo(sequence[sequence.size() - 1]);
    }
    else
    {
        sequence.push_back(image);
    }
}

void LeptonThread::contours_on_fingers(Mat image,Point fingers_begin,Point fingers_end)
{
    Mat temp;
    image.copyTo(temp);

    float v_x = fingers_begin.x - fingers_end.x;
    float v_y = fingers_begin.y - fingers_end.y;//direction vector
    int mag = sqrt(v_x * v_x + v_y * v_y);
    v_y = v_y / mag;
    v_x = v_x / mag;

    //90 degree swap
    float temp_dir = v_x;
    v_x = -1 * v_y;
    v_y = temp_dir;
    int length = 50;
    line(temp, Point(fingers_begin.x + v_x * length, fingers_begin.y + v_y * length),Point(fingers_begin.x + v_x * length * (-1),fingers_begin.y + v_y * length * (-1)),Scalar(0,0,0));
    imshow("zone",temp);
}

bool LeptonThread::is_Mat_empty(Mat img)
 {
     Mat temp;
     img.copyTo(temp);
     cvtColor(temp,temp,CV_BGR2GRAY);
     if(countNonZero(temp) == 0)
     {
         return false;
     }
     return true;
 }

void LeptonThread::update_depth_threshold(Mat img_hand, std::vector<Point>  conto)
{

    RotatedRect min_rect = minAreaRect(Mat(conto));
    Point2f rect_points[4];
    min_rect.points(rect_points);

    int side1 = cv::norm(rect_points[0] - rect_points[1]);
    int side2 = cv::norm(rect_points[1] - rect_points[2]);
    if (side1 > side2)
    {
        depth_thresh = side1;
    }
    else
    {
        depth_thresh = side2;
    }
    for(int i =0 ;i < 4;i++)
    {
        line(img_hand,rect_points[i],rect_points[i % 4],255,1);
    }
      imshow("img",img_hand);

}

//////////////////////////
bool LeptonThread::return_mode()
{
    if(mode == 1)return true;
    return false;
}
void LeptonThread::change_colormap_rainbow()
{
    colormap = colormap_rainbow;
}
void LeptonThread::change_colormap_gray()
{
    colormap = colormap_grayscale;
}
void LeptonThread::change_colormap_iron()
{
     colormap = colormap_ironblack;
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
    mode = 1;
}
void LeptonThread::set_normal_mode()
{
    mode = 0;
    ppmode = 0;
    mediane_on = false;
}
void LeptonThread::change_slider_value_binary(int value)
{
    slider_value_binary = value;
}
void LeptonThread::change_slider_value_canny(int value)
{
    slider_value_canny = value;
}
void LeptonThread::switchon_dilatation()
{
    ppmode = 1;
    }
void LeptonThread::switchon_erosion()
{
    ppmode = 2;
}
void LeptonThread::switchon_open()
{
    ppmode = 3;
}
void LeptonThread::switchon_close()
{
    ppmode = 4;
}
void LeptonThread::switchon_sobel()
{
    ppmode = 5;
}
void LeptonThread::switchon_skeleton()
{
    ppmode = 6;
}
void LeptonThread::switchon_mediane()
{
    ppmode = 7;
}
void LeptonThread::switchon_conting_countour()
{
    counting_contours_on = !counting_contours_on;
}
void LeptonThread::switchon_histogram()
{
    histogram_on =! histogram_on;
}
void LeptonThread::switchon_hull()
{
    mode_hull =! mode_hull;
}
void LeptonThread::switchon_line()
{
    draw_line =! draw_line;
}
void LeptonThread::switchon_recognize()
{
    recognize =! recognize;
}
void LeptonThread::switchon_rescale()
{
    rescale =! rescale;
    hottest_point = 0;
    coolest_point = 65535;
}
void LeptonThread::switchon_subbg()
{
   sub_background =! sub_background;
}
void LeptonThread::switchon_concave()
{
    mode_concave =! mode_concave;
}
void LeptonThread::switchon_save_prev()
{
    save_prev =! save_prev;
}
