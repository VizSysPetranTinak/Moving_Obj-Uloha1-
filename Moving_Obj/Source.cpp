#define _CRT_SECURE_NO_WARNINGS

#include <sstream>
#include <stdlib.h> 
#include <string>
#include <iostream>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <vector>
#include <opencv2\opencv.hpp>



using namespace cv;
using namespace std;
//Poèiatoèné min and max hodnoty HSV filtra .
int H_MIN = 89;
int H_MAX = 117;
int S_MIN = 146;
int S_MAX = 212;
int V_MIN = 110;
int V_MAX = 183;


int clr = 0;
int xold = 0, yold = 0;
int polex[5] = {0,0,0,0,0};
int poley[5] = {0,0,0,0,0};

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

const int MAX_NUM_OBJECTS = 50;

const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;

const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Farba";
void on_trackbar(int, void*)
{
	if (clr == 0) {
		H_MIN = 89;
		H_MAX = 117;
		S_MIN = 146;
		S_MAX = 212;
		V_MIN = 110;
		V_MAX = 183;
	}
	if (clr == 1) {
		H_MIN = 51;
		H_MAX = 86;
		S_MIN = 105;
		S_MAX = 256;
		V_MIN = 137;
		V_MAX = 256;
	}
	if (clr == 2) {
		H_MIN = 0;
		H_MAX = 126;
		S_MIN = 126;
		S_MAX = 256;
		V_MIN = 214;
		V_MAX = 256;
	}
	if (clr == 3) {
		H_MIN = 140;
		H_MAX = 212;
		S_MIN = 142;
		S_MAX = 249;
		V_MIN = 186;
		V_MAX = 256;
	}





}
string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars() {
	//okno pre vo¾bu farby ktorú budeme sledova


	namedWindow(trackbarWindowName, 0);

	char TrackbarName[50];
	sprintf(TrackbarName, "B G Y R", clr);

	//Podoba okna
	      
	createTrackbar("B G Y R", trackbarWindowName, &clr, 3, on_trackbar);
}

void drawObject(int x, int y, Mat &frame) {

	//Funkcia vykreslujúca kurzor

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);

	//vektory potrebné pre findContours funkciu
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//OpenCv funkcia pre h¾adanie objektu konkrétnej farby
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//filter v prípade že máme v obraze viac objektov ako zadefinované
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				xold = x;
				yold = y;

				/*for (int j = 1; j > 4; j++) {
					polex[j + 1] = polex[j];
					poley[j + 1] = poley[j];
				}*/

				//Hladáme objekt s najväèšou plochou
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					//polex[1] = moment.m10 / area;
					//poley[1] = moment.m01 / area;
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;

				/*polex[1] = x;
				poley[1] = y;

				for (int j = 2; j > 5; j++) {
					x += polex[j];
					y += poley[j];
				}

				x = x/5;
				y = y/5;*/
			}
			//Nájdený objekt v obraze + kurzor
			
			if ((objectFound == true)&&(abs(x-xold)>0.5)&&((abs(y - yold)>0.5))) {
				putText(cameraFeed, "Objekt v obraze", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				
				drawObject(x, y, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
int main(int argc, char* argv[])
{
	//program
	bool trackObjects = true;
	bool useMorphOps = true;
	//Matica zaznamenávajúca jednotlivé snímky
	Mat cameraFeed;

	//matica pre HSV obraz
	Mat HSV;

	//matica pre binárny obrázok prahových hodnôt
	Mat threshold;

	int x = 0, y = 0;

	//vytvorenie okna pre vo¾bu farby
	createTrackbars();
	
	VideoCapture capture;
	capture.open(0);

	//nastavenie výšky a šírky snímky záznamu
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	//Program v nekoneènom cykle
	while (1) {
		capture.read(cameraFeed);
		//konvercia snímky z BGR do HSV
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);

		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);

		if (useMorphOps)
			morphOps(threshold);

		//x,y poloha filtrovaného objektu
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);

		//zobrazenia
		imshow(windowName2, threshold);
		imshow(windowName, cameraFeed);
		//imshow(windowName1, HSV);


		waitKey(30);
	}






	return 0;
}