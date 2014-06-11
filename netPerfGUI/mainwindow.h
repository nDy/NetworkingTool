#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QVector>

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
    long maxPing;
    long maxAvgPing;
    int numTests;
    void initPlot();
    void updatePlot();
    void startPing();
    void startTp();

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

signals:
    void stopPing();


private:
    Ui::MainWindow *ui;
    QProcess *ping;
    QProcess *tp;
};

#endif // MAINWINDOW_H
