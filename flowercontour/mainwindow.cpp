/***************************************************************************
[OpenCV2.4.3]
***************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iostream"
#include "QMouseEvent"
#include "math.h"
#include "QDebug"
#include "QMessageBox"
#include "QTextStream"

using namespace cv;
using namespace std;

CGrabCut __GrabCut;

Mat image, image_preview, image_preview1, reset_use, src,showmask, ruler, image_out, image_out1, size_origin;
QString file, string_r, record, record_contours, record_contours_use, record_mc, f_pri[5], s_pri[5];
//src: source; feature-mask: contours(area); contours-drawing: contours
QPoint firstpix, lastpix;

//ruler param
int ed_size, ed_iter, thresh_min, thresh_max;

//label_9 window offset
const int offset_x = 40;
const int offset_y = 52;
const double pi = 3.14159265359;
const int ldmks_radius = 2;
double zoom_ratio=0.0; //zoom_ratio > 1

double pixel, square, square_d[5], square_t[5], square_t1[5], square_t2[5], square_t3[5], square_t4[5];
int num = 0;
int manu = 0;

//landmark
//front: 1->25->19->13->7
//side: 1->7->8->9->15
int highside;   //1:R; 0:L
int interval[6];
QPoint front_primary[5], side_primary[5];
vector<Point> pri_landmark_pts, closept, landmark_pts_x, landmark_pts_y, cali_landmark_x[5], cali_landmark_y[5], pause_x[1], pause_y[1];
vector<Point> line_pri_x, line_pri_y, line_close_x, line_close_y;
vector<Point> line_x, line_y;
vector<Point> cvt_onetwo_x_front[7], cvt_onetwo_y_front[7], cvt_onetwo_x_side[7], cvt_onetwo_y_side[7], cali_x_side[2], cali_y_side[2];
int cali_landmark_check = 0;

int pri = 0;
float bx, by, k, xx, yy, init_x, init_y;
int line_pri[5], line_close[5];     //the gap between contour and primary, close pt
int curve[5] = {0};         //found the curve, can do the reflection
float dif_of_square_pri, dif_of_square_close;
float overlap_close_d[5]={0};  //distance between close and contour
float overlap_pri_d[5]={0};   //distance between primary and contour
int overlap_contours[5]={0};    //which contour number
int overlap_close_pt[5]={0};    //which closest point
int overlap_pri_pt[5]={0};      //which primary point

//color
const Scalar WHITE = Scalar(255, 255, 255);
const Scalar BLACK = Scalar(0, 0, 0);
const Scalar GREEN = Scalar(0, 200, 0);
const Scalar BLUE = Scalar(50, 50, 255);
const Scalar RED = Scalar(255, 0, 0);
const Scalar PURPLE = Scalar(20, 20, 255);

//overlap drawing
int check_pt_setting = 0;
Point temp_points;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_3->setEnabled(false);    ui->pushButton_4->setEnabled(false);
    ui->pushButton_6->setEnabled(false);    ui->pushButton_9->setEnabled(false);
    ui->pushButton_16->setEnabled(false);    ui->pushButton_17->setEnabled(false);
    ui->pushButton_14->setEnabled(true);
    ui->line_5->setVisible(false);    ui->line_6->setVisible(false);
    ui->line_7->setVisible(false);    ui->line_8->setVisible(false);
    ui->label_9->setGeometry(40, 40, 400, 300);       //reset Label
    ui->label_10->setGeometry(40, 390, 400, 300);
    ui->label_11->setGeometry(470, 390, 400, 300);
    ui->label_62->setText("Dorsal");
}

MainWindow::~MainWindow()
{
    delete ui;
}

//exit
void MainWindow::on_pushButton_clicked()
{
    destroyAllWindows();
    close();
}

void on_mouseGrab( int event, int x, int y, int flags, void* param )
{
    __GrabCut.mouseClick( event, x, y, flags, param );
}

//load image
void MainWindow::on_pushButton_2_clicked()
{
    file = QFileDialog::getOpenFileName();      //load the file & resize, show
    if(file.isEmpty())
        return;
    reset();
    //qDebug()<<file;
    string_r.sprintf("%s", file.toStdString());
    ui->label_46->setText(string_r);
    image = imread(file.toStdString());
    //show real size grabcut
//    image.copyTo(size_origin);
//    imshow("origin", size_origin);

    //[record]
//    record = "\t\tPerimeter\tArea\tP/A ratio\t[min circle]\tRadius\tArea\t[Rect]\tWidth\tHeight\tW/H ratio\tArea\t[min rect]\tArea\t\t[Landmark]\tRadius\tArea\tCircle ratio\t\tPrimary\n";
//    qDebug()<<file;
    //get flower name
    file = file.section('/', -1, -1);
    file = file.section('.', 0, 0);
//    qDebug()<<"file"<<file;
    record.append(file);
    record.append("_x\t");
    record_contours.append(file);
    record_contours.append("_x\t");
    record_contours_use.append(file);
    record_contours_use.append("_x\t");

    //record_mc.append(file);
    //record_mc.append("\t");

    //imshow("image", image);
    //img_circle = cvLoadImage(file.toStdString().c_str(), 1);

    zoom.width = image.cols;
    zoom.height = image.rows;
    size.height=ui->label_9->height();
    size.width=(size.height)*(image.cols)/(image.rows);
    zoom_ratio = (image.rows+.0)/(size.height+.0);

    cv::resize(image,image,size);
    image.copyTo(reset_use);
    cvtColor(image,image,CV_BGR2RGB);
    image.copyTo(src);
    QImage img = QImage((uchar*)(image.data),image.cols,image.rows,QImage::Format_RGB888);
    ui->label_9->setPixmap(QPixmap::fromImage(img));
    cvtColor(image,image,CV_RGB2BGR);
    ui->pushButton_9->setEnabled(true);
    ui->pushButton_16->setEnabled(true);    ui->pushButton_17->setEnabled(true);
}

void MainWindow::on_pushButton_16_clicked()
{
    //grabCut
    const cv::string winName = "origin";
    cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE);
    cvMoveWindow(winName.c_str(), 200, 200);
    cvSetMouseCallback( winName.c_str(), on_mouseGrab, 0);
    __GrabCut.setImageAndWinName(size_origin, winName);
    __GrabCut.showImage();
    int kk=1;
    while(kk==1){
        int c = cvWaitKey(0);
        switch(char(c)){
        case 'q':
            kk=0;
            image_preview = __GrabCut.showImage();
            cvDestroyWindow(winName.c_str());
            break;
        case 'r':
            __GrabCut.reset();
            __GrabCut.showImage();
            break;
        case 'n':
            int iterCount = __GrabCut.getIterCount();
            int newIterCount = __GrabCut.nextIter();
            if( newIterCount > iterCount ){
                __GrabCut.showImage();
            }
            break;
        }
    }
    imshow("origin", size_origin);
}

//ruler scale estimation
void MainWindow::ruler_estimation()
{
    src.copyTo(ruler);
    cvtColor(ruler, ruler, CV_BGR2GRAY);
    Rect rect_cut = __GrabCut.rect;
    Point* Points = new cv::Point[4];
    Points[0] = Point(rect_cut.x, rect_cut.y);
    Points[1] = Point(rect_cut.x, rect_cut.y+rect_cut.height);
    Points[2] = Point(rect_cut.x+rect_cut.width, rect_cut.y+rect_cut.height);
    Points[3] = Point(rect_cut.x+rect_cut.width, rect_cut.y);
    fillConvexPoly(ruler, Points, 4, BLACK);
    delete []Points;

    //manually select interval scale - RightMouse
    if(lastpix.isNull() == false)
    {
        square = abs(pow(lastpix.x()-firstpix.x()+.0,2) + pow(lastpix.y()-firstpix.y()+.0,2));   //pixel calibration - firstpixel, lastpixel
        pixel = sqrt(square);
    }
    //automatic select interval scale
    else
    {
        cv::resize(ruler, ruler, zoom);
        if(ui->radioButton_3->isChecked())
            cv::transpose(ruler, ruler);//2//
        if(ruler.empty())
            return;
        //erode & dilate - find the ruler
        Mat ruler1;
        ed_size = ui->spinBox->value();
        ed_iter = ui->spinBox_2->value();
        thresh_min = ui->horizontalSlider_4->value();
        thresh_max = ui->horizontalSlider_5->value();
        Mat element = getStructuringElement(MORPH_RECT, Size(ed_size,ed_size));
    //        erode(ruler, ruler1, element, Point(-1,-1), 2);
        dilate(ruler, ruler1, element, Point(-1,-1), ed_iter);
        absdiff(ruler, ruler1, ruler);
        //imshow("mew_t", ruler);
        threshold(ruler, ruler, thresh_min, thresh_max, THRESH_BINARY);
    //    imshow("mew", ruler);
        //display
        Mat ruler_dis;
        ruler.copyTo(ruler_dis);
    //    cv::resize(ruler, ruler, Size(ui->label_10->width(), ui->label_10->height()));
        cv::resize(ruler_dis, ruler_dis, Size(ui->label_10->width(), ui->label_10->height()));
        QImage img = QImage((uchar*)(ruler_dis.data),ruler_dis.cols,ruler_dis.rows,QImage::Format_Indexed8);
    //    QImage img = QImage((uchar*)(ruler.data),ruler.cols,ruler.rows,QImage::Format_Indexed8);
        ui->label_10->setPixmap(QPixmap::fromImage(img));
    //    imshow("mew", ruler_dis);
    //    QMessageBox::information(0,0,"0");

//        cv::imshow("ya", ruler);
        int rows = ruler.rows;
        int cols = ruler.cols;
        //histogram - find interval
        int array[100000]={0};
        uchar *gray = ruler.ptr<uchar>(0);
        for(int i=0; i<rows; i++)
        {
            for(int j=0; j<cols; j++)
            {
                int a = j+i*cols;
                if(gray[a] > 128)
                    array[i]++;
            }
        }

        int mean = 0;
        for(int i=0; i< rows; i++)
        {
            mean = mean + array[i];
        }
        mean = mean/rows;
        for(int i = 0; i<cols; i++)
        {
            if(array[i]<= 4*mean)
            array[i] = array[i]*array[i];
        }
        mean = 0;
        for(int i=0; i< rows; i++)
        {
            mean = mean + array[i];
        }
        mean = mean/rows;

        int big=0;
        for(int i=0; i<rows; i++)
        {
            if(array[i]>big)
            {
                big = array[i];

            }
        }

        //draw hist
        Mat image_histo;
        int h = ui->label_66->height();
        image_histo.create(h,rows,CV_8UC3);
        image_histo.setTo(0);
        for(int i=0; i<rows; i++)
        {
            int b = (big-array[i])*h/big;
            line(image_histo,Point(i,h),Point(i,b),CV_RGB(255,255,255));
        }

        //histogram to find interval (spectrum)
        int x = 0;
        int interval[50] = {0};
        for(int i = 1; i<rows-1; i++)
        {
            if(array[i-1] > mean && array[i] < mean) //&& (array[i-1]/array[i]) >= 3 有0 會error
            {
                interval[x] = i;
                //qDebug()<<interval[x];
                x++;
            }
        }

        for(int i =1; i<=50; i++)
        {
            interval[i-1] = interval[i] - interval[i-1];
            //qDebug()<<interval[i-1];
        }

        int interval_end = sizeof(interval)/sizeof(int);
        sort(interval, interval+interval_end);

        /*for(int i =0; i<50; i++)
            qDebug()<<interval[i];*/

        double pxl = 0.0;
        for(int i=5; i<45; i++)
        {
            pxl = pxl + interval[i];
        }
        pixel = (pxl*10.0)/(zoom_ratio*40.0);
        Mat image_histo_dis;
        image_histo.copyTo(image_histo_dis);
        cv::resize(image_histo_dis, image_histo_dis, Size(ui->label_66->width(), ui->label_66->height()));
        QImage img1 = QImage((uchar*)(image_histo_dis.data),image_histo_dis.cols,image_histo_dis.rows,QImage::Format_RGB888);
        ui->label_66->setPixmap(QPixmap::fromImage(img1));
        //qDebug()<<zoom_ratio;
//        qDebug()<<pixel;
    }

//    ui->label_16->setText(string_r.sprintf("%.4f", pixel));
}

//grabcut
void MainWindow::on_pushButton_9_clicked()
{
    //grabCut
    reset_use.copyTo(image);
    const cv::string winName = "image";
    cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE);
    cvMoveWindow(winName.c_str(), 200, 200);
    cvSetMouseCallback( winName.c_str(), on_mouseGrab, 0);
    __GrabCut.setImageAndWinName(image, winName);
    __GrabCut.showImage();
    int kk=1;
    while(kk==1){
        int c = cvWaitKey(0);
        switch(char(c)){
        case 'q':
            kk=0;
            image_preview = __GrabCut.showImage();
            cvDestroyWindow(winName.c_str());
            break;
        case 'r':
            __GrabCut.reset();
            __GrabCut.showImage();
            break;
        case 'n':
            int iterCount = __GrabCut.getIterCount();
            int newIterCount = __GrabCut.nextIter();
            if( newIterCount > iterCount ){
                __GrabCut.showImage();
            }
            break;
        }
    }
    cvtColor(image_preview, image_preview, CV_BGR2RGB);
    QImage img = QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888);
    ui->label_9->setPixmap(QPixmap::fromImage(img));
    cvtColor(image, image, CV_BGR2RGB);
    img = QImage((uchar*)(image.data),image.cols,image.rows,QImage::Format_RGB888);
    ui->label_10->setPixmap(QPixmap::fromImage(img));
    image_preview.copyTo(mask);
    ui->pushButton_14->setEnabled(true);
    ruler_estimation();
}

//primary landmark selection, ruler scale, outermost layer contour, find closest pt to pri.
void MainWindow::on_pushButton_4_clicked()
{
    //findcontours  [outermost layer]
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    //Mat mask = __GrabCut.mask;
    //compare(mask, GC_PR_FGD, mask, CMP_EQ);
    cvtColor(mask, mask, CV_RGB2GRAY);
    threshold(mask, mask, 1,255, CV_THRESH_BINARY);
    mask.convertTo(mask, CV_8UC1);
    findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<Moments> mu(contours.size());                               //mass center counting
    for(int i = 0; i < contours.size(); i++)
        mu[i] = moments( contours[i], false);

    vector<Point2f> mc( contours.size());
    for(int i = 0; i < contours.size(); i++){
        mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
    }

    //sequence for displaying the right contours
    for( int i = contours.size()-1; i > 0 ; i-- ){
        if(contourArea(contours[i])>contourArea(contours[i-1])){
            contours[i-1] = contours[i];
            mc[i-1] = mc[i];
        }
    }

    //record center of mass
    circle(image_preview, mc[0], 2, Scalar(255, 255, 255), -1);
    string_r.sprintf("(%d, %d)",int(mc[0].x), int(mc[0].y));
    //record_mc.append(string_r);
    //record_mc.append("\n\n");
    ui->label_60->setText(string_r);
    drawContours(image_preview, contours, 0, WHITE, 2, 2, hierarchy, 0);

    double area_cm_non = 0.0;
    area_cm_non = (contourArea(contours[0])+.0)/(pixel*pixel);
    string_r.sprintf("%.4f", area_cm_non);    record.append(string_r);    record.append("\t");    ui->label_5->setText(string_r);

    QString string_contours;
    for(int i=0; i<contours[0].size(); i++){
        string_contours.sprintf("%d", contours[0][i].x);    record_contours.append(string_contours + "\t");
    }
    string_contours.clear();
    record_contours.append("\n" + file + "_y\t");
    for(int i=0; i<contours[0].size(); i++){
        string_contours.sprintf("%d", contours[0][i].y);    record_contours.append(string_contours + "\t");
    }
    record_contours.append("\n");
    string_contours.clear();

    //find closest pt
    int cali[5];
    for(int i=0; i<5; i++)
        square_t[i] = 5000000;

    for(int k=0; k<5; k++){
        if(ui->radioButton->isChecked()){
            for(size_t j=0; j<contours[0].size(); j++)
            {
                //有offset
                square_d[k] = abs(pow(front_primary[k].x()-contours[0][j].x-offset_x+.0,2) + pow(front_primary[k].y()-contours[0][j].y-offset_y+.0,2));
                if(square_d[k] < square_t[k]){
                    square_t[k] = square_d[k];
                    cali[k] = j;
                }
            }
        }
        else if(ui->radioButton_2->isChecked()){
            for(size_t j=0; j<contours[0].size(); j++){
                //有offset
                square_d[k] = abs(pow(side_primary[k].x()-contours[0][j].x-offset_x+.0,2) + pow(side_primary[k].y()-contours[0][j].y-offset_y+.0,2));
                if(square_d[k] < square_t[k]){
                    square_t[k] = square_d[k];
                    cali[k] = j;
                }
            }
        }
    }
    for(int i=0; i<5; i++)
        closept.push_back(Point(contours[0][cali[i]].x, contours[0][cali[i]].y));

    QImage img = QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888);
    ui->label_9->setPixmap(QPixmap::fromImage(img));
    src.copyTo(image_preview);
    img = QImage((uchar*)(image_preview1.data),image_preview1.cols,image_preview1.rows,QImage::Format_RGB888);
    ui->label_10->setPixmap(QPixmap::fromImage(img));
    if(ui->radioButton_2->isChecked())
        ui->pushButton_3->setEnabled(true);
}

//Only catch Size information
void MainWindow::on_pushButton_17_clicked()
{
    //findcontours  [outermost layer]
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    //compare(mask, GC_PR_FGD, mask, CMP_EQ);
    cvtColor(mask, mask, CV_RGB2GRAY);
    threshold(mask, mask, 1,255, CV_THRESH_BINARY);
    mask.convertTo(mask, CV_8UC1);
    findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<Moments> mu(contours.size());                               //mass center counting
    for(int i = 0; i < contours.size(); i++)
        mu[i] = moments( contours[i], false);

    vector<Point2f> mc( contours.size());
    for(int i = 0; i < contours.size(); i++){
        mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
    }

    //sequence for displaying the right contours
    for( int i = contours.size()-1; i > 0 ; i-- ){
        if(contourArea(contours[i])>contourArea(contours[i-1])){
            contours[i-1] = contours[i];
            mc[i-1] = mc[i];
        }
    }

    QString record_size;
    double area_cm_non = (contourArea(contours[0])+.0)/(pixel*pixel);
    double contour_l = contours[0].size()/pixel;
    record_size.append(file + "\t" + QString::number(area_cm_non) + "\t" + QString::number(contour_l) + "\n");

    QFile outfile_size;
    outfile_size.setFileName("record_OnlySize.txt");
    outfile_size.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&outfile_size);
    out << record_size;
    outfile_size.close();
    qDebug()<<pixel;
    drawContours(image_preview, contours, 0, WHITE, 2, 2, hierarchy, 0);
    QImage img = QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888);
    ui->label_9->setPixmap(QPixmap::fromImage(img));
}

//processing - Contour detection, Ldmks identification
void MainWindow::on_pushButton_3_clicked()
{
    //find contours
    Mat black = Mat::zeros(image.size()+Size(200,200), CV_8UC3);
    vector<vector<Point>> contours;
    vector<vector<Point>> contours_use;
    vector<Vec4i> hierarchy;

    findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    //qDebug()<<contours_use[0].size();
    vector<Moments> mu(contours.size() );                               //mass center counting
    for(unsigned int i = 0; i < contours.size(); i++)
        mu[i] = moments( contours[i], false);

    //center of mass
    vector<Point2f> mc( contours.size() );
    for(unsigned int i = 0; i < contours.size(); i++){
        mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

    //sequence for displaying the right contours
    for( int i = contours.size()-1; i > 0 ; i-- ){
        if(contourArea(contours[i])>contourArea(contours[i-1])){
            contours[i-1] = contours[i];
            mc[i-1] = mc[i];
        }
    }
    //drawContours(reset_use, contours, 0, WHITE, 2, 2, hierarchy, 0);
    //imshow("re", reset_use);

    //calibration for initial pts of contours!!
    int init_x, init_y;
    if(ui->radioButton->isChecked()){
        init_x = contours[0][0].x;
        init_y = contours[0][0].y;
    }
    else if(ui->radioButton_2->isChecked()){
        init_x = side_primary[1].x()-offset_x;
        init_y = side_primary[1].y()-offset_y;
    }
    circle(black, Point(init_x, init_y), 1, WHITE, -1);
    //circle(image, Point(init_x, init_y), 1,RED, 5);
    cvtColor(black, black, CV_BGR2GRAY);
    findContours(black, contours_use, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

    ////
    //find closest pt to pri. ldmks
    int i =0;
    int cali[5];
    int connect[2];
    for(int i=0; i<5; i++){
        square_t[i] = 5000000;        square_t1[i] = 5000000;
        square_t2[i] = 5000000;        square_t3[i] = 5000;
        square_t4[i] = 5000;
    }

    for(int k=0; k<5; k++){
        for(size_t j=0; j<contours[0].size(); j++){
            //有offset
            square_d[k] = abs(pow(front_primary[k].x()-contours[0][j].x-offset_x+.0,2) + pow(front_primary[k].y()-contours[0][j].y-offset_y+.0,2));
            if(square_d[k] < square_t[k]){
                square_t[k] = square_d[k];
                cali[k] = j;
            }
            else if(ui->radioButton_2->isChecked()){
                //有offset
                square_d[k] = abs(pow(side_primary[k].x()-contours[0][j].x-offset_x+.0,2) + pow(side_primary[k].y()-contours[0][j].y-offset_y+.0,2));
                if(square_d[k] < square_t[k]){
                    square_t[k] = square_d[k];
                    cali[k] = j;
                }
            }
        }
    }

    //insert overlapping part
    if(ui->radioButton_2->isChecked()){
        //Dorsal
        if(cali_landmark_x[0].size()>0){
            for(size_t j=0; j<contours[0].size(); j++){
                if(cali_landmark_x[0].size()>0)
                {
                    square_d[0] = abs(pow(cali_landmark_x[0][(cali_landmark_x[0].size())-1].x-contours[0][j].x+.0,2) +
                                      pow(cali_landmark_y[0][(cali_landmark_y[0].size())-1].x-contours[0][j].y+.0,2));
                    if(square_d[0] < square_t3[0])
                    {
                        square_t3[0] = square_d[0];
                        connect[0] = j;
                    }
                }
            }
        }
        //Ventral
        if(cali_landmark_x[1].size()>0){
            for(size_t j=0; j<contours[0].size(); j++){
                if(cali_landmark_x[1].size()>0)
                {
                    square_d[1] = abs(pow(cali_landmark_x[1][0].x-contours[0][j].x+.0,2) +
                                      pow(cali_landmark_y[1][0].x-contours[0][j].y+.0,2));
                    if(square_d[1] < square_t4[1])
                    {
                        square_t4[1] = square_d[1];
                        connect[1] = j;
                    }
                }
            }
        }
    }

    //distinguish highest pt - start from where
    if(ui->radioButton->isChecked()){
        if(contours[0][0].x+offset_x > front_primary[0].x())    //right
            highside = 1;
        else if(contours[0][0].x+offset_x < front_primary[0].x())   //Left
            highside = 0;
    }

    ////
    //contours->contours_use
    //Front - Highest
    if(ui->radioButton->isChecked()){
        //Right
        if(highside == 1){
            for(int k=0; k<5; k++){
                if(k==0){
                    for(size_t j=0; j<cali[0]; j++){
                        //要先2->1 1->2
                        cvt_onetwo_x_front[0].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[0].push_back(Point(contours[0][j].y));
                        contours_use[0].push_back(Point(cvt_onetwo_x_front[0][j].x, cvt_onetwo_y_front[0][j].x));
                    }
                    for(size_t j=0; j<cali_landmark_x[0].size(); j++){
                        contours_use[0].push_back(Point(cali_landmark_x[0][j].x, cali_landmark_y[0][j].x));
                    }
                }
                else{
                    for(size_t j=cali[k-1]; j<cali[k]; j++){
                        cvt_onetwo_x_front[k].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[k].push_back(Point(contours[0][j].y));
                    }
                    for(size_t j=0; j<cvt_onetwo_x_front[k].size(); j++)
                            contours_use[0].push_back(Point(cvt_onetwo_x_front[k][j].x, cvt_onetwo_y_front[k][j].x));
                    if(cali_landmark_x[k].size() > 0)
                        for(size_t j=0; j<cali_landmark_x[k].size(); j++)
                            contours_use[0].push_back(Point(cali_landmark_x[k][j].x, cali_landmark_y[k][j].x));
                }
                if(k==4){
                    for(size_t j=cali[k]; j<contours[0].size(); j++){
                        cvt_onetwo_x_front[k+1].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[k+1].push_back(Point(contours[0][j].y));
                    }
                    for(size_t j=0; j<cvt_onetwo_x_front[k+1].size(); j++)
                            contours_use[0].push_back(Point(cvt_onetwo_x_front[k+1][j].x, cvt_onetwo_y_front[k+1][j].x));
                }
            }
        }
        //Left
        else if(highside == 0){
            for(int k=1; k<5; k++){
                if(k==1){
                    for(size_t j=0; j<cali[1]; j++){
                        cvt_onetwo_x_front[0].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[0].push_back(Point(contours[0][j].y));
                        contours_use[0].push_back(Point(cvt_onetwo_x_front[0][j].x, cvt_onetwo_y_front[0][j].x));
                    }
                    for(size_t j=0; j<cali_landmark_x[1].size(); j++){
                        contours_use[0].push_back(Point(cali_landmark_x[1][j].x, cali_landmark_y[1][j].x));
                    }
                }
                else{
                    for(size_t j=cali[k-1]; j<cali[k]; j++){
                        cvt_onetwo_x_front[k].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[k].push_back(Point(contours[0][j].y));
                    }
                    for(size_t j=0; j<cvt_onetwo_x_front[k].size(); j++)
                            contours_use[0].push_back(Point(cvt_onetwo_x_front[k][j].x, cvt_onetwo_y_front[k][j].x));
                    if(cali_landmark_x[k].size() > 0)
                            for(size_t j=0; j<cali_landmark_x[k].size(); j++)
                                contours_use[0].push_back(Point(cali_landmark_x[k][j].x, cali_landmark_y[k][j].x));
                }
                if(k==4){
                    for(size_t j=cali[4]; j<cali[0]; j++){
                        cvt_onetwo_x_front[k+1].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[k+1].push_back(Point(contours[0][j].y));
                    }
                    for(size_t j=0; j<cvt_onetwo_x_front[k+1].size(); j++)
                            contours_use[0].push_back(Point(cvt_onetwo_x_front[k+1][j].x, cvt_onetwo_y_front[k+1][j].x));
                    for(size_t j=0; j<cali_landmark_x[0].size(); j++)
                        contours_use[0].push_back(Point(cali_landmark_x[0][j].x, cali_landmark_y[0][j].x));
                    for(size_t j=cali[0]; j<contours[0].size(); j++){
                        cvt_onetwo_x_front[k+2].push_back(Point(contours[0][j].x));
                        cvt_onetwo_y_front[k+2].push_back(Point(contours[0][j].y));
                    }
                    for(size_t j=0; j<cvt_onetwo_x_front[k+2].size(); j++)
                            contours_use[0].push_back(Point(cvt_onetwo_x_front[k+2][j].x, cvt_onetwo_y_front[k+2][j].x));
                }
            }
        }
    }
    //Side
    else if(ui->radioButton_2->isChecked()){
        //Dorsal
        /*cvt_onetwo_x_side[0].push_back(Point(side_primary[1].x()-offset_x));
        cvt_onetwo_y_side[0].push_back(Point(side_primary[1].y()-offset_y));*/
        if(cali_landmark_x[0].size()>0){
            for(size_t j=0; j< cali_landmark_x[0].size(); j++){
                cvt_onetwo_x_side[0].push_back(Point(cali_landmark_x[0][j].x));
                cvt_onetwo_y_side[0].push_back(Point(cali_landmark_y[0][j].x));
            }
            if(connect[0]<=cali[0]){
                for(size_t j=connect[0]; j<cali[0]; j++){
                    cvt_onetwo_x_side[0].push_back(Point(contours[0][j].x));
                    cvt_onetwo_y_side[0].push_back(Point(contours[0][j].y));
                }
            }
            else{
                for(size_t j=connect[0]; j<contours[0].size(); j++){
                    cvt_onetwo_x_side[0].push_back(Point(contours[0][j].x));
                    cvt_onetwo_y_side[0].push_back(Point(contours[0][j].y));
                }
                for(size_t j=0; j<cali[0]; j++){
                    cvt_onetwo_x_side[0].push_back(Point(contours[0][j].x));
                    cvt_onetwo_y_side[0].push_back(Point(contours[0][j].y));
                }
            }
        }
        else{
            if(cali[1]<=cali[0]){
                for(size_t j=cali[1]; j<cali[0]; j++){
                    cvt_onetwo_x_side[0].push_back(Point(contours[0][j].x));
                    cvt_onetwo_y_side[0].push_back(Point(contours[0][j].y));
                }
            }
            else{////contours get through pt1
                for(size_t j=cali[1]; j<contours[0].size(); j++){
                    cvt_onetwo_x_side[0].push_back(Point(contours[0][j].x));
                    cvt_onetwo_y_side[0].push_back(Point(contours[0][j].y));
                }
                for(size_t j=0; j<cali[0]; j++){
                    cvt_onetwo_x_side[0].push_back(Point(contours[0][j].x));
                    cvt_onetwo_y_side[0].push_back(Point(contours[0][j].y));
                }
            }
        }

        //Ventral
        if(cali_landmark_x[1].size()>0){
            //qDebug()<<1;
            for(size_t j=cali[4]; j<=connect[1]; j++){
                cvt_onetwo_x_side[1].push_back(Point(contours[0][j].x));
                cvt_onetwo_y_side[1].push_back(Point(contours[0][j].y));
            }
            for(size_t j=0; j< cali_landmark_x[1].size(); j++){
                cvt_onetwo_x_side[1].push_back(Point(cali_landmark_x[1][j].x));
                cvt_onetwo_y_side[1].push_back(Point(cali_landmark_y[1][j].x));
                /*qDebug()<<cali_landmark_x[1][j].x;
                qDebug()<<cali_landmark_y[1][j].x;*/
            }
        }
        else
            for(size_t j=cali[4]; j<cali[3]; j++){
                cvt_onetwo_x_side[1].push_back(Point(contours[0][j].x));
                cvt_onetwo_y_side[1].push_back(Point(contours[0][j].y));
            }

        cvt_onetwo_x_side[1].push_back(Point(side_primary[2].x()-offset_x));
        cvt_onetwo_y_side[1].push_back(Point(side_primary[2].y()-offset_y));

        for(size_t j=0; j<cvt_onetwo_x_side[0].size(); j++)
                contours_use[0].push_back(Point(cvt_onetwo_x_side[0][j].x, cvt_onetwo_y_side[0][j].x));
        for(size_t j=0; j<cvt_onetwo_x_side[1].size(); j++)
                contours_use[0].push_back(Point(cvt_onetwo_x_side[1][j].x, cvt_onetwo_y_side[1][j].x));

        /*for(int i=0; i<5; i++)
            qDebug()<<cali[i];*/
    }

    /*contours_cali.push_back(contours[i][j]);
    int x = contours[i][j].x;
    int y = contours[i][j].y;
    qDebug()<<x;
    qDebug()<<y;*/

    /*for(int j=0; j<contours_use[0].size(); j++)                   //draw test
    {
        circle(image, contours_use[0][j],1, Scalar(100,100,200), 5);
        qDebug()<<contours_use[0][j].x;
    }*/

    //draw contours & mass center
    src.copyTo(showmask);
    if(ui->radioButton->isChecked())    ////
        drawContours( showmask, contours_use, i, WHITE, 2, 2, hierarchy, 0);
    else if(ui->radioButton_2->isChecked())
        drawContours( showmask, contours_use, i, WHITE, 2, 2, hierarchy, 0);

    QString string_contours;
    for(int i=0; i<contours_use[0].size(); i++){
        string_contours.sprintf("%d", contours_use[0][i].x);    record_contours_use.append(string_contours + "\t");
    }
    string_contours.clear();
    record_contours_use.append("\n" + file + "_y\t");
    for(int i=0; i<contours_use[0].size(); i++){
        string_contours.sprintf("%d", contours_use[0][i].y);    record_contours_use.append(string_contours + "\t");
    }
    record_contours_use.append("\n");
    string_contours.clear();

    //Draw Min radius circle & primary center
    Point2f min_circle_center, pri_circle_center, rect_pt[4];
    float radius, pri_radius;
    minEnclosingCircle(contours[i], min_circle_center, radius);
    circle(showmask, min_circle_center, int(radius), BLUE, 2);

    if(pri_landmark_pts.size()>0){
        minEnclosingCircle(Mat(pri_landmark_pts), pri_circle_center, pri_radius);
        pri_circle_center.x = pri_circle_center.x - offset_x;
        pri_circle_center.y = pri_circle_center.y - offset_y;
        circle(showmask, pri_circle_center, int(pri_radius), GREEN, 2);
    }

    //circle(showmask, min_circle_center, 3, BLUE, -1, 8, 0 );

    /*if(ui->radioButton->isChecked())                                                              //draw primary point
    {
        for(int i = 0; i<30; i++)
            circle(showmask, Point(front_primary[i].x()-offset_x, front_primary[i].y()-offset_y), 3, RED, -1, 8, 0);
    }
    else if(ui->radioButton_2->isChecked())
        for(int i = 0; i<15; i++)
            circle(showmask, Point(side_primary[i].x()-offset_x, side_primary[i].y()-offset_y), 3, RED, -1, 8, 0);*/

    RotatedRect min_rect;
    double min_rect_area, rect_w, rect_h;
    if(ui->radioButton->isChecked())        //front
    {
        min_rect = minAreaRect(contours[0]);                                                    //draw Min rect
        min_rect.points(rect_pt);
        for(i=0; i<4; i++)
            line(showmask, rect_pt[i], rect_pt[(i+1)%4], PURPLE, 2);
        min_rect_area = sqrt(abs(pow((rect_pt[0].x-rect_pt[1].x), 2)+pow((rect_pt[0].y-rect_pt[1].y), 2)) *
                                    abs(pow((rect_pt[2].x-rect_pt[1].x), 2)+pow((rect_pt[2].y-rect_pt[1].y), 2)));
//        rectangle(showmask, boundingRect(contours_use[i]), PURPLE, 2);                  //draw rect
//        rect_w = boundingRect(contours[i]).width;
//        rect_h = boundingRect(contours[i]).height;
    }
//    else if(ui->radioButton_2->isChecked())     //side
//    {
//       min_rect = minAreaRect(contours[0]);                                                    //draw Min rect
//       min_rect.points(rect_pt);
//       for(i=0; i<4; i++)
//           line(showmask, rect_pt[i], rect_pt[(i+1)%4], PURPLE, 2);
//       min_rect_area = sqrt(abs(pow((rect_pt[0].x-rect_pt[1].x), 2)+pow((rect_pt[0].y-rect_pt[1].y), 2)) *
//                                   abs(pow((rect_pt[2].x-rect_pt[1].x), 2)+pow((rect_pt[2].y-rect_pt[1].y), 2)));
////       rectangle(showmask, boundingRect(contours_use[i]), PURPLE, 2);                  //draw rect
////       rect_w = boundingRect(contours[i]).width;
////       rect_h = boundingRect(contours[i]).height;
//    }


    //find landmarks
    //Front - from landmark[0] counterclockwise
    if(ui->radioButton->isChecked()){
        if(highside == 1){
            for(int k=0; k<5; k++){
                for(size_t j=0; j<contours_use[0].size(); j++){
                    square_d[k] = abs(pow(front_primary[k].x()-offset_x-contours_use[0][j].x+.0,2) +
                                      pow(front_primary[k].y()-offset_y-contours_use[0][j].y+.0,2));
                    if(square_d[k] < square_t1[k]){
                        square_t1[k] = square_d[k];
                        cali[k] = j;
                    }
                }
            }
            interval[5] = (cali[0]+(contours_use[0].size()-cali[4]))/6;
            for(int i=0; i<5; i++)
                interval[i]= (cali[i+1]-cali[i])/6;

            for(int k=0; k<4;k++){
                landmark_pts_x.push_back(Point(pri_landmark_pts[k].x-offset_x));
                landmark_pts_y.push_back(Point(pri_landmark_pts[k].y-offset_y));
                for(int i=1; i<6; i++){
                    landmark_pts_x.push_back(Point(contours_use[0][cali[k]+interval[k]*i].x));
                    landmark_pts_y.push_back(Point(contours_use[0][cali[k]+interval[k]*i].y));
                }
            }
            for(int i=0; i<6; i++){
                if( cali[4]+interval[0]*i < contours_use[0].size()){
                    landmark_pts_x.push_back(Point(contours_use[0][cali[4]+interval[5]*i].x));
                    landmark_pts_y.push_back(Point(contours_use[0][cali[4]+interval[5]*i].y));
                }
                else{
                    landmark_pts_x.push_back(Point(contours_use[0][cali[0]-interval[5]*(6-i)].x));
                    landmark_pts_y.push_back(Point(contours_use[0][cali[0]-interval[5]*(6-i)].y));
                }
            }
        }
        else if(highside == 0){
            for(int k=0; k<5; k++){
                for(size_t j=0; j<contours_use[0].size(); j++){
                    square_d[k] = abs(pow(front_primary[k].x()-offset_x-contours_use[0][j].x+.0,2) +
                                      pow(front_primary[k].y()-offset_y-contours_use[0][j].y+.0,2));
                    if(square_d[k] < square_t1[k]){
                        square_t1[k] = square_d[k];
                        cali[k] = j;
                    }
                }
            }

            interval[0] = ((contours_use[0].size() - cali[0])+cali[1])/6;
            for(int i=1; i<4; i++)
                interval[i]= (cali[i+1]-cali[i])/6;
            interval[4] = (cali[0]-cali[4])/6;

            for(int k=1; k<5;k++){
                pause_x[0].push_back(Point(pri_landmark_pts[k].x-offset_x));
                pause_y[0].push_back(Point(pri_landmark_pts[k].y-offset_y));
                for(int i=1; i<6; i++){
                    pause_x[0].push_back(Point(contours_use[0][cali[k]+interval[k]*i].x));
                    pause_y[0].push_back(Point(contours_use[0][cali[k]+interval[k]*i].y));
                }
                if(k==4){
                    pause_x[0].push_back(Point(pri_landmark_pts[0].x-offset_x));
                    pause_y[0].push_back(Point(pri_landmark_pts[0].y-offset_y));
                }
            }
            for(int i=1; i<6; i++){
                if( cali[0]+interval[0]*i < contours_use[0].size()){
                    pause_x[0].push_back(Point(contours_use[0][cali[0]+interval[0]*i].x));
                    pause_y[0].push_back(Point(contours_use[0][cali[0]+interval[0]*i].y));
                }
                else{
                    pause_x[0].push_back(Point(contours_use[0][cali[1]-interval[0]*(6-i)].x));
                    pause_y[0].push_back(Point(contours_use[0][cali[1]-interval[0]*(6-i)].y));
                }
            }
            for(int i = pause_x[0].size()-6; i< pause_x[0].size(); i++){
                landmark_pts_x.push_back(Point(pause_x[0][i].x));
                landmark_pts_y.push_back(Point(pause_y[0][i].x));
            }
            for(int i = 0; i< pause_x[0].size()-6; i++){
                landmark_pts_x.push_back(Point(pause_x[0][i].x));
                landmark_pts_y.push_back(Point(pause_y[0][i].x));
            }
        }
    }
    //Side
    ////check where is the first point
    else if(ui->radioButton_2->isChecked()){
        for(int k=0; k<5; k++){
            if(k!=2)
                for(size_t j=0; j<contours_use[0].size(); j++){
                    square_d[k] = abs(pow(side_primary[k].x()-contours_use[0][j].x-offset_x+.0,2) +
                                      pow(side_primary[k].y()-contours_use[0][j].y-offset_y+.0,2));
                    if(square_d[k] < square_t2[k]){
                        square_t2[k] = square_d[k];
                        cali[k] = j;
                    }
                }
        }

        interval[0]= (cali[0]-cali[1])/6;
        interval[1]= (cali[3]-cali[4])/6;//**//
        /*for(int i=0; i<5; i++)
            qDebug()<<cali[i];*/
        landmark_pts_x.push_back(Point(side_primary[1].x()-offset_x));
        landmark_pts_y.push_back(Point(side_primary[1].y()-offset_y));
        for(int i=1; i<6; i++){
            landmark_pts_x.push_back(Point(contours_use[0][cali[1]+interval[0]*i].x));
            landmark_pts_y.push_back(Point(contours_use[0][cali[1]+interval[0]*i].y));
        }
        landmark_pts_x.push_back(Point(side_primary[0].x()-offset_x));
        landmark_pts_y.push_back(Point(side_primary[0].y()-offset_y));
        landmark_pts_x.push_back(Point(side_primary[4].x()-offset_x));
        landmark_pts_y.push_back(Point(side_primary[4].y()-offset_y));
        for(int i=1; i<6; i++){
            landmark_pts_x.push_back(Point(contours_use[0][cali[4]+interval[1]*i].x));
            landmark_pts_y.push_back(Point(contours_use[0][cali[4]+interval[1]*i].y));
        }
        landmark_pts_x.push_back(Point(side_primary[3].x()-offset_x));
        landmark_pts_y.push_back(Point(side_primary[3].y()-offset_y));
        landmark_pts_x.push_back(Point(side_primary[2].x()-offset_x));
        landmark_pts_y.push_back(Point(side_primary[2].y()-offset_y));
    }

    /*for(int i=0; i<landmark_pts_x.size();i++)
    {
        qDebug()<<landmark_pts_x[i].x;
        qDebug()<<landmark_pts_y[i].x;
    }*/

    //draw landmarks
    for(unsigned int i=0; i<landmark_pts_x.size(); i++){
        circle(showmask, Point(landmark_pts_x[i].x, landmark_pts_y[i].x) , 1, RED, 2);
    }

    //PIXEL 2 CM
    i=0;
    double area_cm, peri_cm, pa_ratio, radius_cm, min_rect_area_cm, radius_area_cm, rect_area_cm, rect_w_cm, rect_h_cm,
            wh_ratio, pri_radius_cm, pri_radius_area_cm, circle_ratio;
    area_cm = contourArea(contours[i])/square;
    peri_cm = arcLength( contours[i], true )/pixel;
    pa_ratio= area_cm/peri_cm;
    radius_cm = radius/pixel;
    radius_area_cm = abs(pow(radius_cm, 2))*pi;
    //rect_area_cm = boundingRect(contours_use[i]).area()/square;
    //min_rect_area_cm = min_rect_area/square;
    rect_w_cm = rect_w/pixel;
    rect_h_cm = rect_h/pixel;   //**// value aren't correst
    //wh_ratio = rect_w_cm/rect_h_cm;
    pri_radius_cm = pri_radius/pixel;
    pri_radius_area_cm = abs(pow(pri_radius_cm, 2))*pi;
    circle_ratio = pri_radius_area_cm/radius_area_cm;

    //qDebug("Area: %.1f , Length: %.1f \n", contourArea(contours[i]), arcLength( contours[i], true));
    //[record] & output
    string_r.sprintf("%.4f", peri_cm);    record.append(string_r);    record.append("\t");    ui->label_4->setText(string_r);
//    string_r.sprintf("%.4f", area_cm);    record.append(string_r);    record.append("\t");    ui->label_5->setText(string_r);
    //string_r.sprintf("%.4f", pa_ratio);    record.append(string_r);    record.append("\t\t");    ui->label_6->setText(string_r);
    string_r.sprintf("%.4f", radius_cm);    record.append(string_r);    record.append("\t");    ui->label_18->setText(string_r);
    string_r.sprintf("%.4f", radius_area_cm);    record.append(string_r);    record.append("\t");    ui->label_25->setText(string_r);
    string_r.sprintf("%.4f", rect_w_cm);    record.append(string_r);    record.append("\t");    ui->label_28->setText(string_r);
    string_r.sprintf("%.4f", rect_h_cm);    record.append(string_r);    record.append("\t");    ui->label_30->setText(string_r);
//    string_r.sprintf("%.4f", wh_ratio);    record.append(string_r);    record.append("\t");    ui->label_34->setText(string_r);
    //string_r.sprintf("%.4f", rect_area_cm);    record.append(string_r);    record.append("\t\t");    ui->label_32->setText(string_r);
    //string_r.sprintf("%.4f", min_rect_area_cm);    record.append(string_r);    record.append("\t\t");    ui->label_20->setText(string_r);
    string_r.sprintf("%.4f", pri_radius_cm);    record.append(string_r);    record.append("\t");    ui->label_50->setText(string_r);
    string_r.sprintf("%.4f", pri_radius_area_cm);    record.append(string_r);    record.append("\t");    ui->label_52->setText(string_r);
    string_r.sprintf("%.4f", circle_ratio);    record.append(string_r);    record.append("\t");    ui->label_54->setText(string_r);
    string_r.sprintf("%.4f", pixel);           record.append(string_r);    record.append("\t\t");


    //[output] - landmarks
    int k=0;
    if(ui->radioButton->isChecked()){
        for(k = 0; k < 5; k++){
            record.append(f_pri[k]);
            record.append("\t");
        }
        record.append("\t\t");
    }
    else if(ui->radioButton_2->isChecked()){
        for(k = 0; k < 5; k++){
            record.append(s_pri[k]);
            record.append("\t");
        }
        record.append("\t\t");
    }
    for(unsigned int i=0; i<landmark_pts_x.size(); i++){
        string_r.sprintf("%d\t", landmark_pts_x[i].x);
        record.append(string_r);
    }
    record.append("\t\n");
    record.append(file);
    record.append("_y\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
    for(unsigned int i=0; i<landmark_pts_x.size(); i++){
        if(ui->radioButton->isChecked())
            string_r.sprintf("%d\t",landmark_pts_y[i].x);
        if(ui->radioButton_2->isChecked())
            string_r.sprintf("%d\t",300-landmark_pts_y[i].x);
        record.append(string_r);
    }
    record.append("\n");

    /*size.height=ui->label_10->height();                                 //display image on Label
    size.width=(size.height)*(image.cols)/(image.rows);

    cv::resize(image,image,size);*/
    QImage img = QImage((uchar*)(showmask.data),showmask.cols,showmask.rows,QImage::Format_RGB888);
    ui->label_10->setPixmap(QPixmap::fromImage(img));
    ui->pushButton_6->setEnabled(true);
    //imshow("src", src);
    //imshow("contours", drawing);
    //imshow("ruler", ruler);

    for(unsigned int i=0; i<contours.size(); i++)
        contours[i].clear();
    for(unsigned int i=0; i<contours_use.size(); i++)
        contours_use[i].clear();
}

//
void MainWindow::on_pushButton_14_clicked()
{
    /*qDebug()<< record_mc;
    QFile outfile;
    if(ui->radioButton->isChecked())
        outfile.setFileName("record_mass.txt");
    outfile.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&outfile);
    out << record_mc;*/
}
                                                                        //Label use - undone
void MainWindow::mousePressEvent(QMouseEvent *event)
{
//LeftButton: rect;  RightButton: pixel to cm;  MidButton: primary points
if(event->x()<520 && event->y()<380)
{
    if((event->buttons() & Qt::MidButton) && ui->radioButton->isChecked()){
        if(ui->lcdNumber->value() > 0){
            if(QMessageBox::information(NULL, "Primary Point", "Primay Point ?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
                front_primary[num] = event->pos();
                string_r.sprintf("(%d, %d)", front_primary[num].x()-offset_x, front_primary[num].y()-offset_y);
                switch(num)
                {
                case 0:
                    ui->label_37->setText(string_r);
                    f_pri[0] = f_pri[0].append(string_r);                    
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(4);
                    break;
                case 1:
                    ui->label_41->setText(string_r);
                    f_pri[1] = f_pri[1].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(3);
                    break;
                case 2:
                    ui->label_45->setText(string_r);
                    f_pri[2] = f_pri[2].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(2);
                    break;
                case 3:
                    ui->label_43->setText(string_r);
                    f_pri[3] = f_pri[3].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(1);
                    break;
                case 4:
                    ui->label_39->setText(string_r);
                    f_pri[4] = f_pri[4].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(0);
                    break;
                }
                num++;
                pri_landmark_pts.push_back(Point(event->x(), event->y()));
            }
        }
    }
    else if((event->buttons() & Qt::MidButton) && ui->radioButton_2->isChecked()){
        if(ui->lcdNumber->value() > 0){
            if(QMessageBox::information(NULL, "Primary Point", "Primay Point ?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes){
                side_primary[num] = event->pos();
                string_r.sprintf("(%d, %d)", side_primary[num].x()-offset_x, side_primary[num].y()-offset_y);
                switch(num)
                {
                case 0:
                    ui->label_37->setText(string_r);
                    s_pri[0] = s_pri[0].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(4);
                    break;
                case 1:
                    ui->label_39->setText(string_r);
                    s_pri[1] = s_pri[1].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(3);
                    break;
                case 2:
                    ui->label_41->setText(string_r);
                    s_pri[2] = s_pri[2].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(2);
                    break;
                case 3:
                    ui->label_43->setText(string_r);
                    s_pri[3] = s_pri[3].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(1);
                    break;
                case 4:
                    ui->label_45->setText(string_r);
                    s_pri[4] = s_pri[4].append(string_r);
                    circle(image_preview, Point(event->x()-offset_x, event->y()-offset_y), ldmks_radius, RED, -1, 8, 0);
                    ui->label_9->setPixmap(QPixmap::fromImage(QImage((uchar*)(image_preview.data),image_preview.cols,image_preview.rows,QImage::Format_RGB888)));
                    ui->lcdNumber->display(0);
                    break;
                }
                num++;
                pri_landmark_pts.push_back(Point(event->x(), event->y()));
            }
        }
    }

    else if((event->buttons() & Qt::MidButton) && ui->lcdNumber->value() == 0 && (ui->radioButton->isChecked() || ui->radioButton_2->isChecked()))
        cali_landmark_x[cali_landmark_check].clear();
    else if((event->buttons() & Qt::RightButton) && ui->lcdNumber->value() == 0 && (ui->radioButton->isChecked() || ui->radioButton_2->isChecked())){
        cali_landmark_check++;
        if(ui->radioButton_2->isChecked())
            if(cali_landmark_check == 1)
                ui->label_62->setText("Ventral");
    }
}
    //pixel estimation
    if(event->buttons() & Qt::RightButton){
        if(ui->lcdNumber->value() == 5)
            firstpix = event->pos();
        string_r.sprintf("(%d, %d)", event->pos());
        ui->label_12->setText(string_r);
        ui->label_13->setText(string_r);

        temp_points = Point(event->x(), event->y());
        qDebug()<<temp_points.x;
        qDebug()<<temp_points.y;
        check_pt_setting= 1;
        qDebug()<<"ok";
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //pixel estimation
    if(event->buttons() & Qt::RightButton){
        if(ui->lcdNumber->value() == 5)
            lastpix = event->pos();
        string_r.sprintf("(%d, %d)", event->pos());
        ui->label_13->setText(string_r);
    }
    if(event->buttons() & Qt::LeftButton){
        if(ui->lcdNumber->value() == 0){
                cali_landmark_x[manu].push_back(Point(event->x()-offset_x));
                cali_landmark_y[manu].push_back(Point(event->y()-offset_y));
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->x()<520 && event->y()<380){
        if(pri_landmark_pts.size()>0)
            ui->pushButton_4->setEnabled(true);
        if(event->buttons() & Qt::RightButton)
            lastpix = event->pos();
    }
}

void MainWindow::on_radioButton_clicked()
{
    ui->line_6->setVisible(false);
    ui->line_5->setVisible(true);
    ui->line_7->setVisible(true);
    string_r.sprintf("Dorsal");
    ui->label_36->setText(string_r);
    string_r.sprintf("Lateral - R");
    ui->label_38->setText(string_r);
    string_r.sprintf("            - L");
    ui->label_40->setText(string_r);
    string_r.sprintf("Ventral - R");
    ui->label_42->setText(string_r);
    string_r.sprintf("             - L");
    ui->label_44->setText(string_r);
}

void MainWindow::on_radioButton_2_clicked()
{
    ui->line_5->setVisible(false);
    ui->line_6->setVisible(true);
    ui->line_7->setVisible(true);
    string_r.sprintf("Dorsal Base");
    ui->label_36->setText(string_r);
    string_r.sprintf("Dorsal Lobe");
    ui->label_38->setText(string_r);
    string_r.sprintf("Lateral Lobe");
    ui->label_40->setText(string_r);
    string_r.sprintf("Ventral Lobe");
    ui->label_42->setText(string_r);
    string_r.sprintf("Ventral Base");
    ui->label_44->setText(string_r);
}

void MainWindow::reset()
{
    int i;
    manu = 0;
    num = 0;                                         //reset if selected
    cali_landmark_check = 0;
    ui->pushButton_3->setEnabled(false);    ui->pushButton_6->setEnabled(false);    ui->pushButton_4->setEnabled(false);
    this->ui->radioButton->setAutoExclusive(false);     this->ui->radioButton->setChecked(false);
    this->ui->radioButton_2->setAutoExclusive(false);   this->ui->radioButton_2->setChecked(false);
    this->ui->pushButton_14->setEnabled(true);
    ui->line_5->setVisible(false);    ui->line_6->setVisible(false);
    ui->line_7->setVisible(false);    ui->line_8->setVisible(false);
    string_r.clear();    record.clear();    record_mc.clear();  record_contours.clear(); record_contours_use.clear();
    lastpix.setX(NULL);    lastpix.setY(NULL);
    pri_landmark_pts.clear();
    ui->lcdNumber->display(5);
    src.copyTo(image);    src.copyTo(ruler);
    ui->label_4->setText(string_r);    ui->label_5->setText(string_r);    ui->label_6->setText(string_r);
    ui->label_18->setText(string_r);    ui->label_25->setText(string_r);    ui->label_20->setText(string_r);
    ui->label_28->setText(string_r);    ui->label_30->setText(string_r);    ui->label_32->setText(string_r);
    ui->label_34->setText(string_r);    ui->label_36->setText(string_r);    ui->label_37->setText(string_r);
    ui->label_38->setText(string_r);    ui->label_39->setText(string_r);    ui->label_40->setText(string_r);
    ui->label_41->setText(string_r);    ui->label_42->setText(string_r);    ui->label_43->setText(string_r);
    ui->label_44->setText(string_r);    ui->label_45->setText(string_r);
    for(i=0; i<7; i++){
        cvt_onetwo_x_front[i].clear();  cvt_onetwo_y_front[i].clear();
        cvt_onetwo_x_side[i].clear();  cvt_onetwo_y_front[i].clear();}
    for(i=0; i<5; i++){
        pri_landmark_pts.clear();
        cali_landmark_x[i].clear();
        cali_landmark_y[i].clear();
        f_pri[i].clear();    s_pri[i].clear();
        curve[i] = 0;
        overlap_close_pt[i] = 0;        overlap_contours[i] = 0;        overlap_pri_pt[i] = 0;}
    pause_x[0].clear(); pause_y[0].clear();
    pri = 0;
    closept.clear();
    ui->label_62->setText("Dorsal");
    landmark_pts_x.clear();
    landmark_pts_y.clear();
    cvDestroyAllWindows();
}

void MainWindow::on_pushButton_5_clicked()
{
    //reset
    reset();
    record.append(file);
    record.append("_x\t");
    record_contours.append(file);
    record_contours.append("_x\t");
    record_contours_use.append(file);
    record_contours_use.append("_x\t");

    QImage img = QImage((uchar*)(image.data),image.cols,image.rows,QImage::Format_RGB888);
    ui->label_9->setPixmap(QPixmap::fromImage(img));
}

//save data
void MainWindow::on_pushButton_6_clicked()
{
    //qDebug()<< record;
    QFile outfile, outfile_contours, outfile_contours_use;
    if(ui->radioButton->isChecked())
        outfile.setFileName("record_front.txt");
    else if(ui->radioButton_2->isChecked())
        outfile.setFileName("record_side.txt");
    outfile.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&outfile);
    out << record << record_contours << record_contours_use << "\n";
    outfile.close();

  /*  if(ui->radioButton->isChecked())
        outfile_contours.setFileName("record_contours_front.txt");
    else if(ui->radioButton_2->isChecked())
        outfile_contours.setFileName("record_contours_side.txt");
    outfile_contours.open(QIODevice::Append | QIODevice::Text);
    QTextStream out_contours(&outfile_contours);
    qDebug()<<record_contours;
    out_contours << record_contours;
    outfile_contours.close();

    if(ui->radioButton->isChecked())
        outfile_contours_use.setFileName("record_contours_use_front.txt");
    else if(ui->radioButton_2->isChecked())
        outfile_contours_use.setFileName("record_contours_use_side.txt");
    outfile_contours_use.open(QIODevice::Append | QIODevice::Text);
    QTextStream out_contours_use(&outfile_contours_use);
    qDebug()<<record_contours_use;
    out_contours_use << record_contours_use;
    outfile_contours_use.close();
*/

}


void MainWindow::on_horizontalSlider_sliderReleased()
{
    slider_overlapping();
}

void MainWindow::on_horizontalSlider_2_sliderReleased()
{
    slider_overlapping();
}

void MainWindow::slider_overlapping()
{
    GaussianBlur(image_preview, image_preview, Size(1,1),0);
    Canny(image_preview, image_out, ui->horizontalSlider->value(), ui->horizontalSlider_2->value());
    imshow("1", image_out);
    //cvMoveWindow("1", 100, 600);
    //contours
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(image_out, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
    Mat black = Mat::zeros(image_preview.size(), CV_8UC3);
    black.copyTo(image_out1);
    bool pri_p;
    //find overlapping part
    if(ui->radioButton->isChecked()){
        for(int i=0; i<pri_landmark_pts.size(); i++){
            curve[i]=0;
            for(int j=0; j< contours.size(); j++){
                overlap_pri_d[i] = 1000.0;
                for(int k=0; k< contours[j].size(); k++){
                    dif_of_square_pri = pow(pri_landmark_pts[i].x - offset_x - contours[j][k].x + .0, 2) +
                            pow(pri_landmark_pts[i].y - offset_y - contours[j][k].y + .0, 2);
                    if(overlap_pri_d[i] > dif_of_square_pri){
                        overlap_pri_d[i] = dif_of_square_pri;
                        pri_p = true;

                        overlap_close_d[i] = 1000.0;
                        for(int m=0; m< contours[j].size(); m++){
                            dif_of_square_close = pow(closept[i].x - contours[j][m].x + .0, 2) + pow(closept[i].y - contours[j][m].y + .0, 2);
                            //find curve of overlapped part
                            if(overlap_close_d[i] > dif_of_square_close){
                                overlap_close_d[i] = dif_of_square_close;
                                overlap_close_pt[i] = m;

                                if(pri_p)
                                    overlap_pri_pt[i] = k;
                                pri_p = false;
                                overlap_contours[i] = j;
                                curve[i]=1;

                                switch(i)
                                {
                                case 0:
                                    drawContours(image_out1, contours, j, GREEN, 1, 8, hierarchy, 0);
                                    break;
                                case 1:
                                    drawContours(image_out1, contours, j, GREEN, 1, 8, hierarchy, 0);
                                    break;
                                case 2:
                                    drawContours(image_out1, contours, j, GREEN, 1, 8, hierarchy, 0);
                                    break;
                                case 3:
                                    drawContours(image_out1, contours, j, GREEN, 1, 8, hierarchy, 0);
                                    break;
                                case 4:
                                    drawContours(image_out1, contours, j, GREEN, 1, 8, hierarchy, 0);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if(curve[i]==1 && i==0){
                circle(image_out1, Point(contours[overlap_contours[i]][overlap_close_pt[i]]), 5, Scalar(133,20,255), -1);
                circle(image_out1, Point(contours[overlap_contours[i]][overlap_pri_pt[i]]), 5, Scalar(133,20,255), -1);
                circle(image_out1, Point(contours[overlap_contours[i]][int((overlap_pri_pt[i]+overlap_close_pt[i])/3)]), 5, Scalar(255,0,255), -1);
                circle(image_out1, Point(contours[overlap_contours[i]][int((overlap_pri_pt[i]+overlap_close_pt[i])*2/3)]), 5, Scalar(255,255,255), -1);
                qDebug()<<overlap_close_pt[i];
                qDebug()<<overlap_pri_pt[i];
                }
            if(curve[i]==1){
                if(overlap_pri_d[i] > 2){
                    line(image_out1, Point(pri_landmark_pts[i].x-offset_x, pri_landmark_pts[i].y-offset_y), Point(contours[overlap_contours[i]][overlap_pri_pt[i]].x, contours[overlap_contours[i]][overlap_pri_pt[i]].y), PURPLE, 1);
                    line_pri[i] = 1;
                }
                if(overlap_close_d[i] > 2){
                    line(image_out1, Point(closept[i].x, closept[i].y), Point(contours[overlap_contours[i]][overlap_close_pt[i]].x, contours[overlap_contours[i]][overlap_close_pt[i]].y), PURPLE, 1);
                    line_close[i] = 1;
                }
            }
            /*else if(curve[i]!=1)
                line(image_out1, Point(pri_landmark_pts[i].x-offset_x, pri_landmark_pts[i].y-offset_y), Point(closept[i].x, closept[i].y), BLUE, 1);*/
        }
    }
    else if(ui->radioButton_2->isChecked()){

    }

    imshow("2", image_out1);
}

void MainWindow::on_pushButton_10_clicked()
{
    overlapped_drawing();
}

//draw straight line
void MainWindow::on_pushButton_8_clicked()
{
   line_judge();
   pri++;
   manu++;
   if(ui->radioButton->isChecked())
       if(pri == 5 || manu == 5)
           ui->pushButton_3->setEnabled(true);
   if(ui->radioButton_2->isChecked())
       if(pri == 2 || manu == 2)
           ui->pushButton_3->setEnabled(true);
}

//if overlapped part didn't detect draw straight line
void MainWindow::line_judge()
{
    //y-ax-b=0
    Point pt1 = Point(closept[pri].x, closept[pri].y);
    Point pt2 = Point(pri_landmark_pts[pri].x-offset_x, pri_landmark_pts[pri].y-offset_y);
    LineIterator it(image_out1, pt1, pt2, 8);
    LineIterator it2 = it;
    vector<Vec3b> buf(it.count);
    for(int i = 0; i < it.count; i++, ++it){
        buf[i] = (const Vec3b)*it;
    }
    // alternative way of iterating through the line
    for(int i = 0; i < it2.count; i++, ++it2){
        Vec3b val = image_out1.at<Vec3b>(it2.pos());
        CV_Assert(buf[i] == val);
    }
    for(int i = 0; i < it2.count; i++, ++it2, ++it){
        //circle(image_out1, it2.pos()- (pt2 - pt1), 1, Scalar(255,0,0),-1);
        circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 2, Scalar(255,0,0),-1);
        cali_landmark_x[pri].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
        cali_landmark_y[pri].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
    }
    int size_cali = cali_landmark_x[pri].size()-1;
    for(int i=size_cali; i>=0 ; i--){
        cali_landmark_x[pri].push_back(Point(cali_landmark_x[pri][i].x));
        cali_landmark_y[pri].push_back(Point(cali_landmark_y[pri][i].x));
    }
    imshow("2", image_out1);
}

void MainWindow::overlapped_drawing()
{
    GaussianBlur(image_preview, image_preview, Size(1,1),0);
    Canny(image_preview, image_out, ui->horizontalSlider->value(), ui->horizontalSlider_2->value());
    //contours
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(image_out, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

    //front
    if(ui->radioButton->isChecked()){
        float slope_pri[5] = {0};
        float const_pri[5] = {0};
        float slope_close[5] = {0};
        float const_close[5] = {0};
        float slope_full=0.0;
        float const_full=0.0;
        float slope_contours, const_contours;
        float vertical_straight_pri = 0;
        float vertical_straight_close = 0;
        float vertical_straight_contours = 0;
        int RL = 0; //1->R, 2->L

        //overlap
        qDebug()<<pri;
        //detect the curve
        if(curve[pri] == 1){
            //line func - connect the pri and contour
            if(line_pri[pri] == 1){
                if(pri_landmark_pts[pri].x-offset_x-contours[overlap_contours[pri]][overlap_pri_pt[pri]].x == 0){
                    vertical_straight_pri = 1;
                    const_pri[pri] = (-1)*(pri_landmark_pts[pri].x-offset_x);
                }
                else{
                    slope_pri[pri] = (pri_landmark_pts[pri].y-offset_y-contours[overlap_contours[pri]][overlap_pri_pt[pri]].y)/(pri_landmark_pts[pri].x-offset_x-contours[overlap_contours[pri]][overlap_pri_pt[pri]].x);
                    const_pri[pri] = pri_landmark_pts[pri].y-offset_y-slope_pri[pri]*(pri_landmark_pts[pri].x-offset_x);
                }
            }
            //line func - connect the close and contour
            if(check_pt_setting == 0){
                if(line_close[pri] == 1){
                    if(closept[pri].x-contours[overlap_contours[pri]][overlap_close_pt[pri]].x == 0){
                        vertical_straight_close = 1;
                        const_close[pri] = (-1)*closept[pri].x;
                    }
                    else{
                        slope_close[pri] = (closept[pri].y-contours[overlap_contours[pri]][overlap_close_pt[pri]].y)/(closept[pri].x-contours[overlap_contours[pri]][overlap_close_pt[pri]].x);
                        const_close[pri] = closept[pri].y - slope_close[pri]*closept[pri].x;
                    }
                }
            }
            else{
                slope_close[pri] = (temp_points.y-contours[overlap_contours[pri]][overlap_close_pt[pri]].y)/(temp_points.x-contours[overlap_contours[pri]][overlap_close_pt[pri]].x);
                const_close[pri] = temp_points.y - slope_close[pri]*temp_points.x;
            }
            //line func - contour
            if(contours[overlap_contours[pri]][overlap_pri_pt[pri]].x-contours[overlap_contours[pri]][overlap_close_pt[pri]].x == 0){
                vertical_straight_contours = 1;
                const_contours = (-1)*contours[overlap_contours[pri]][overlap_pri_pt[pri]].x;
            }
            else{
                slope_contours = (contours[overlap_contours[pri]][overlap_pri_pt[pri]].y-contours[overlap_contours[pri]][overlap_close_pt[pri]].y)/(contours[overlap_contours[pri]][overlap_pri_pt[pri]].x-contours[overlap_contours[pri]][overlap_close_pt[pri]].x);
                const_contours = contours[overlap_contours[pri]][overlap_pri_pt[pri]].y - slope_contours*(contours[overlap_contours[pri]][overlap_pri_pt[pri]].x);
            }
        }
        //line func - pri to close
        //**//check_pt_setting is a big problem!!!!!!
        if(check_pt_setting == 0){
            if(pri_landmark_pts[pri].x-offset_x-closept[pri].x == 0){
                slope_full = 1;     ////wihtout this, pri[0] won't reflect
                const_full = (-1)*closept[pri].x;
            }
            else{
                slope_full = (pri_landmark_pts[pri].y-offset_y-closept[pri].y+.0)/(pri_landmark_pts[pri].x-offset_x-closept[pri].x+.0);
                const_full = closept[pri].y - slope_full*closept[pri].x;
            }
        }
        else{
            slope_full = (pri_landmark_pts[pri].y-temp_points.y+.0)/(pri_landmark_pts[pri].x-temp_points.x+.0);
            const_full = temp_points.y - slope_full*temp_points.x;
        }

        //line(image_out1, Point(pri_landmark_pts[pri].x-offset_x, pri_landmark_pts[pri].y-offset_y), Point(closept[pri].x, closept[pri].y),Scalar(255,255,0));

        //judge - visible petal is left or right part
        //pri[3]&[4]: + -> L, - -> R   [else]: + -> R, - -> L
        if(curve[pri] == 1){
            float count;
            for(int j = 0; j < contours[overlap_contours[pri]].size(); j++){
                if(contours[overlap_contours[pri]][j].y - slope_full*contours[overlap_contours[pri]][j].x - const_full > 0)
                    count++;//**//
            }
            count = count/(contours[overlap_contours[pri]].size()+.0);
            if(count >= 0.5){
                switch(pri)
                {
                case 0:
                    if(slope_full >= 0)
                        RL = 2;
                    else
                        RL = 1;
                    break;
                case 1:
                    RL = 2;
                    break;
                case 2:
                    RL = 2;
                    break;
                case 3:
                    RL = 1;
                    break;
                case 4:
                    RL = 1;
                    break;
                }
            }
            else{
                switch(pri)
                {
                case 0:
                    if(slope_full >= 0)
                        RL = 1;
                    else
                        RL = 2;
                    break;
                case 1:
                    RL = 1;
                    break;
                case 2:
                    RL = 1;
                    break;
                case 3:
                    RL = 2;
                    break;
                case 4:
                    RL = 2;
                    break;
                }
            }

            //pri close line - draw close2contour, contour2pri
            Point close = Point(closept[pri].x, closept[pri].y);
            Point contour_c = Point(contours[overlap_contours[pri]][overlap_close_pt[pri]].x, contours[overlap_contours[pri]][overlap_close_pt[pri]].y);
            Point contour_p = Point(contours[overlap_contours[pri]][overlap_pri_pt[pri]].x, contours[overlap_contours[pri]][overlap_pri_pt[pri]].y);
            Point pp = Point(pri_landmark_pts[pri].x-offset_x, pri_landmark_pts[pri].y-offset_y);

            int h, l;
            //initial pt of contours
            if(overlap_close_pt[pri] <= overlap_pri_pt[pri]){
                l = overlap_close_pt[pri];
                h = overlap_pri_pt[pri];
                //close->contours->pri
                if(RL ==1){
                    qDebug()<<"R, clo2pri";
                    //close
                    if(line_close[pri] == 1){
                        Point pt1 = close;
                        Point pt2 = contour_c;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            pause_x[0].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            pause_y[0].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //contours part
                    for(size_t j= l; j< h; j++){
                        pause_x[0].push_back(Point(int(contours[overlap_contours[pri]][j].x)));
                        pause_y[0].push_back(Point(int(contours[overlap_contours[pri]][j].y)));
                    }
                    //pri
                    if(line_pri[pri] == 1){
                        Point pt1 = contour_p;
                        Point pt2 = pp;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            pause_x[0].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            pause_y[0].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //reflect
                    init_x = pri_landmark_pts[pri].x-offset_x;
                    init_y = pri_landmark_pts[pri].y-offset_y;
                    if(pri_landmark_pts[pri].x-offset_x-closept[pri].x ==0){
                        bx = 0;
                        by = 1;
                    }
                    else if(pri_landmark_pts[pri].y-offset_y-closept[pri].y ==0){
                        bx = 1;
                        by = 0;
                    }
                    else{
                        bx = 1;
                        by = slope_full;
                    }
                    for(size_t j = 0; j<pause_x[0].size()-1; j++){
                        k = 2*((pause_x[0][j].x-init_x)*bx+(pause_y[0][j].x-init_y)*by)/(bx*bx+by*by);
                        xx = k*bx-(pause_x[0][j].x-init_x)+init_x;
                        yy = k*by-(pause_y[0][j].x-init_y)+init_y;
                        cali_landmark_x[pri].push_back(Point(int(xx)));
                        cali_landmark_y[pri].push_back(Point(int(yy)));
                        circle(image_out1, Point(int(xx),int(yy)), 1, WHITE,-1);
                    }
                    for(size_t j = pause_x[0].size()-1; j>0; j--){
                        cali_landmark_x[pri].push_back(Point(int(pause_x[0][j].x)));
                        cali_landmark_y[pri].push_back(Point(int(pause_y[0][j].x)));
                        circle(image_out1, Point(int(pause_x[0][j].x),int(pause_y[0][j].x)), 1, WHITE,-1);
                    }
                    pause_x[0].clear();
                    pause_y[0].clear();
                }
                else if(RL ==2){
                    qDebug()<<"L, clo2pri";
                    //close
                    if(line_close[pri] == 1){
                        Point pt1 = close;
                        Point pt2 = contour_c;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            cali_landmark_x[pri].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            cali_landmark_y[pri].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //contours part
                    qDebug()<<l;qDebug()<<h;qDebug()<<int(l + (h-l)/3);qDebug()<<int(l + (h-l)*2/3);
                    for(size_t j= l; j< h; j++){
                        cali_landmark_x[pri].push_back(Point(int(contours[overlap_contours[pri]][j].x)));
                        cali_landmark_y[pri].push_back(Point(int(contours[overlap_contours[pri]][j].y)));
                        /*if(j%4 == 1)
                            circle(image_out1, Point(contours[overlap_contours[pri]][j]), 3, Scalar(21,21,200), -1);
                        else
                            circle(image_out1, Point(contours[overlap_contours[pri]][j]), 2, Scalar(210,20,20), -1);*/
                    }
                    //pri
                    if(line_pri[pri] == 1){
                        Point pt1 = contour_p;
                        Point pt2 = pp;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            pause_x[0].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            pause_y[0].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //reflect
                    init_x = pri_landmark_pts[pri].x-offset_x;
                    init_y = pri_landmark_pts[pri].y-offset_y;
                    if(pri_landmark_pts[pri].x-offset_x-closept[pri].x ==0){
                        bx = 0;
                        by = 1;
                    }
                    else if(pri_landmark_pts[pri].y-offset_y-closept[pri].y ==0){
                        bx = 1;
                        by = 0;
                    }
                    else{
                        bx = 1;
                        by = slope_full;
                    }
                    for(size_t j = cali_landmark_x[pri].size()-1; j>0; j--){
                        k = 2*((cali_landmark_x[pri][j].x-init_x)*bx+(cali_landmark_y[pri][j].x-init_y)*by)/(bx*bx+by*by);
                        xx = k*bx-(cali_landmark_x[pri][j].x-init_x)+init_x;
                        yy = k*by-(cali_landmark_y[pri][j].x-init_y)+init_y;
                        cali_landmark_x[pri].push_back(Point(int(xx)));
                        cali_landmark_y[pri].push_back(Point(int(yy)));
                        circle(image_out1, Point(int(xx),int(yy)), 1, WHITE,-1);
                    }
                    pause_x[0].clear();
                    pause_y[0].clear();
                }
            }
            //pri->contours->close
            else if(overlap_close_pt[pri] > overlap_pri_pt[pri]){
                h = overlap_close_pt[pri];
                l = overlap_pri_pt[pri];
                if(RL ==1){
                    qDebug()<<"R, pri2clo";
                    //pri
                    if(line_pri[pri] == 1){
                        Point pt1 = pp;
                        Point pt2 = contour_p;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            pause_x[0].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            pause_y[0].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //contours part
                    for(size_t j= l; j< h; j++){
                        pause_x[0].push_back(Point(int(contours[overlap_contours[pri]][j].x)));
                        pause_y[0].push_back(Point(int(contours[overlap_contours[pri]][j].y)));
                    }
                    //close
                    if(line_close[pri] == 1){
                        Point pt1 = contour_c;
                        Point pt2 = close;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 3, Scalar(255,0,0),-1);
                            pause_x[0].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            pause_y[0].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //reflect
                    init_x = pri_landmark_pts[pri].x-offset_x;
                    init_y = pri_landmark_pts[pri].y-offset_y;
                    if(pri_landmark_pts[pri].x-offset_x-closept[pri].x ==0){
                        bx = 0;
                        by = 1;
                    }
                    else if(pri_landmark_pts[pri].y-offset_y-closept[pri].y ==0){
                        bx = 1;
                        by = 0;
                    }
                    else{
                        bx = 1;
                        by = slope_full;
                    }
                    for(size_t j = pause_x[0].size()-1; j > 0; j--){
                        k = 2*((pause_x[0][j].x-init_x)*bx+(pause_y[0][j].x-init_y)*by)/(bx*bx+by*by);
                        xx = k*bx-(pause_x[0][j].x-init_x)+init_x;
                        yy = k*by-(pause_y[0][j].x-init_y)+init_y;
                        cali_landmark_x[pri].push_back(Point(int(xx)));
                        cali_landmark_y[pri].push_back(Point(int(yy)));
                        circle(image_out1, Point(int(xx),int(yy)), 1, WHITE,-1);
                    }
                    for(size_t j = 0; j < pause_x[0].size(); j++){
                        cali_landmark_x[pri].push_back(Point(int(pause_x[0][j].x)));
                        cali_landmark_y[pri].push_back(Point(int(pause_y[0][j].x)));
                        circle(image_out1, Point(int(pause_x[0][j].x), int(pause_y[0][j].x)), 1, WHITE,-1);
                    }
                    pause_x[0].clear();
                    pause_y[0].clear();
                }
                else if(RL ==2){
                    qDebug()<<"L, pri2clo";
                    //close
                    if(line_close[pri] == 1){
                        Point pt1 = close;
                        Point pt2 = contour_c;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            cali_landmark_x[pri].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            cali_landmark_y[pri].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //contours part
                    qDebug()<<h;
                    qDebug()<<l;
                    for(size_t j = h; j > l; j--){
                        cali_landmark_x[pri].push_back(Point(int(contours[overlap_contours[pri]][j].x)));
                        cali_landmark_y[pri].push_back(Point(int(contours[overlap_contours[pri]][j].y)));
                    }
                    //pri
                    if(line_pri[pri] == 1){
                        Point pt1 = contour_p;
                        Point pt2 = pp;
                        LineIterator it(image_out1, pt1, pt2, 8);
                        LineIterator it2 = it;
                        vector<Vec3b> buf(it.count);
                        for(int i = 0; i < it.count; i++, ++it){
                            buf[i] = (const Vec3b)*it;
                        }
                        // alternative way of iterating through the line
                        for(int i = 0; i < it2.count; i++, ++it2){
                            Vec3b val = image_out1.at<Vec3b>(it2.pos());
                            CV_Assert(buf[i] == val);
                        }
                        for(int i = 0; i < it2.count; i++, ++it2, ++it){
                            circle(image_out1, Point(it2.pos().x- (pt2.x - pt1.x), it2.pos().y-(pt2.y-pt1.y)), 1, Scalar(255,0,0),-1);
                            cali_landmark_x[pri].push_back(Point(it2.pos().x- (pt2.x - pt1.x)));
                            cali_landmark_y[pri].push_back(Point(it2.pos().y-(pt2.y-pt1.y)));
                        }
                    }
                    //reflect
                    init_x = pri_landmark_pts[pri].x-offset_x;
                    init_y = pri_landmark_pts[pri].y-offset_y;
                    if(pri_landmark_pts[pri].x-offset_x-closept[pri].x ==0){
                        bx = 0;
                        by = 1;
                    }
                    else if(pri_landmark_pts[pri].y-offset_y-closept[pri].y ==0){
                        bx = 1;
                        by = 0;
                    }
                    else{
                        bx = 1;
                        by = slope_full;
                    }
                    for(size_t j = cali_landmark_x[pri].size()-1; j>0; j--){
                        k = 2*((cali_landmark_x[pri][j].x-init_x)*bx+(cali_landmark_y[pri][j].x-init_y)*by)/(bx*bx+by*by);
                        xx = k*bx-(cali_landmark_x[pri][j].x-init_x)+init_x;
                        yy = k*by-(cali_landmark_y[pri][j].x-init_y)+init_y;
                        cali_landmark_x[pri].push_back(Point(int(xx)));
                        cali_landmark_y[pri].push_back(Point(int(yy)));
                        circle(image_out1, Point(int(xx),int(yy)), 1, WHITE,-1);
                    }
                }
            }
        }
        else{
            line_judge();
        }
    }
    else if(ui->radioButton_2->isChecked()){

    }

    line_close[pri] = 0;
    line_pri[pri] = 0;
    imshow("2", image_out1);
    //cvMoveWindow("2", 100,600);
    pri++;
    manu++;
    if(ui->radioButton->isChecked())
        if(pri == 5 || manu == 5)
            ui->pushButton_3->setEnabled(true);
    if(ui->radioButton_2->isChecked())
        if(pri == 2 || manu == 2)
            ui->pushButton_3->setEnabled(true);

    check_pt_setting = 0;
}

//manu - R, reflect L
void MainWindow::on_pushButton_11_clicked()
{
    if(ui->radioButton->isChecked()){
        init_x = pri_landmark_pts[manu].x-offset_x;
        init_y = pri_landmark_pts[manu].y-offset_y;
        if(check_pt_setting == 0){
            if(init_x-closept[manu].x ==0){
                bx = 0;
                by = 1;
            }
            else if(init_y-closept[manu].y ==0){
                bx = 1;
                by = 0;
            }
            else{
                bx = 1;
                by = (pri_landmark_pts[manu].y-offset_y-closept[manu].y+.0)/(pri_landmark_pts[manu].x-offset_x-closept[manu].x+.0);;
            }
        }
        else{
            bx = 1;
            by = (pri_landmark_pts[manu].y-temp_points.y+.0)/(pri_landmark_pts[manu].x-temp_points.x+.0);;
        }
        for(size_t j = cali_landmark_x[manu].size()-1; j>0; j--){
            pause_x[0].push_back(Point(cali_landmark_x[manu][j].x));
            pause_y[0].push_back(Point(cali_landmark_y[manu][j].x));
        }
        cali_landmark_x[manu].clear();
        cali_landmark_y[manu].clear();
        for(size_t j = pause_x[0].size()-1; j>0; j--){
            k = 2*((pause_x[0][j].x-init_x)*bx+(pause_y[0][j].x-init_y)*by)/(bx*bx+by*by);
            xx = k*bx-(pause_x[0][j].x-init_x)+init_x;
            yy = k*by-(pause_y[0][j].x-init_y)+init_y;
            cali_landmark_x[manu].push_back(Point(int(xx)));
            cali_landmark_y[manu].push_back(Point(int(yy)));
            circle(image_out1, Point(int(xx), int(yy)), 2, Scalar(255,255,255), -1);
        }
        for(size_t j=0; j<pause_x[0].size()-1; j++){
            cali_landmark_x[manu].push_back(Point(int(pause_x[0][j].x)));
            cali_landmark_y[manu].push_back(Point(int(pause_y[0][j].x)));
            circle(image_out1, Point(pause_x[0][j].x, pause_y[0][j].x), 2, Scalar(255,255,255), -1);
        }
        pause_x[0].clear(); pause_y[0].clear();
    }

    imshow("2", image_out1);
    manu++;
    pri++;
    if(ui->radioButton->isChecked())
        if(pri == 5 || manu == 5)
            ui->pushButton_3->setEnabled(true);
    if(ui->radioButton_2->isChecked())
        if(pri == 2 || manu == 2)
            ui->pushButton_3->setEnabled(true);
    check_pt_setting = 0;
}

//manu - L, reflect R
void MainWindow::on_pushButton_13_clicked()
{
    init_x = pri_landmark_pts[manu].x-offset_x;
    init_y = pri_landmark_pts[manu].y-offset_y;
    if(check_pt_setting == 0){
        if(init_x-closept[manu].x ==0){
            bx = 0;
            by = 1;
        }
        else if(init_y-closept[manu].y ==0){
            bx = 1;
            by = 0;
        }
        else{
            bx = 1;
            by = (pri_landmark_pts[manu].y-offset_y-closept[manu].y+.0)/(pri_landmark_pts[manu].x-offset_x-closept[manu].x+.0);;
        }
    }
    else{
        bx = 1;
        by = (pri_landmark_pts[manu].y-temp_points.y+.0)/(pri_landmark_pts[manu].x-temp_points.x+.0);;
    }
    for(size_t j = 0; j<cali_landmark_x[manu].size(); j++){
        circle(image_out1, Point(cali_landmark_x[manu][j].x, cali_landmark_y[manu][j].x), 2, Scalar(255,255,255), -1);
    }
    for(size_t j = cali_landmark_x[manu].size()-1; j>0; j--){
        k = 2*((cali_landmark_x[manu][j].x-init_x)*bx+(cali_landmark_y[manu][j].x-init_y)*by)/(bx*bx+by*by);
        xx = k*bx-(cali_landmark_x[manu][j].x-init_x)+init_x;
        yy = k*by-(cali_landmark_y[manu][j].x-init_y)+init_y;
        cali_landmark_x[manu].push_back(Point(int(xx)));
        cali_landmark_y[manu].push_back(Point(int(yy)));
        circle(image_out1, Point(int(xx), int(yy)), 2, Scalar(255,255,255), -1);
    }
    imshow("2", image_out1);
    manu++;
    pri++;
    if(ui->radioButton->isChecked())
        if(pri == 5 || manu == 5)
            ui->pushButton_3->setEnabled(true);
    if(ui->radioButton_2->isChecked())
        if(pri == 2 || manu == 2)
            ui->pushButton_3->setEnabled(true);

    check_pt_setting = 0;
}

void MainWindow::on_pushButton_12_clicked()
{
    manu--;
    pri--;
    cali_landmark_x[manu].clear();
    cali_landmark_y[manu].clear();
}

void MainWindow::on_pushButton_15_clicked()
{
    manu++;
    pri++;
    if(ui->radioButton->isChecked())
        if(pri == 5 || manu == 5)
            ui->pushButton_3->setEnabled(true);
    if(ui->radioButton_2->isChecked())
        if(pri == 2 || manu == 2)
            ui->pushButton_3->setEnabled(true);
}

//read - asymmetric score
void MainWindow::on_pushButton_7_clicked()
{
    //load data
    QFile outfile;
    outfile.setFileName("record_score.txt");
    outfile.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&outfile);
    QFile infile;
    int linecount = 0;
    int numb;
    float d_1, d_2, d_3, d_4, d_5;
    infile.setFileName("record_mass.txt");
    infile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&infile);
    vector<Point2f> center, petal_pt_1, petal_pt_2, petal_pt_3, petal_pt_4, petal_pt_5;
    while(!in.atEnd()){
        QString line_x = in.readLine();
        QString line_y = in.readLine();
        linecount+=2;
        numb = linecount/2-1;
        qDebug()<<numb+1;
        QString string_x = line_x.section("\t", 1, 1);
        QString string_y = line_y.section("\t", 1, 1);
        center.push_back(Point2f(string_x.toFloat(), string_y.toFloat()));
        string_x = line_x.section("\t", 2, 2);
        string_y = line_y.section("\t", 2, 2);
        petal_pt_5.push_back(Point2f(string_x.toFloat(), string_y.toFloat()));
        string_x = line_x.section("\t", 3, 3);
        string_y = line_y.section("\t", 3, 3);
        petal_pt_4.push_back(Point2f(string_x.toFloat(), string_y.toFloat()));
        string_x = line_x.section("\t", 4, 4);
        string_y = line_y.section("\t", 4, 4);
        petal_pt_3.push_back(Point2f(string_x.toFloat(), string_y.toFloat()));
        string_x = line_x.section("\t", 5, 5);
        string_y = line_y.section("\t", 5, 5);
        petal_pt_2.push_back(Point2f(string_x.toFloat(), string_y.toFloat()));
        string_x = line_x.section("\t", 6, 6);
        string_y = line_y.section("\t", 6, 6);
        petal_pt_1.push_back(Point2f(string_x.toFloat(), string_y.toFloat()));

        d_1 = sqrtf(powf(center[numb].x - petal_pt_1[numb].x + .0, 2) + pow(center[numb].y - petal_pt_1[numb].y + .0, 2));
        d_2 = sqrtf(powf(center[numb].x - petal_pt_2[numb].x + .0, 2) + pow(center[numb].y - petal_pt_2[numb].y + .0, 2));
        d_3 = sqrtf(powf(center[numb].x - petal_pt_3[numb].x + .0, 2) + pow(center[numb].y - petal_pt_3[numb].y + .0, 2));
        d_4 = sqrtf(powf(center[numb].x - petal_pt_4[numb].x + .0, 2) + pow(center[numb].y - petal_pt_4[numb].y + .0, 2));
        d_5 = sqrtf(powf(center[numb].x - petal_pt_5[numb].x + .0, 2) + pow(center[numb].y - petal_pt_5[numb].y + .0, 2));
//        qDebug()<<petal_pt_1[numb].x;qDebug()<<petal_pt_1[numb].y;
//        qDebug()<<petal_pt_3[numb].x;qDebug()<<petal_pt_3[numb].y;
//        qDebug()<<"----------";
//        qDebug()<<d_1;qDebug()<<d_2;qDebug()<<d_3;qDebug()<<d_4;qDebug()<<d_5;//qDebug()<<longest;qDebug()<<longest_d;qDebug()<<"----------";

        //find longest line: longest->number, value of d_o->0~1
        int longest = 1;
        float longest_d = d_1;
        if(longest_d < d_2){
            longest = 2;
            longest_d = d_2;
        }
        if(longest_d < d_3){
            longest = 3;
            longest_d = d_3;
        }
        if(longest_d < d_4){
            longest = 4;
            longest_d = d_4;
        }
        if(longest_d < d_5){
            longest = 5;
            longest_d = d_5;
        }
        d_1 = d_1/longest_d;
        d_2 = d_2/longest_d;
        d_3 = d_3/longest_d;
        d_4 = d_4/longest_d;
        d_5 = d_5/longest_d;
//        qDebug()<<"longest";qDebug()<<longest;

        //adjust "1": longest line-> number 1
        float d_tmp;
        Point petal_tmp;
        switch(longest){
        case 2:
            d_tmp = d_1;            d_1 = d_2;
            d_2 = d_3;            d_3 = d_4;
            d_4 = d_5;            d_5 = d_tmp;
            petal_tmp.x = petal_pt_1[numb].x;           petal_tmp.y = petal_pt_1[numb].y;
            petal_pt_1[numb].x = petal_pt_2[numb].x;    petal_pt_1[numb].y = petal_pt_2[numb].y;
            petal_pt_2[numb].x = petal_pt_3[numb].x;    petal_pt_2[numb].y = petal_pt_3[numb].y;
            petal_pt_3[numb].x = petal_pt_4[numb].x;    petal_pt_3[numb].y = petal_pt_4[numb].y;
            petal_pt_4[numb].x = petal_pt_5[numb].x;    petal_pt_4[numb].y = petal_pt_5[numb].y;
            petal_pt_5[numb].x = petal_tmp.x;           petal_pt_5[numb].y = petal_tmp.y;
            break;
        case 3:
            d_tmp = d_1;            d_1 = d_3;
            d_3 = d_5;            d_5 = d_2;
            d_2 = d_4;            d_4 = d_tmp;
            petal_tmp.x = petal_pt_1[numb].x;           petal_tmp.y = petal_pt_1[numb].y;
            petal_pt_1[numb].x = petal_pt_3[numb].x;    petal_pt_1[numb].y = petal_pt_3[numb].y;
            petal_pt_3[numb].x = petal_pt_5[numb].x;    petal_pt_3[numb].y = petal_pt_5[numb].y;
            petal_pt_5[numb].x = petal_pt_2[numb].x;    petal_pt_5[numb].y = petal_pt_2[numb].y;
            petal_pt_2[numb].x = petal_pt_4[numb].x;    petal_pt_2[numb].y = petal_pt_4[numb].y;
            petal_pt_4[numb].x = petal_tmp.x;           petal_pt_4[numb].y = petal_tmp.y;
            break;
        case 4:
            d_tmp = d_1;            d_1 = d_4;
            d_4 = d_2;            d_2 = d_5;
            d_5 = d_3;            d_3 = d_tmp;
            petal_tmp.x = petal_pt_1[numb].x;           petal_tmp.y = petal_pt_1[numb].y;
            petal_pt_1[numb].x = petal_pt_4[numb].x;    petal_pt_1[numb].y = petal_pt_4[numb].y;
            petal_pt_4[numb].x = petal_pt_2[numb].x;    petal_pt_4[numb].y = petal_pt_2[numb].y;
            petal_pt_2[numb].x = petal_pt_5[numb].x;    petal_pt_2[numb].y = petal_pt_5[numb].y;
            petal_pt_5[numb].x = petal_pt_3[numb].x;    petal_pt_5[numb].y = petal_pt_3[numb].y;
            petal_pt_3[numb].x = petal_tmp.x;           petal_pt_3[numb].y = petal_tmp.y;
            break;
        case 5:
            d_tmp = d_1;            d_1 = d_5;
            d_5 = d_4;            d_4 = d_3;
            d_3 = d_2;            d_2 = d_tmp;
            petal_tmp.x = petal_pt_1[numb].x;           petal_tmp.y = petal_pt_1[numb].y;
            petal_pt_1[numb].x = petal_pt_5[numb].x;    petal_pt_1[numb].y = petal_pt_5[numb].y;
            petal_pt_5[numb].x = petal_pt_4[numb].x;    petal_pt_5[numb].y = petal_pt_4[numb].y;
            petal_pt_4[numb].x = petal_pt_3[numb].x;    petal_pt_4[numb].y = petal_pt_3[numb].y;
            petal_pt_3[numb].x = petal_pt_2[numb].x;    petal_pt_3[numb].y = petal_pt_2[numb].y;
            petal_pt_2[numb].x = petal_tmp.x;           petal_pt_2[numb].y = petal_tmp.y;
            break;
        }
        //avg length
        float alpha = 0;
        alpha = (d_1+d_2+d_3+d_4+d_5)/5.0;
//        qDebug()<<"alpha";qDebug()<<alpha;
//        qDebug()<<d_1;qDebug()<<d_2;qDebug()<<d_3;qDebug()<<d_4;qDebug()<<d_5;qDebug()<<"----------";

        //angle rotation to longest line is zero degree to x-axis
        float pxx, pyy, theta_1, theta_2, theta_3, theta_4, theta_5, x_f, y_f, tmp;
        y_f = petal_pt_1[numb].y - center[numb].y +.0;
        x_f = petal_pt_1[numb].x - center[numb].x +.0;
        if(longest == 1){
            theta_1 = 1.5*pi - atanf((y_f)/(x_f));
        }
        else if(longest == 2){
            theta_1 = pi + atanf((y_f)/(x_f));
        }
        else if(longest == 3){
            if(petal_pt_1[numb].x > center[numb].x)
                theta_1 = atanf((y_f)/(x_f));
            else if(petal_pt_1[numb].x == center[numb].x)
                theta_1 = pi/2;
            else
                theta_1 = pi + atanf((y_f)/(x_f));
        }
        else if(longest == 4){
            theta_1 = atanf((y_f)/(x_f));
        }
        else if(longest == 5){
            theta_1 = 2*pi + atanf((y_f)/(x_f));
        }
        theta_1 = -1.0*(theta_1);
        tmp = theta_1+.0;

        //rotate other line to I, IV phases
        while(theta_1 != 0.0){
            pxx = cosf(theta_1) * (petal_pt_1[numb].x-center[numb].x+.0) - sinf(theta_1) * (petal_pt_1[numb].y - center[numb].y+.0) + center[numb].x;
            pyy = sinf(theta_1) * (petal_pt_1[numb].x-center[numb].x+.0) + cosf(theta_1) * (petal_pt_1[numb].y - center[numb].y+.0) + center[numb].y;
            petal_pt_1[numb].x = pxx;
            petal_pt_1[numb].y = pyy;
            theta_1 = -1.0*atanf((petal_pt_1[numb].y-center[numb].y+.0)/(petal_pt_1[numb].x-center[numb].x+.0));
            tmp = tmp + theta_1;
        }
//        qDebug()<<petal_pt_1[numb].x;qDebug()<<petal_pt_1[numb].y;
//        qDebug()<<tmp;
        theta_2 = -0.4*pi + tmp;
        pxx = cosf(theta_2) * (petal_pt_2[numb].x-center[numb].x+.0) - sinf(theta_2) * (petal_pt_2[numb].y - center[numb].y+.0) + center[numb].x;
        pyy = sinf(theta_2) * (petal_pt_2[numb].x-center[numb].x+.0) + cosf(theta_2) * (petal_pt_2[numb].y - center[numb].y+.0) + center[numb].y;
        petal_pt_2[numb].x = pxx;
        petal_pt_2[numb].y = pyy;
        theta_2 = atanf((petal_pt_2[numb].y-center[numb].y+.0)/(petal_pt_2[numb].x-center[numb].x+.0));
        theta_3 = -0.8*pi + tmp;
        pxx = cosf(theta_3) * (petal_pt_3[numb].x-center[numb].x+.0) - sinf(theta_3) * (petal_pt_3[numb].y - center[numb].y+.0) + center[numb].x;
        pyy = sinf(theta_3) * (petal_pt_3[numb].x-center[numb].x+.0) + cosf(theta_3) * (petal_pt_3[numb].y - center[numb].y+.0) + center[numb].y;
        petal_pt_3[numb].x = pxx;
        petal_pt_3[numb].y = pyy;
        theta_3 = atanf((petal_pt_3[numb].y-center[numb].y+.0)/(petal_pt_3[numb].x-center[numb].x+.0));
        theta_4 = -1.2*pi + tmp;
        pxx = cosf(theta_4) * (petal_pt_4[numb].x-center[numb].x+.0) - sinf(theta_4) * (petal_pt_4[numb].y - center[numb].y+.0) + center[numb].x;
        pyy = sinf(theta_4) * (petal_pt_4[numb].x-center[numb].x+.0) + cosf(theta_4) * (petal_pt_4[numb].y - center[numb].y+.0) + center[numb].y;
        petal_pt_4[numb].x = pxx;
        petal_pt_4[numb].y = pyy;
        theta_4 = atanf((petal_pt_4[numb].y-center[numb].y+.0)/(petal_pt_4[numb].x-center[numb].x+.0));
        theta_5 = -1.6*pi + tmp;
        pxx = cosf(theta_5) * (petal_pt_5[numb].x-center[numb].x+.0) - sinf(theta_5) * (petal_pt_5[numb].y - center[numb].y+.0) + center[numb].x;
        pyy = sinf(theta_5) * (petal_pt_5[numb].x-center[numb].x+.0) + cosf(theta_5) * (petal_pt_5[numb].y - center[numb].y+.0) + center[numb].y;
        petal_pt_5[numb].x = pxx;
        petal_pt_5[numb].y = pyy;
        theta_5 = atanf((petal_pt_5[numb].y-center[numb].y+.0)/(petal_pt_5[numb].x-center[numb].x+.0));
//        qDebug()<<theta_1*180/pi;qDebug()<<theta_2*180/pi;qDebug()<<theta_3*180/pi;qDebug()<<theta_4*180/pi;qDebug()<<theta_5*180/pi;
        //avg angle
        float belta = 0;
        belta = (theta_1+theta_2+theta_3+theta_4+theta_5)/5.0;
//        qDebug()<<"belta";qDebug()<<belta;

        float dpq_1, dpq_2, dpq_3, dpq_4, dpq_5;
        dpq_1 = powf(alpha, 2)+ powf(d_1, 2) - 2*alpha*d_1*cos(theta_1-belta);
        dpq_2 = powf(alpha, 2)+ powf(d_2, 2) - 2*alpha*d_2*cos(theta_2-belta);
        dpq_3 = powf(alpha, 2)+ powf(d_3, 2) - 2*alpha*d_3*cos(theta_3-belta);
        dpq_4 = powf(alpha, 2)+ powf(d_4, 2) - 2*alpha*d_4*cos(theta_4-belta);
        dpq_5 = powf(alpha, 2)+ powf(d_5, 2) - 2*alpha*d_5*cos(theta_5-belta);
//        qDebug()<<dpq_1;qDebug()<<dpq_2;qDebug()<<dpq_3;qDebug()<<dpq_4;qDebug()<<dpq_5;qDebug()<<"------";
        float score;
        score = (dpq_1 + dpq_2 + dpq_3 + dpq_4 + dpq_5)/5.0;
        qDebug()<<"score";qDebug()<<score;
        qDebug()<<"------------";

        out << score;
        out << "\n";
    }
    out << "-------------\n";
    infile.close();
    outfile.close();
}


void MainWindow::on_horizontalSlider_4_valueChanged(int value)
{
    ruler_estimation();
}

void MainWindow::on_horizontalSlider_5_valueChanged(int value)
{
    ruler_estimation();
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ruler_estimation();
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    ruler_estimation();
}
