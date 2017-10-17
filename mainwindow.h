#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void controlsEnable(bool enabled);
    void parseResponce(QString response);

private slots:
    void on_btnDo_clicked();
    void connectToMachine();
    void onReadyRead();

    void statusOnline();
    void statusOffline();
    void statusProcessing();
    void statusWaiting();
    void statusConnecting();


private:
    Ui::MainWindow *ui;

    QTimer timer;
    QSerialPort port;
    QDateTime lastResTime;

    void sendCmd(QString cmd);

    enum Statuses {
        StatusOnline,
        StatusOffline,
        StatusWaiting,
        StatusProcessing,
        StatusConnecting
    };

    Statuses status;
};

#endif // MAINWINDOW_H
