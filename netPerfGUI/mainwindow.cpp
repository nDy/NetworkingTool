#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ping = new QProcess();
    connect(ping,SIGNAL(readyReadStandardOutput()),this,SLOT(read()));
    connect(ping,SIGNAL(readyReadStandardError()),this,SLOT(readError()));

    connect(this,SIGNAL(stopPing()),ping,SLOT(terminate()));

    connect(ping,SIGNAL(started()),this,SLOT(startedProperly()));
    connect(ping,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finished(int,QProcess::ExitStatus)));
    connect(ping,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));
}

MainWindow::~MainWindow(){

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //ping with nerperf
    QStringList args;
    args << "tcp://192.168.0.108:7770" << "29" << "100" << "2";
    //QString program = "/home/jesus/qk";
    QString program = "/home/jesus/Documents/Redes/NetworkingTool/netPerf/netPerf";
    ping->start(program,args);

}

void MainWindow::read(){
    QString pingOutput = ping->readAllStandardOutput();
    qDebug()<< pingOutput;

}



void MainWindow::on_pushButton_2_clicked()
{
    emit stopPing();
}

void MainWindow::startedProperly(){
    qDebug()<<"The application started!";
}

void MainWindow::readError(){
    QString input = ping->readAllStandardError();

}

void MainWindow::finished(int exitCode, QProcess::ExitStatus status){
    qDebug()<<"process finished with exitCode"<<exitCode;
    if(exitCode==0){
        on_pushButton_clicked();
    }
}

void MainWindow::error(QProcess::ProcessError errorCode){
    if(errorCode==QProcess::FailedToStart){
        qDebug()<<"process failed to start!";
    }
}
