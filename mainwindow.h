#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <wiringPi.h>

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
    void handleRFIDOutput();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleRFIDError();
    void startRFIDPythonScript();

private:
    Ui::MainWindow *ui;
    void updateConnectionStatus(bool connected);
    void setupMqtt(); // Declare the setupMqtt method
    MqttManager *mqttManager;
    QProcess *rfidProcess;
    void redButton();
    void blueButton();
    void greenButton();
    void turnOffAllColors();
    void raveButton();

signals:
    void uidScanned(const QString &uid);  // Signal to emit scanned UID
};

#endif // MAINWINDOW_H
