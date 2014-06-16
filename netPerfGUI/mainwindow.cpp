#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QDebug>
double sum;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ping = new QProcess();
    tp = new QProcess();
    timer = new QTimer();
    numTests = 0;
    maxPing = 0;
    maxAvgPing = 0;
    stopRequested = false;

    connect(ping,SIGNAL(readyReadStandardOutput()),this,SLOT(readPing()));
    connect(tp,SIGNAL(readyReadStandardOutput()),this,SLOT(readTp()));

    connect(ping,SIGNAL(readyReadStandardError()),this,SLOT(readError()));
    connect(tp,SIGNAL(readyReadStandardError()),this,SLOT(readError()));

    connect(this,SIGNAL(stopPing()),ping,SLOT(terminate())); //this should be fixed
    connect(this,SIGNAL(startTpSignal()),this,SLOT(startTp()));

    connect(ping,SIGNAL(started()),this,SLOT(startedProperly()));
    connect(tp,SIGNAL(started()),this,SLOT(startedProperly()));

    connect(ping,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finishedPing(int,QProcess::ExitStatus)));
    connect(tp,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finishedTp(int,QProcess::ExitStatus)));

    connect(ping,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));
    connect(tp,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));

    connect(timer,SIGNAL(timeout()),this,SLOT(on_startButton_clicked()));

    ui->udpBandwidthField->hide();
    initPlot();
}

MainWindow::~MainWindow(){

    delete ui;
}

void MainWindow::startPing(){

    qDebug()<<"starting test:"<<numTests+1;
    //ping
    QStringList args;
    QString ip=ui->ipField->text();
    args << ip << "-i0.2" << "-c10";
    QString program = "ping";
    sum = 0;
    ping->start(program,args);
}


void MainWindow::startTp(){
    //iperf test
    QStringList args;
    QString ip=ui->ipField->text();
    QString port=ui->portField->text();

    //add arguments to iperf
    args << "-c" << ip <<"-p"<< port;
    if(ui->protocolField->currentIndex()==1){
        args << "-u" << "-b" << QString::number(ui->udpBandwidthField->value())+'m';
    }
    QString program = "iperf";
    tp->start(program,args);
}

void MainWindow::startTest(){
    ui->ipField->setEnabled(false);
    ui->protocolField->setEnabled(false);
    ui->udpBandwidthField->setEnabled(false);
    ui->intervalField->setEnabled(false);
    ui->portField->setEnabled(false);
}


void MainWindow::on_startButton_clicked()
{
    qDebug()<<"on start button clicked was called for some reason";
    if(ping->state()==QProcess::Running||tp->state()==QProcess::Running){
        if(ping->state()==QProcess::Running)
            qDebug()<<"ping still running";
        if(tp->state()==QProcess::Running)
                qDebug()<<"tp still running";
        return;
    }

    startTest();
    startPing();
    ui->startButton->setEnabled(false);
}

void MainWindow::readPing(){
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

void MainWindow::readTp(){
    qDebug()<<"reading tp output:";
    QString tpOutput = tp->readAllStandardOutput();
    qDebug()<<tpOutput;
    if(ui->protocolField->currentIndex()==0){ //tcp test
        if(tpOutput.indexOf("Mbits/sec")!=-1){
            QStringList tokens = tpOutput.split(" ");
            ui->tpLine->setText(tokens.at(24)+" "+tokens.at(25));
            float tp = tokens.at(24).toFloat();
            tpList.append(tp);
        }
    }else{ //udp test
        if(tpOutput.indexOf("Server Report:")!=-1){
            QStringList tokens = tpOutput.split(" ");
            //14 is TP
            //15 is TP unit
            //18 is Jitter
            //19 is Jitter unit
            //lost/send DG 21,22
            float tp = tokens.at(14).toFloat();
            tpList.append(tp);
            ui->tpLine->setText(tokens.at(14)+" "+tokens.at(15));
            ui->jitterLine->setText(tokens.at(18)+" "+tokens.at(19));
            QString aux;
            aux =tokens.at(21);
            aux.chop(1);
            float lost = aux.toFloat();
            float sent = tokens.at(22).toFloat();
            float percentage = lost/sent;
            ui->datagramLossLine->setText(tokens.at(21)+tokens.at(22)+"  ("+QString::number(percentage,'f',2)+"%)");
        }
    }
}



void MainWindow::on_stopButton_clicked()
{
    if(timer->isActive()){
        timer->stop();
        ui->startButton->setEnabled(true);
    }else{
        stopRequested = true;
    }
}

void MainWindow::startedProperly(){
    qDebug()<<"The application started!";
}

void MainWindow::readError(){
    qDebug()<<"reading stderr";
    qDebug()<<ping->readAllStandardError();
    qDebug()<<tp->readAllStandardError();

}

void MainWindow::finishedPing(int exitCode, QProcess::ExitStatus status){    
    qDebug()<<"process (ping) finished with exitCode"<<exitCode;
    if(exitCode==0){
        double prom = sum/10;
        qDebug()<<"Average ping this test: "<<prom;
        if(maxAvgPing<prom){
            maxAvgPing = prom;
        }
        avgPing.append(prom);
        testTime.append(numTests++);
        this->ui->pingLine->setText(QString::number(prom)+" ms");
    }else{
        //client isn't online error.
        qDebug()<<"host is not present or is bloquing ping";
    }
    emit startTp();

}

void MainWindow::finishedTp(int exitCode, QProcess::ExitStatus status){
    qDebug()<<"process (throughput) finished with exitCode"<<exitCode;
    //update graph
    updatePlot();
    if(stopRequested){
        //enable start
        ui->startButton->setEnabled(true);
        stopRequested = false;
    }else{ //setup a timer to run the test again with given interval.
        timer->setSingleShot(true);
        timer->start(getInterval());
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
    ui->tpPlotter->yAxis->setRange(0, 10); //set max range as 10 because that's the wlan0 iface limit
    ui->tpPlotter->replot();

}

void MainWindow::updatePlot(){

    ui->plotter->graph(0)->setData(testTime,avgPing);

    ui->plotter->xAxis->setRange(0, numTests+1); //increase as we make more tests
    ui->plotter->yAxis->setRange(0, maxAvgPing); //set max range as maximum ping!!!!
    ui->plotter->replot();

    ui->tpPlotter->graph(0)->setData(testTime,tpList);

    ui->tpPlotter->xAxis->setRange(0, numTests+1); //increase as we make more tests
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
