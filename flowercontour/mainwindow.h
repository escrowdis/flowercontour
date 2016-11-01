#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <CGrabCut.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:

    void reset();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void mousePressEvent(QMouseEvent *);

    void mouseMoveEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

    void on_horizontalSlider_sliderReleased();

    void on_horizontalSlider_2_sliderReleased();

    void line_judge();

    void overlapped_drawing();

    void slider_overlapping();

    void ruler_estimation();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_16_clicked();

    void on_horizontalSlider_4_valueChanged(int value);

    void on_horizontalSlider_5_valueChanged(int value);

    void on_spinBox_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_pushButton_17_clicked();

private:
    Ui::MainWindow *ui;
    Size size, zoom;
    Mat mask;

};

#endif // MAINWINDOW_H
