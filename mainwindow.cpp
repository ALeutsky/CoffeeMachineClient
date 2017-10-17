#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&timer, SIGNAL(timeout()), this, SLOT(connectToMachine()));
    connect(&port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    timer.start(3000);
    statusOffline();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::controlsEnable(bool enabled)
{
    ui->btn1Cup->setEnabled(enabled);
    ui->btn2Cup->setEnabled(enabled);
    ui->btn3Cup->setEnabled(enabled);
    ui->btnTea->setEnabled(enabled);
    ui->btnCoffee->setEnabled(enabled);
    ui->btnSCoffee->setEnabled(enabled);
    ui->btnDo->setEnabled(enabled);
}

void MainWindow::parseResponce(QString response)
{
    response = response.trimmed();

    qDebug() << "RES:" << response;

    lastResTime = QDateTime::currentDateTime();

    QStringList chunks = response.split(",");
    if (chunks[0] == "$STATE") {
        if (chunks[1] == "WAITING" && status != StatusWaiting) {
            statusWaiting();
        } else if (chunks[1] == "PROCESSING" && status != StatusProcessing) {
            statusProcessing();
        }
        ui->waterLevel->setValue(chunks[2].toInt());
    } else if (chunks[0] == "$READY") {
        if (status == StatusProcessing) {
            statusWaiting();
        }
        QMessageBox::information(this, "Info", "Your drink is ready.\nGo to the kitchen!");
    } else if (chunks[0] == "$ERROR") {
        QMessageBox::critical(this, "Error", chunks[1]);
    }
}

void MainWindow::on_btnDo_clicked()
{
    QString cmd = "$DO";
    QString cups;

    if (ui->btn1Cup->isChecked()) {
        cups = "1";
    } else if (ui->btn2Cup->isChecked()) {
        cups = "2";
    } else if (ui->btn3Cup->isChecked()) {
        cups = "3";
    }

    if (ui->btnTea->isChecked()) {
        cmd += ",TEA," + cups + ",1";
    } else if (ui->btnCoffee->isChecked()) {
        cmd += ",COFFEE," + cups + ",1";
    } else if (ui->btnSCoffee->isChecked()) {
        cmd += ",COFFEE," + cups + ",2";
    }

    if (port.isOpen()) {
        sendCmd(cmd);
        statusProcessing();
    }
}

void MainWindow::connectToMachine()
{
    if (port.isOpen()) {
        if (lastResTime.secsTo(QDateTime::currentDateTime()) > 6) {
            port.close();
            //statusOffline();
            //return;
        } else {
            sendCmd("$STATE");
            return;
        }
    }

    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();

    statusConnecting();

    int i;
    QString portName;
    for (i = 0; i < list.length(); i++) {
        portName = list[i].portName();

        qDebug() << "DEV:" << portName;

        if (portName.contains("A9UXTR7B") || portName.contains("HC-06")) {
            port.setPort(list[i]);
            port.setBaudRate(QSerialPort::Baud9600);
            port.open(QSerialPort::ReadWrite);
            break;
        }
    }

    if (port.isOpen()) {
        lastResTime = QDateTime::currentDateTime();
        statusOnline();
    } else {
        statusOffline();
    }
}

void MainWindow::onReadyRead()
{
    QByteArray input;

    if (port.canReadLine()) {
        input = port.readLine();
        parseResponce(input);
    }
}

void MainWindow::statusOnline()
{
    this->controlsEnable(true);
    ui->statusBar->showMessage("Online");
    ui->btnDo->setText("DO");
    status = StatusOnline;
}

void MainWindow::statusOffline()
{
    this->controlsEnable(false);
    ui->statusBar->showMessage("Offline");
    ui->btnDo->setText("DO");
    status = StatusOffline;
    ui->waterLevel->setValue(0);
}

void MainWindow::statusProcessing()
{
    this->controlsEnable(false);
    ui->btnDo->setText("processing...");
    status = StatusProcessing;
}

void MainWindow::statusWaiting()
{
    this->controlsEnable(true);
    ui->btnDo->setText("DO");
    status = StatusWaiting;
}

void MainWindow::statusConnecting()
{
    this->controlsEnable(false);
    ui->btnDo->setText("DO");
    ui->statusBar->showMessage("Connecting...");
    status = StatusConnecting;
}

void MainWindow::sendCmd(QString cmd)
{
    qDebug() << "CMD:" << cmd;
    cmd += "\n";
    port.write(cmd.toLatin1());
}
