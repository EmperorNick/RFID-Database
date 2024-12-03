#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QProcess>

#include "mqttmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void connectToDatabase();

private slots:
    void openDatabaseDialog();
    void handleIncomingMessage(const QString &message, const QMqttTopicName &topic);
    void connectToMqttWithIpInput();
    void startRFIDPolling();
    void pollRFID();
    void handleRFIDOutput();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleRFIDError();

private:
    Ui::MainWindow *ui;
    void updateConnectionStatus(bool connected);
    void setupMqtt(); // Declare the setupMqtt method
    MqttManager *mqttManager;
    QProcess *rfidProcess;
};

#endif // MAINWINDOW_H
