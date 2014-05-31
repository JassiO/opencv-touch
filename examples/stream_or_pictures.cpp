#include <BSystem.h>
#include <BCamera.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

int SYSTEM_INPUT = 0; // use a picture; change it to 1 to use the stream

baumer::BCamera* g_cam = 0;
baumer::BSystem* g_system  = 0;

int width = 640;
int height = 480;

Mat img;
Mat background;
char key;

void cleanup();
void init_camera();
void open_stream(int width, int height);
void get_contours(IplImage* frame);

int main() {

	/////////// STREAM //////////
	if (SYSTEM_INPUT == 1) {
		namedWindow("Stream", CV_WINDOW_AUTOSIZE);
		namedWindow("Contour", CV_WINDOW_AUTOSIZE );

		// initialize baumer camera
		init_camera();

		// infinte loop for the stream
		open_stream(width, height);
	}
	
	////////// PICTURE //////////
	else {
		namedWindow("Image", CV_WINDOW_AUTOSIZE);
		namedWindow("Contour", CV_WINDOW_AUTOSIZE);

		// load image
		IplImage* img = cvLoadImage("touchevent.png", 1);
		if (img == NULL) {
			std::cout << "Error: couldn't load image" << std::endl;
			return 0;
		}

		// show loaded image
		cvShowImage("Image", img);

		get_contours(img);

		// wait until any key is pressed
		cvWaitKey(0);
	}

	return 0;
}

void init_camera() {
	g_system = new baumer::BSystem;
	g_system->init();
	if(g_system->getNumCameras() == 0) {
		std::cerr << "Error: no camera found" << std::endl;
		return;
	}

	g_cam = g_system->getCamera(0 /*the first camera*/, false /*use not rgb*/, false /*not fastest mode but full resolution*/);
	
	width = g_cam->getWidth();
	height = g_cam->getHeight();
}

void open_stream(int width, int height) {
	while (true) {
		if (!g_cam->capture() == NULL) {
			if (g_cam->capture() == NULL) {
				std::cout << "Error: stream is empty" << std::endl;
			}
			else {
				// save the data stream in each frame
				IplImage* frame = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 1);
				frame -> imageData = (char*) g_cam-> capture();

				// show stream
				cvShowImage("Stream", frame);
   
		        if (char(key) == 27) { // ESC breaks the loop
		            break;      
		        }
		        else if (char(key) == 32) { // Space saves the current image
		        	cvSaveImage("current.png", frame);
		        }
		        else if (char(key) == 10) { // Enter takes an image of the background
		        	background = Mat(frame);
		        	imshow("Background", background);
		        }

				get_contours(frame);

		        key = cvWaitKey(10); // throws a segmentation fault
	    	}
		}
	}
}

void get_contours(IplImage* frame) {
	
	// calculate contours
	Mat img(frame);
	//img = img - background;
	//imshow("Background2", img);
	Mat img2;
	if (SYSTEM_INPUT == 0) {
		cvtColor(img, img, CV_BGR2GRAY);
	}
	threshold(img, img2, 65, 255, CV_THRESH_BINARY); // threshold [0, 255] // _INV

	// detect contours
	vector<vector<Point> > contours;
	vector<vector<Point> > contours0;

	findContours( img2, contours0, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE );

	contours.resize(contours0.size());
	for( size_t k = 0; k < contours0.size(); k++ )
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	// draw contours
	Mat cnt_img = Mat::zeros(img2.rows, img2.cols, CV_8UC3);
	drawContours(cnt_img, contours, -1, Scalar(128,255,255));

	// show contours
	imshow("Contour", cnt_img);
}