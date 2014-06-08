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
    this->msgCount = 100;
    this->msgSize = 29;

    //zmq init
    ctx = zmq_init(1);
    if (!ctx) {
        std::cout<<"error in zmq_init: "<<zmq_strerror(errno)<<std::endl;
        this->close();
    }else{
        std::cout<<"zmq start successful"<<std::endl;
    }

}

MainWindow::~MainWindow()
{
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

    }else{
        //habilitar timer
        std::cout<<"start"<<std::endl;
        this->t->start(500);
        working = true;

        //Actualizar texto del boton
        this->ui->startButton->setText("Stop");

        //habilitar lineedit
        this->ui->IDlineEdit->setReadOnly(true);
    }
}

void MainWindow::update(){
    //interfaz para update(labelPing,labelTP,labelPkgLoss)
    std::cout<<"update"<<std::endl;
    //funcion timer
    _update();

    //print stats

}

void MainWindow::_update(){

    //zmq update loop

    //EXPERIMENTOS

    //Ping calc
    //zmq socket initialization
    s = zmq_socket(ctx, ZMQ_PUSH); //rep es para ping...ZMQ_PUSH es para tp
    if (!s) {
        std::cout<<"error in ping zmq_socket:"<<zmq_strerror(errno)<<std::endl;
        this->close();
    }else{
        std::cout<<"ping socket start successful"<<std::endl;
    }



    //experimento

    std::cout<< this->ui->IDlineEdit->text().toStdString().c_str() <<std::endl;

    rc = zmq_connect(s, this->ui->IDlineEdit->text().toStdString().c_str());
    if (rc != 0) {
        printf("error in zmq_connect: %s\n", zmq_strerror(errno));
    }
    std::cout<<"conexion realizada"<<std::endl;

    //inicializacion de mensaje
    zmq_msg_t msg;
    rc = zmq_msg_init_size(&msg, this->msgSize);
    if (rc != 0) {
        printf("error in zmq_msg_init_size: %s\n", zmq_strerror(errno));
    }
    memset(zmq_msg_data(&msg), 0, this->msgSize);
    std::cout<<"mensaje inicializado"<<std::endl;

    unsigned long total;
    total = 0;

    for (int i = 0; i != this->msgCount; i++) {
        std::cout<<"i vale "<<i<<std::endl;
        watch = zmq_stopwatch_start();
        rc = zmq_send(s, &msg,this->msgSize, 0);
        if (rc < 0) {
            printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
        }
        std::cout<<"mensaje enviado"<<std::endl;
        rc = zmq_recv(s, &msg,this->msgSize, 0);
        if (rc < 0) {
            printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
        }
        std::cout<<"mensaje recibido"<<std::endl;
        if (zmq_msg_size(&msg) != this->msgSize) {
            printf("message of incorrect size received\n");
        }
        std::cout<<"mensaje analizado"<<std::endl;
        elapsed = zmq_stopwatch_stop(watch);
        //update ping
        std::cout<< "Ping"<< elapsed <<std::endl;
        total += elapsed;
    }

    unsigned long throughput;
    double megabits;
    throughput = (unsigned long) ((double) this->msgCount / (double) total
                                  * 1000000);
    megabits = (double) (throughput * this->msgSize * 8) / 1000000;

    printf("message size: %d [B]\n", (int) this->msgSize);
    printf("message count: %d\n", (int) this->msgCount);
    printf("mean throughput: %d [msg/s]\n", (int) throughput);
    printf("mean throughput: %.3f [Mb/s]\n", (double) megabits);

    rc = zmq_msg_close(&msg);
    if (rc != 0) {
        printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
    }


    //zmq socket closure
    rc = zmq_close(s);
    if (rc != 0) {
        std::cout<<"error in ping zmq_close:"<<zmq_strerror(errno)<<std::endl;
    }else{
        std::cout<<"ping zmq socket exit successful"<<std::endl;
    }
}
