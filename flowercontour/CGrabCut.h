#ifndef CGRABCUT_H
#define CGRABCUT_H
#include "opencv2/opencv.hpp"
using namespace cv;
class CGrabCut
{
public:
    CGrabCut();

	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName( const Mat& _image, const string& _winName );
    cv::Mat showImage();

    Mat mask;
    Rect rect;

    void mouseClick( int event, int x, int y, int flags, void* param );
	int nextIter();
	int getIterCount() const { return iterCount; }
private:
	void setRectInMask();
    void setLblsInMask( int flags, Point p, bool isPr );

	const string* winName;
	const Mat* image;

    Mat bgdModel, fgdModel;

	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	int iterCount;


};

#endif // CGRABCUT_H
