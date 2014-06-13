#include <BSystem.h>
#include <BCamera.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace cv;

int SYSTEM_INPUT = 0; // uses a picture; change it to 1 to use the stream
int ALGORITHM = 1; // uses only contours; change it to 1 to use MSER or to two to use both

baumer::BCamera* g_cam = 0;
baumer::BSystem* g_system  = 0;

int width = 640;
int height = 480;

Mat frameMat, back, background, MaskMOG, img0, ellipses;
char key;
Ptr<BackgroundSubtractor> pMOG;
vector<vector<Point> > contours;
int skip_first_frame = 0;

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

// mser values
int _delta=1; 				// default: 1; 			good: 1 						[1; infinity]
int _min_area=60; 			// default: 60; 		good: 60 						[1; infinity]
int _max_area=20000;		// default: 14 400; 	good: 20 000 for fingertips		[1; infinity]
double _max_variation=.03; 	// default: 0.25;		good: 0.03 - 0.05				[0; 1]
double _min_diversity=.5;	// default: 0.2;		good: 0.5 - 0.7					[0; 1]

//used only with colored images
int _max_evolution=200;
double _area_threshold=5.5;
double _min_margin=0.003; 
int _edge_blur_size=5;

// callbacks
void cleanup();
void init_camera();
void open_stream(int width, int height, Ptr<BackgroundSubtractor> pMOG);
void get_contours(Mat img_cont);
void mser_algo(Mat temp_img);
void draw_ellipses(vector<vector<Point> > contours, Mat ellipses, Mat img0);

int main() {

	/////////// STREAM //////////
	if (SYSTEM_INPUT == 1) {
		namedWindow("Stream", CV_WINDOW_AUTOSIZE);
		//namedWindow("Ellipses", CV_WINDOW_AUTOSIZE );

		pMOG = new BackgroundSubtractorMOG();

		// initialize baumer camera
		init_camera();

		// infinte loop for the stream
		open_stream(width, height, pMOG);
	}
	
	////////// PICTURE //////////
	else if (SYSTEM_INPUT == 0) {
		//namedWindow("Contour", CV_WINDOW_AUTOSIZE);

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
		        	pMOG->operator()(background, MaskMOG);
		        }
		        else if(char(key) ==  49 ) {
		        	++_delta;
		        }
		        else if(char(key) ==  33 ) {
		        	--_delta;
		        }
		        else if(char(key) ==  50 ) {
		        	_min_area += 100;
		        }
		        else if(char(key) ==  34 ) {
		        	_min_area -= 100;
		        }
		        else if(char(key) ==  51 ) {
		        	_max_area += 100;
		        }
		        else if(char(key) ==  35 ) {
		        	_max_area -= 100;
		        }
		        else if(char(key) ==  52 ) {
		        	_max_variation += 0.2;
		        }
		        else if(char(key) ==  36 ) {
		        	_max_variation -= 0.2;
		        }
		        else if(char(key) ==  53 ) {
		        	_max_variation += 0.1;
		        }
		        else if(char(key) ==  37 ) {
		        	_max_variation -= 0.1;
		        }
		        else if(char(key) ==  54 ) {
		        	g_cam->setGain(1.05 * g_cam->getGain());
		        }
		        else if(char(key) ==  38 ) {
		        	g_cam->setGain(0.95 * g_cam->getGain());
		        }
		        
		        

		        // substract the background image, if possible
		        //if (background.size().width > 0 && background.size().height > 0) {
		        if(background.data) {
			        
			        //MaskMOG.inv();
			        //frameMat = MaskMOG;
					//subtract(Mat(frame),background, frameMat, noArray(), -1);
			        frameMat = MaskMOG - background;
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

				img0 = frameMat;

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

	if (SYSTEM_INPUT == 0) {
		cvtColor(temp_img, temp_img, CV_BGR2GRAY);
	}

	temp_img.copyTo(ellipses);

	MSER ms(_delta, _min_area, _max_area, _max_variation, _min_diversity, _max_evolution, _area_threshold, _min_margin, _edge_blur_size);
	ms(temp_img, contours, Mat()); 		

	std::cout 	<< "Delta: " <<_delta << ", " 
				<< "Min Area: " <<_min_area << ", " 
				<< "Max Area: " << _max_area << ", "
				<< "Max Variation " << _max_variation << ", "
				<< "Min Diversity " << _min_diversity << ", "
				<< "Contours" << contours.size() << std::endl;

	cvtColor(ellipses, ellipses, CV_GRAY2BGR);
	
	if (SYSTEM_INPUT == 0) {
		draw_ellipses(contours, ellipses, img0);
		
	}
	else {
		if (skip_first_frame == 1) {
			draw_ellipses(contours, ellipses, img0);
		}

		if(skip_first_frame == 0) {
			++skip_first_frame;
		}
	}

	//imshow( "response", img0);
	imshow( "Ellipses", ellipses);

}

void draw_ellipses(vector<vector<Point> > contours, Mat ellipses, Mat img0) {
	for( int i = (int)contours.size()-1; i >= 0; i-- ) {
			const vector<Point>& r = contours[i];
			RotatedRect box = fitEllipse( r );
			ellipse( ellipses, box, Scalar(125,125,0), 2 ); //196,255,255
		}
}