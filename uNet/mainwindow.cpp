#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <zmq.h>
#include <zmq_utils.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->t = new QTimer(this);
    connect(this->t, SIGNAL(timeout()), this, SLOT(update()));
    working = false;

    //zmq parameters init
    this->msgCount = 10;
    this->msgSize = 100;

    //zmq init
    ctx = zmq_init(1);
    if (!ctx) {
        std::cout<<"error in zmq_init: "<<zmq_strerror(errno)<<std::endl;
        this->close();
    }else{
        std::cout<<"zmq start successful"<<std::endl;
    }

    //zmq socket initialization
    s = zmq_socket(ctx, ZMQ_REP); //rep es para ping...ZMQ_PUSH es para tp
    if (!s) {
        std::cout<<"error in zmq_socket:"<<zmq_strerror(errno)<<std::endl;
        this->close();
    }else{
        std::cout<<"socket start successful"<<std::endl;
    }

}

MainWindow::~MainWindow()
{
    //zmq socket closure
    rc = zmq_close(s);
    if (rc != 0) {
        std::cout<<"error in zmq_close:"<<zmq_strerror(errno)<<std::endl;
    }else{
        std::cout<<"zmq socket exit successful"<<std::endl;
    }
    //zmq closure
    rc = zmq_term(ctx);
    if (rc != 0) {
        std::cout<<"error in zmq_term:"<< zmq_strerror(errno)<<std::endl;
    }else{
        std::cout<<"zmq exit successful"<<std::endl;
    }

    delete ui;
}

void MainWindow::start(){
    if (working){
        //deshabilitar timer
        std::cout<<"stop"<<std::endl;
        this->t->stop();
        working = false;

        //Actualizar texto del boton
        this->ui->startButton->setText("Start");

        //deshabilitar lineedit
        this->ui->IDlineEdit->setReadOnly(false);

        //close zmq connection
        elapsed = zmq_stopwatch_stop(watch);

    }else{
        //habilitar timer
        std::cout<<"start"<<std::endl;
        this->t->start(500);
        working = true;

        //Actualizar texto del boton
        this->ui->startButton->setText("Stop");

        //habilitar lineedit
        this->ui->IDlineEdit->setReadOnly(true);

        //start zmq connection
        rc = zmq_connect(s, this->ui->IDlineEdit->text().toStdString().c_str());
        if (rc != 0) {
            std::cout<<"error in zmq_connect:"<<zmq_strerror(errno)<<std::endl;
            this->close();
        }else{
            std::cout<<"connection successful"<<std::endl;
        }

        watch = zmq_stopwatch_start();
    }
}

void MainWindow::update(){
    //interfaz para update(labelPing,labelTP,labelPkgLoss)
    std::cout<<"update"<<std::endl;
    _update(this->ui->PingLabel,this->ui->TPlabel,this->ui->PLlabel);
}

void MainWindow::_update(QLabel*& Ping, QLabel*& TP, QLabel*& PkgLoss){

    //zmq update loop

    //message
    zmq_msg_t msg;

    rc = zmq_msg_init_size(&msg, msgSize);
    if (rc != 0) {
        std::cout<<"error in zmq_msg_init_size: "<< zmq_strerror(errno)<<std::endl;
    }else{
        std::cout<<"msg initializated with size "<<msgSize<<std::endl;
    }

    memset(zmq_msg_data(&msg), 0, msgSize);

    for (int i = 0; i != this->msgCount - 1; i++){
        rc = zmq_send(s, &msg,msgSize, 0);
        if (rc < 0) {
            std::cout<<"error in zmq_sendmsg:"<< zmq_strerror(errno)<<std::endl;
        }
        rc = zmq_recv(s, &msg,msgSize, 0);
        if (rc < 0) {
            std::cout<<"error in zmq_recvmsg:"<< zmq_strerror(errno)<<std::endl;
        }
        if (zmq_msg_size(&msg) != msgSize) {
            std::cout<<"message of incorrect size received"<<std::endl;
        }
    }

    //closemsg
    rc = zmq_msg_close(&msg);
    if (rc != 0) {
        printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
    }

    //print stats

    Ping->setText("ping");
    TP->setText("TP");
    PkgLoss->setText("PL");
}
