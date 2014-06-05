#include <BSystem.h>
#include <BCamera.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////																							/////
/////	mser geht zur zeit gar nicht mehr; an sich läuft alles zwar, 							/////
///// 	der algorithmus ist aber rauskommentiert, da es sonst nur zu Fehlern kommt				/////
/////																							/////
/////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace cv;

int SYSTEM_INPUT = 0; // uses a picture; change it to 1 to use the stream
int ALGORITHM = 2; // uses only contours; change it to 1 to use MSER or to two to use both

baumer::BCamera* g_cam = 0;
baumer::BSystem* g_system  = 0;

int width = 640;
int height = 480;

Mat frameMat, back, background, MaskMOG, img0, ellipses, gray;
char key;
Ptr<BackgroundSubtractor> pMOG;
vector<vector<Point> > contours;

static const Vec3b bcolors[] = {
    Vec3b(0,0,255),
    Vec3b(0,128,255),
    Vec3b(0,255,255),
    Vec3b(0,255,0),
    Vec3b(255,128,0),
    Vec3b(255,255,0),
    Vec3b(255,0,0),
    Vec3b(255,0,255),
    Vec3b(255,255,255)
};

// callbacks
void cleanup();
void init_camera();
void open_stream(int width, int height, Ptr<BackgroundSubtractor> pMOG);
void get_contours(Mat img_cont);
void mser_algo(Mat temp_img);

int main() {

	/////////// STREAM //////////
	if (SYSTEM_INPUT == 1) {
		namedWindow("Stream", CV_WINDOW_AUTOSIZE);
		namedWindow("Contour", CV_WINDOW_AUTOSIZE );

		pMOG = new BackgroundSubtractorMOG();

		// initialize baumer camera
		init_camera();

		// infinte loop for the stream
		open_stream(width, height, pMOG);
	}
	
	////////// PICTURE //////////
	else if (SYSTEM_INPUT == 0) {
		namedWindow("Contour", CV_WINDOW_AUTOSIZE);

		// load image
		Mat img;
		img = imread("touchevent.png", CV_LOAD_IMAGE_COLOR);
		if (!img.data) {
			std::cout << "Error: couldn't load image" << std::endl;
			return 0;
		}

		if (ALGORITHM == 0) {
			    	get_contours(img);
			    }
		else if (ALGORITHM == 1) {
			mser_algo(img);
		}
		else if (ALGORITHM == 2) {
			get_contours(img);
			mser_algo(img);
		}
		else {
			std::cout << "Error: invalide algorithm number" << std::endl;
		}

		// wait until any key is pressed
		cvWaitKey(0);
	}
	else {
		std::cout << "Error: invalide system input number" << std::endl;
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

			    if (ALGORITHM == 0) {
			    	get_contours(frameMat);
			    }
				else if (ALGORITHM == 1) {
					mser_algo(frameMat);
				}
				else if (ALGORITHM == 2) {
					get_contours(frameMat);
					mser_algo(frameMat);
				}
				else {
					std::cout << "Error: invalide algorithm number" << std::endl;
				}

		        key = cvWaitKey(10); // throws a segmentation fault (?)
	    	}
		}
	}
}

void get_contours(Mat img_cont) {
	
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

	Mat cnt_img = Mat::zeros(img2.rows, img2.cols, CV_8UC3);
	if (SYSTEM_INPUT == 0) {
		img_cont.copyTo(cnt_img); // when it is used with the stream. it shows the same image as the mser algorithm
	}
	drawContours(cnt_img, contours, -1, Scalar(128,255,255));

	imshow("Contour", cnt_img);
	

}

void mser_algo(Mat temp_img)  {

	//cvtColor(temp_img, gray, COLOR_BGR2GRAY);

	if (SYSTEM_INPUT == 0) {
		cvtColor(temp_img, temp_img, CV_BGR2GRAY);
	}

	cvtColor(temp_img, img0, COLOR_GRAY2BGR);
	img0.copyTo(ellipses);

	//MSER ms;
	//ms(gray, contours, Mat()); 			//do not work

	if (contours.size() <= 0) {
		std::cout << "Error: no contours extracted" << std::endl; // ERROR ???
	}

	for( int i = (int)contours.size()-1; i >= 0; i-- ) {
		const vector<Point>& r = contours[i];
		for ( int j = 0; j < (int)r.size(); j++ ) {
		    Point pt = r[j];
		    img0.at<Vec3b>(pt) = bcolors[i%9];
		}

		RotatedRect box = fitEllipse( r ); // maybe try cvfitellipse2

		box.angle=(float)CV_PI/2-box.angle;
		ellipse( ellipses, box, Scalar(196,255,255), 2 );
	}

	imshow( "response", img0);
	imshow( "ellipses", ellipses);

}