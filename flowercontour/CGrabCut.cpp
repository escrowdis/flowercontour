#include "CGrabCut.h"
/*
void help()
{
 cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
   "and then grabcut will attempt to segment it out.\n"
   "Call:\n"
   "./grabcut <image_name>\n"
  "\nSelect a rectangular area around the object you want to segment\n" <<
  "\nHot keys: \n"
  "\tESC - quit the program\n"
  "\tr - restore the original image\n"
  "\tn - next iteration\n"
  "\n"
  "\tleft mouse button - set rectangle\n"
  "\n"
  "\tCTRL+left mouse button - set GC_BGD pixels\n"
  "\tSHIFT+left mouse button - set CG_FGD pixels\n"
  "\n"
  "\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
  "\tSHIFT+right mouse button - set CG_PR_FGD pixels\n" << endl;
}
*/
CGrabCut::CGrabCut()
{
}

const Scalar RED = Scalar(0,0,255);
const Scalar PINK = Scalar(230,130,255);
const Scalar BLUE = Scalar(255,0,0);
const Scalar LIGHTBLUE = Scalar(255,255,160);
const Scalar GREEN = Scalar(0,255,0);

const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;

void getBinMask( const Mat& comMask, Mat& binMask )
{
	if( comMask.empty() || comMask.type()!=CV_8UC1 )
		CV_Error( CV_StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)" );
	if( binMask.empty() || binMask.rows!=comMask.rows || binMask.cols!=comMask.cols )
		binMask.create( comMask.size(), CV_8UC1 );
	binMask = comMask & 1;
}


void CGrabCut::reset()
{
	if( !mask.empty() )
		mask.setTo(Scalar::all(GC_BGD));
	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear();  prFgdPxls.clear();

	isInitialized = false;
	rectState = NOT_SET;
	lblsState = NOT_SET;
	prLblsState = NOT_SET;
	iterCount = 0;
}

void CGrabCut::setImageAndWinName( const Mat& _image, const string& _winName  )
{
	if( _image.empty() || _winName.empty() )
		return;
	image = &_image;
	winName = &_winName;
	mask.create( image->size(), CV_8UC1);
	reset();
}

cv::Mat CGrabCut::showImage()
{
	if( image->empty() || winName->empty() )
        return cv::Mat();
    cv::Mat().create(image->size(),CV_8UC3);

    Mat res;
    Mat binMask;
    //Mat resTmp;
    if( !isInitialized ){
        image->copyTo( res );
    }
	else
	{
		getBinMask( mask, binMask );
		image->copyTo( res, binMask );
	}

	vector<Point>::const_iterator it;
	for( it = bgdPxls.begin(); it != bgdPxls.end(); ++it )
		circle( res, *it, radius, BLUE, thickness );
	for( it = fgdPxls.begin(); it != fgdPxls.end(); ++it )
		circle( res, *it, radius, RED, thickness );
	for( it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it )
		circle( res, *it, radius, LIGHTBLUE, thickness );
	for( it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it )
		circle( res, *it, radius, PINK, thickness );

    if( rectState == IN_PROCESS)    // || rectState == SET
		rectangle( res, Point( rect.x, rect.y ), Point(rect.x + rect.width, rect.y + rect.height ), GREEN, 2);

    imshow( *winName, res );
    return res;
}

void CGrabCut::setRectInMask()
{
	assert( !mask.empty() );
	mask.setTo( GC_BGD );
	rect.x = max(0, rect.x);
	rect.y = max(0, rect.y);
	rect.width = min(rect.width, image->cols-rect.x);
	rect.height = min(rect.height, image->rows-rect.y);
	(mask(rect)).setTo( Scalar(GC_PR_FGD) );
}

void CGrabCut::setLblsInMask( int flags, Point p, bool isPr )
{
	vector<Point> *bpxls, *fpxls;
	uchar bvalue, fvalue;
	if( !isPr )
	{
		bpxls = &bgdPxls;
		fpxls = &fgdPxls;
		bvalue = GC_BGD;
		fvalue = GC_FGD;
	}
	else
	{
		bpxls = &prBgdPxls;
		fpxls = &prFgdPxls;
		bvalue = GC_PR_BGD;
		fvalue = GC_PR_FGD;
	}
	if( flags & BGD_KEY )
	{
		bpxls->push_back(p);
		circle( mask, p, radius, bvalue, thickness );
	}
	if( flags & FGD_KEY )
	{
		fpxls->push_back(p);
		circle( mask, p, radius, fvalue, thickness );
	}
}

void CGrabCut::mouseClick( int event, int x, int y, int flags, void* )
{
	// TODO add bad args check
	switch( event )
	{
	case CV_EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
	{
		bool isb = (flags & BGD_KEY) != 0,
				isf = (flags & FGD_KEY) != 0;
		if( rectState == NOT_SET && !isb && !isf )
		{
			rectState = IN_PROCESS;
			rect = Rect( x, y, 1, 1 );
		}
		if ( (isb || isf) && rectState == SET )
			lblsState = IN_PROCESS;
	}
		break;
	case CV_EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
	{
		bool isb = (flags & BGD_KEY) != 0,
				isf = (flags & FGD_KEY) != 0;
		if ( (isb || isf) && rectState == SET )
			prLblsState = IN_PROCESS;
	}
		break;
	case CV_EVENT_LBUTTONUP:
		if( rectState == IN_PROCESS )
		{
			rect = Rect( Point(rect.x, rect.y), Point(x,y) );
			rectState = SET;
			setRectInMask();
			assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty() );
            showImage();
		}
		if( lblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case CV_EVENT_RBUTTONUP:
		if( prLblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case CV_EVENT_MOUSEMOVE:
		if( rectState == IN_PROCESS )
		{
			rect = Rect( Point(rect.x, rect.y), Point(x,y) );
			assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty() );
			showImage();
		}
		else if( lblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), false);
			showImage();
		}
		else if( prLblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), true);
			showImage();
		}
		break;
	}
}

int CGrabCut::nextIter()
{
	if( isInitialized )
		grabCut( *image, mask, rect, bgdModel, fgdModel, 1 );
	else
	{
		if( rectState != SET )
			return iterCount;

        if( lblsState == SET || prLblsState == SET )
			grabCut( *image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK );
        else
			grabCut( *image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT );

		isInitialized = true;
	}

    iterCount++;
	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear(); prFgdPxls.clear();

	return iterCount;
}
