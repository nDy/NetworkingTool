#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QDebug>
#include <iostream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ping = new QProcess();
    iperf = new QProcess();
    timer = new QTimer();
    timer->setSingleShot(true);

    numTests = 0;

    ui->udpBandwidthField->hide();

    connect(ping,SIGNAL(readyReadStandardOutput()),this,SLOT(readPingOutput()));
    connect(iperf,SIGNAL(readyReadStandardOutput()),this,SLOT(readIperfOutput()));

    connect(ping,SIGNAL(readyReadStandardError()),this,SLOT(readError()));
    connect(iperf,SIGNAL(readyReadStandardError()),this,SLOT(readError()));

    connect(ping,SIGNAL(started()),this,SLOT(processStarted()));
    connect(iperf,SIGNAL(started()),this,SLOT(processStarted()));

    connect(ping,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(pingFinished(int,QProcess::ExitStatus)));
    connect(iperf,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(iperfFinished(int,QProcess::ExitStatus)));

    connect(ping,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));
    connect(iperf,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));

    connect(timer,SIGNAL(timeout()),this,SLOT(startPing()));

    initPlot();
}

MainWindow::~MainWindow(){

    delete ui;
}

void MainWindow::startPing(){

    qDebug()<<"starting test:"<<numTests+1;
    ui->statusLabel->setText("TESTING");
    //ping
    QStringList args;
    QString ip=ui->ipField->text();
    args << ip << "-i0.2" << "-c10";
    QString program = "ping";
    sum = 0;
    ping->start(program,args);
}


void MainWindow::startIperf(){
    //iperf test
    QStringList args;
    QString ip=ui->ipField->text();
    QString port=ui->portField->text();

    //add arguments to iperf
    args << "-c" << ip <<"-p"<< port<< "-f"<<"m";
    if(ui->protocolField->currentIndex()==1){
        args << "-u" << "-b" << QString::number(ui->udpBandwidthField->value())+'m';
    }
    QString program = "iperf";
    iperf->start(program,args);
}

void MainWindow::stopTest(){
    ui->ipField->setEnabled(true);
    ui->protocolField->setEnabled(true);
    ui->udpBandwidthField->setEnabled(true);
    ui->intervalField->setEnabled(true);
    ui->portField->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->statusLabel->setText("STOPPED");
}

void MainWindow::startTest(){
    ui->ipField->setEnabled(false);
    ui->protocolField->setEnabled(false);
    ui->udpBandwidthField->setEnabled(false);
    ui->intervalField->setEnabled(false);
    ui->portField->setEnabled(false);

    maxPing = 0;
    maxTp = 0;
    sum = 0;
    stopRequested = false;
}


void MainWindow::on_startButton_clicked()
{
    if(ping->state()==QProcess::Running||iperf->state()==QProcess::Running){
        if(ping->state()==QProcess::Running)
            qDebug()<<"ping still running";
        if(iperf->state()==QProcess::Running)
                qDebug()<<"tp still running";
        return;
    }

    startTest();
    startPing();
    ui->startButton->setEnabled(false);
}

void MainWindow::readPingOutput(){
    qDebug()<<"reading ping output:";
    QString pingOutput = ping->readAllStandardOutput();
    qDebug()<< pingOutput;
    int startIndex = pingOutput.indexOf("time=") + 5;
    if(startIndex==-1){
        return;
    }
    int endIndex = pingOutput.indexOf(" ms");
    double ping = pingOutput.mid(startIndex,endIndex-startIndex).toDouble();
    sum += ping;
}

void MainWindow::readIperfOutput(){
    qDebug()<<"reading tp output:";
    QString tpOutput = iperf->readAllStandardOutput();
    qDebug()<<tpOutput;
    float tp=0;
    if(ui->protocolField->currentIndex()==0){ //tcp test
        if(tpOutput.indexOf("Bandwidth")!=-1){
            QStringList tokens = tpOutput.split(" ");
            for(int i=0;i<tokens.size();i++){
                if(tokens.at(i).contains("/sec")){
                    ui->tpLine->setText(tokens.at(i-1)+" "+tokens.at(i));
                    tp = tokens.at(i-1).toFloat();
                }
            }

            tpVector.append(tp);
        }
    }else{ //udp test
        if(tpOutput.indexOf("Server Report:")!=-1){
            QStringList tokens = tpOutput.split(" ");
            //14 is TP
            //15 is TP unit
            //18 is Jitter
            //19 is Jitter unit
            //lost/send DG 21,22
            for(int i=0;i<tokens.size();i++){
                if(tokens.at(i).contains("/sec")){
                    tp = tokens.at(i-1).toFloat();
                    tpVector.append(tp);
                    ui->tpLine->setText(tokens.at(i-1)+" "+tokens.at(i));
                }if(tokens.at(i).contains("ms")&&(!tokens.at(i).contains("datagrams"))){
                    ui->jitterLine->setText(tokens.at(i-1)+" "+tokens.at(i));
                }
                if(tokens.at(i).contains("%")){
                    QString aux;
                    aux =tokens.at(i-2);
                    aux.chop(1);
                    float lost = aux.toFloat();
                    std::cout<<aux.toStdString()<<"lost"<<std::endl;
                    float sent = tokens.at(i-1).toFloat();
                    float percentage;
                    if(sent=!0){
                        percentage = lost/sent;
                    }else{
                        percentage = 100.0;
                    }
                    std::cout<<tokens.at(i-1).toStdString()<<"sent"<<std::endl;
                    ui->datagramLossLine->setText(tokens.at(i-2)+tokens.at(i-1)+"  ("+QString::number(percentage,'f',2)+"%)");
                }
            }

        }
    }
    if(tp>maxTp){
        maxTp=tp;
    }
}



void MainWindow::on_stopButton_clicked()
{
    if(timer->isActive()){
        timer->stop();
        stopTest();
    }else{
        ui->statusLabel->setText("STOPPING...");
        stopRequested = true;
    }
}

void MainWindow::processStarted(){
    qDebug()<<"The application started!";
}

void MainWindow::readError(){
    qDebug()<<"reading stderr";
    QMessageBox *message = new QMessageBox(this);
    message->setWindowTitle("Error");
    message->setText(ping->readAllStandardError()+iperf->readAllStandardError());

}

void MainWindow::pingFinished(int exitCode, QProcess::ExitStatus status){
    qDebug()<<"process (ping) finished with exitCode"<<exitCode;
    if(exitCode==0){
        double prom = sum/10;
        qDebug()<<"Average ping this test: "<<prom;
        if(maxPing<prom){
            maxPing = prom;
        }
        pingVector.append(prom);
        timeVector.append(numTests++);
        this->ui->pingLine->setText(QString::number(prom)+" ms");
    }else{
        //client isn't online error.
        qDebug()<<"host is not present or is bloquing ping";
    }
    updatePlot();
    startIperf();

}

void MainWindow::iperfFinished(int exitCode, QProcess::ExitStatus status){
    qDebug()<<"process (throughput) finished with exitCode"<<exitCode;
    //update graph
    updatePlot();
    if(stopRequested){
        //enable start
        stopRequested = false;
        stopTest();
    }else{ //setup a timer to run the test again with given interval.
        timer->start(getInterval());
        ui->statusLabel->setText("WAITING");
    }
    qDebug()<<"test finished properly";
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
    ui->plotter->yAxis->setRange(0, 2000); //set max range as maximum avg ping!!!!
    ui->plotter->replot();

    ui->tpPlotter->addGraph();
    ui->tpPlotter->xAxis->setLabel("Test num");
    ui->tpPlotter->yAxis->setLabel("Throughput(mb/s)");
    // set axes ranges
    ui->tpPlotter->xAxis->setRange(0, 1); //increase as we make more tests
    ui->tpPlotter->yAxis->setRange(0, maxTp); //set max range as 10 because that's the wlan0 iface limit
    ui->tpPlotter->replot();
}

void MainWindow::updatePlot(){

    ui->plotter->graph(0)->setData(timeVector,pingVector);

    ui->plotter->xAxis->setRange(0, numTests+1); //increase as we make more tests
    ui->plotter->yAxis->setRange(0, maxPing); //set max range as maximum ping!!!!
    ui->plotter->replot();

    ui->tpPlotter->graph(0)->setData(timeVector,tpVector);

    ui->tpPlotter->xAxis->setRange(0, numTests+1); //increase as we make more tests
    ui->tpPlotter->yAxis->setRange(0, maxTp);
    ui->tpPlotter->replot();
}

int MainWindow::getInterval()
{
    int interval = ui->intervalField->time().second() +
            ui->intervalField->time().minute()*60 +
            ui->intervalField->time().hour()*3600;
    interval = interval*1000;
    qDebug()<<"interval between tests is: "<<interval<<"(ms)";
    return interval;
}

void MainWindow::on_protocolField_currentIndexChanged(int index)
{
    if(index==1){
        ui->udpBandwidthField->show();
    }else{
        ui->udpBandwidthField->hide();
    }
}
