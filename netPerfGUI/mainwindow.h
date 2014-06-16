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
    QVector<double> avgPing;
    QVector<double> tpList;
    QVector<double> testTime;

    QTimer* timer;
    bool stopRequested;

    long maxPing;
    long maxAvgPing;
    int numTests;

    void initPlot();
    void updatePlot();
    void startTest();
    int getInterval();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();

    void readPing();
    void readTp();
    void readError();

    void startedProperly();
    void finishedPing(int,QProcess::ExitStatus);
    void finishedTp(int,QProcess::ExitStatus);
    void error(QProcess::ProcessError);

    void on_protocolField_currentIndexChanged(int index);

    void startPing();
    void startTp();

signals:
    void stopPing();
    void startTpSignal();



private:
    Ui::MainWindow *ui;
    QProcess *ping;
    QProcess *tp;
};

#endif // MAINWINDOW_H
