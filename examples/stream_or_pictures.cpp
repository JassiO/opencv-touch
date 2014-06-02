#include <BSystem.h>
#include <BCamera.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace cv;

int SYSTEM_INPUT = 0; // use a picture; change it to 1 to use the stream

baumer::BCamera* g_cam = 0;
baumer::BSystem* g_system  = 0;

int width = 640;
int height = 480;


Mat frameMat;
char key;
Mat back;
Mat background;
Ptr<BackgroundSubtractor> pMOG;
Mat MaskMOG;

void cleanup();
void init_camera();
void open_stream(int width, int height, Ptr<BackgroundSubtractor> pMOG);
void get_contours(Mat img_cont);

int main() {

	/////////// STREAM //////////
	if (SYSTEM_INPUT == 1) {
		namedWindow("Stream", CV_WINDOW_AUTOSIZE);
		namedWindow("Contour", CV_WINDOW_AUTOSIZE );
		namedWindow("FG Mask MOG", CV_WINDOW_AUTOSIZE);

		pMOG = new BackgroundSubtractorMOG();

		// initialize baumer camera
		init_camera();

		// infinte loop for the stream
		open_stream(width, height, pMOG);
	}
	
	////////// PICTURE //////////
	else {
		namedWindow("Image", CV_WINDOW_AUTOSIZE);
		namedWindow("Contour", CV_WINDOW_AUTOSIZE);

		// load image
		Mat img;
		img = imread("touchevent.png", CV_LOAD_IMAGE_COLOR);
		if (!img.data) {
			std::cout << "Error: couldn't load image" << std::endl;
			return 0;
		}

		// show loaded image
		imshow("Image", img);

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

void open_stream(int width, int height, Ptr<BackgroundSubtractor> pMOG) {
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
		        }

		        // substract the background image, if possible
		        if (background.size().width > 0 && background.size().height > 0) {
			        pMOG->operator()(background, MaskMOG);
			        MaskMOG.inv();
			        frameMat = MaskMOG;
			    }
			    else {
			    	frameMat = Mat(frame);
			    }

				get_contours(frameMat);

		        key = cvWaitKey(10); // throws a segmentation fault (?)
	    	}
		}
	}
}

void get_contours(Mat img_cont) {
	
	// calculate contours
	Mat img2;
	if (SYSTEM_INPUT == 0) {
		cvtColor(img_cont, img_cont, CV_BGR2GRAY);
		threshold(img_cont, img2, 65, 255, CV_THRESH_BINARY);
	}
	else {
		threshold(img_cont, img2, 20, 255, CV_THRESH_BINARY); // smaller threshold possible because of the background substraction
	}

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