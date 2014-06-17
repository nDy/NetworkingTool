#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QVector>
#include <QTimer>

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

    Ui::MainWindow *ui;

    QProcess *ping;
    QProcess *iperf;

    QVector<double> pingVector;
    QVector<double> tpVector;
    QVector<double> timeVector;

    QTimer* timer;
    bool stopRequested;

    float sum;
    float maxPing;
    float maxTp;
    int numTests;

    void initPlot();
    void updatePlot();
    void startTest();
    void stopTest();
    void finishTest();

    int getInterval();

private slots:

    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_protocolField_currentIndexChanged(int index);

    void readPingOutput();
    void readIperfOutput();
    void readError();

    void processStarted();
    void pingFinished(int,QProcess::ExitStatus);
    void iperfFinished(int,QProcess::ExitStatus);
    void error(QProcess::ProcessError);

    void startPing();
    void startIperf();
};

#endif // MAINWINDOW_H
