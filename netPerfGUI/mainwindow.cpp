#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ping = new QProcess();
    tp = new QProcess();

    connect(ping,SIGNAL(readyReadStandardOutput()),this,SLOT(readPing()));
    connect(tp,SIGNAL(readyReadStandardOutput()),this,SLOT(readTp()));

    connect(ping,SIGNAL(readyReadStandardError()),this,SLOT(readError()));
    connect(tp,SIGNAL(readyReadStandardError()),this,SLOT(readError()));

    connect(this,SIGNAL(stopPing()),ping,SLOT(terminate())); //this should be fixed

    connect(ping,SIGNAL(started()),this,SLOT(startedProperly()));
    connect(tp,SIGNAL(started()),this,SLOT(startedProperly()));

    connect(ping,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finishedPing(int,QProcess::ExitStatus)));
    connect(tp,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finishedTp(int,QProcess::ExitStatus)));

    connect(ping,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));
    connect(tp,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));

    numTests = 0;
    maxPing = 0;
    maxAvgPing = 0;
    initPlot();
}

MainWindow::~MainWindow(){

    delete ui;
}

void MainWindow::startPing(){

    //ping with netPerf
    QStringList args;
    QString ip=ui->ipField->text();
    QString port=ui->portPingField->text();
    args << "tcp://"+ip+":"+port << "29" << "100" << "2";
    QString program = "/home/jesus/Workspace/NetworkingTool/netPerf/netPerf";
    ping->start(program,args);
}

void MainWindow::startTp(){
    //ping with netPerf
    QStringList args;
    QString ip=ui->ipField->text();
    QString port=ui->portTpField->text();
    args << "tcp://"+ip+":"+port << "29" << "100000" << "4";
    QString program = "/home/jesus/Workspace/NetworkingTool/netPerf/netPerf";
    tp->start(program,args);
}

void MainWindow::on_startButton_clicked()
{
    startPing();
    ui->startButton->setEnabled(false);
}

void MainWindow::readPing(){
    QString pingOutput = ping->readAllStandardOutput();
    qDebug()<< pingOutput;
    QStringList pingList = pingOutput.split("latency:");
    long sum = 0;
    for(int i=0;i<pingList.size();i++){
       long ping = pingList.at(i).toInt();
       if(ping != 0){
       qDebug()<<"ping seq="<<i<<": "<<ping;
       if(ping>maxPing)
           maxPing=ping;
       sum += ping;
       }
    }

    long prom = sum/pingList.size();
    qDebug()<<"Average ping this test: "<<prom;
    if(maxAvgPing<prom){
        maxAvgPing = prom;
    }
    avgPing.append(prom);
    testTime.append(numTests++);
    this->ui->pingLine->setText(QString::number(prom));
}

void MainWindow::readTp(){
    QString tpOutput = tp->readAllStandardOutput();
    qDebug()<<tpOutput;
    float tp = tpOutput.mid(11).toFloat();
    ui->tpLine->setText(QString::number(tp)); //I swear there is a good reason for this, you will see guys
}



void MainWindow::on_stopButton_clicked()
{
    emit stopPing();
}

void MainWindow::startedProperly(){
    qDebug()<<"The application started!";
}

void MainWindow::readError(){
    qDebug()<<ping->readAllStandardError();

}

void MainWindow::finishedPing(int exitCode, QProcess::ExitStatus status){
    qDebug()<<"process (ping) finished with exitCode"<<exitCode;
    if(exitCode==0){
        startTp();
    }
}

void MainWindow::finishedTp(int exitCode, QProcess::ExitStatus status){
    qDebug()<<"process (throughput) finished with exitCode"<<exitCode;
    if(exitCode==0){

    }
    //setup a timer to run the test again.
    //update graph
    updatePlot();
    //enable start
    ui->startButton->setEnabled(true);
}

void MainWindow::error(QProcess::ProcessError errorCode){
    if(errorCode==QProcess::FailedToStart){
        qDebug()<<"process failed to start!";
    }
}

void MainWindow::initPlot(){

    //label the axes
    ui->plotter->addGraph();
    ui->plotter->xAxis->setLabel("Test num");
    ui->plotter->yAxis->setLabel("Latency(us)");
    // set axes ranges
    ui->plotter->xAxis->setRange(0, 1); //increase as we make more tests
    ui->plotter->yAxis->setRange(0, 2000); //set max range as maximum ping!!!!
    ui->plotter->replot();
}

void MainWindow::updatePlot(){

    ui->plotter->graph(0)->setData(testTime,avgPing);

    ui->plotter->xAxis->setRange(0, numTests+1); //increase as we make more tests
    ui->plotter->yAxis->setRange(0, maxAvgPing); //set max range as maximum ping!!!!
    ui->plotter->replot();
}
