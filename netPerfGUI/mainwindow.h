#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_pushButton_clicked();
    void readPing();
    void readTp();
    void readError();
    void on_pushButton_2_clicked();

    void startPing();
    void startTp();

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
