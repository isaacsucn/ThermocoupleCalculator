#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../../src/thermocouple.h"
#include "QDebug"
#include "QClipboard"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_emf_result_value->setVisible(false);
    ui->label_temp_result_value->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_cal_temp_clicked()
{
    QString out_string;
    QString emf_string = ui->LineEdit_EMF_input->text();
    double emf=emf_string.toDouble();
    double temp;
    double cal_temp_range_low,cal_temp_range_high;
    double cal_emf_range_low,cal_emf_range_high;
    float error_range_low,error_range_high;
    int type;
    type= ui->comboBox_type->currentIndex();
    int rc;
    rc=TcEMFtoTwithRc((ThermocoupleType)type,emf,temp,error_range_low,error_range_high);
    if(rc==TC_CAL_SUCCESS )
    {
        out_string.sprintf("Tpye:%s,EMF=%lfmV,Temperature=%lf°C, \n error range %f~%f°C", ui->comboBox_type->currentText().toLocal8Bit().data()\
                           ,emf,temp,error_range_low,error_range_high);
        ui->label_temp_result->setText(out_string);
        ui->label_temp_result_value->setText(QString::number(temp));
        return;
    }

    if(rc==TC_CAL_OUT_OF_LOW_RANGE||rc==TC_CAL_OUT_OF_HIGH_RANGE)
    {
        rc=TcTypeRange((ThermocoupleType)type,cal_temp_range_low,cal_temp_range_high,cal_emf_range_low,cal_emf_range_high);
        out_string.sprintf("EMF is out of range, \nit should be in the range %lf~%lf mV for type ",cal_emf_range_low,cal_emf_range_high);
        out_string+=ui->comboBox_type->currentText();
        ui->label_temp_result->setText(out_string);
        return;
    }

    if(rc==TC_INVALID_TYPE)
    {
        out_string.sprintf("Invalid Type!");
        return;
    }
}

void MainWindow::on_pushButton_cal_EMF_clicked()
{
    QString out_string;
    QString temp_string = ui->LineEdit_temp_input->text();
    double temp=temp_string.toDouble();
    double emf;
    double cal_temp_range_low,cal_temp_range_high;
    double cal_emf_range_low,cal_emf_range_high;
    int type;
    type= ui->comboBox_type->currentIndex();
    int rc;
    rc=TcTtoEMFwithRc((ThermocoupleType)type,temp,emf);
    if(rc==TC_CAL_SUCCESS )
    {
        out_string.sprintf("Tpye:%s,Temperature=%lf°C,EMF=%lfmV", ui->comboBox_type->currentText().toLocal8Bit().data(),temp,emf);
        ui->label_emf_result->setText(out_string);
        ui->label_emf_result_value->setText(QString::number(emf));
        return;
    }

    if(rc==TC_CAL_OUT_OF_LOW_RANGE||rc==TC_CAL_OUT_OF_HIGH_RANGE)
    {
        rc=TcTypeRange((ThermocoupleType)type,cal_temp_range_low,cal_temp_range_high,cal_emf_range_low,cal_emf_range_high);
        out_string.sprintf("Temperature is out of range, \nit should be in the range of %lf~%lf °C for type ",cal_temp_range_low,cal_temp_range_high);
        out_string+=ui->comboBox_type->currentText();
        ui->label_emf_result->setText(out_string);
        return;
    }

    if(rc==TC_INVALID_TYPE)
    {
        out_string.sprintf("Invalid Type!");
        ui->label_emf_result->setText(out_string);
        return;
    }
}

void MainWindow::on_pushButton_clip_temp_clicked()
{
    QClipboard *board = QApplication::clipboard();
    QString text=ui->label_temp_result_value->text();
    board->setText(text);
}

void MainWindow::on_pushButton_clip_emf_clicked()
{
    QClipboard *board = QApplication::clipboard();
    QString text=ui->label_emf_result_value->text();
    board->setText(text);
}
