#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void _update(QLabel*&, QLabel*&, QLabel*&);

    Ui::MainWindow *ui;
    QTimer* t;
    bool working;
    //zmq
    int msgCount;
    size_t msgSize;

    //init value;
    void* ctx;
    //return code
    int rc;
    //socket
    void *s;
    //watch
    void *watch;
    //elapsed time on zmq
    unsigned long elapsed;

public slots:
    void start();
    void update();
};

#endif // MAINWINDOW_H
