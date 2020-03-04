#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_cal_temp_clicked();

    void on_pushButton_cal_EMF_clicked();

    void on_pushButton_clip_temp_clicked();

    void on_pushButton_clip_emf_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
