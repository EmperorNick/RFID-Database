#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasedialog.h"
#include "mqttmanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QProcess>
#include <QTimer>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mqttManager(new MqttManager("", 1883, this))
    , rfidProcess(new QProcess(this))  // Initialize QProcess
{
    ui->setupUi(this);
    setupMqtt();  // Set up MQTT connections

    mqttManager->publishDatabaseEntry("test.db", QByteArray("Test message"));

    QString topic = "test.db";
    QString data = "<Formatted database data>";  // Fetch and format database content
    mqttManager->publishDatabaseEntry(topic, data);

/***************************************RFID START***********************************************************************/

    // Connect the process signals for output handling
    connect(rfidProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::handleRFIDOutput);
    connect(rfidProcess, &QProcess::readyReadStandardError, this, &MainWindow::handleRFIDError);
    connect(rfidProcess, &QProcess::finished, this, &MainWindow::handleProcessFinished);

    // Start the RFID scanning
    startRFIDPolling();

/***************************************RFID END*************************************************************************/

    // Connect UI buttons
    connect(ui->testbutton, &QPushButton::clicked, this, &MainWindow::connectToMqttWithIpInput);
    connect(ui->exitbutton, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->openDatabaseButton, &QPushButton::clicked, this, &MainWindow::openDatabaseDialog);

    // Monitor MQTT connection state and update the UI
    connect(mqttManager->getClient(), &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state) {
    updateConnectionStatus(state == QMqttClient::Connected);
    setFocus();
    });
}

MainWindow::~MainWindow() {
    delete mqttManager;  // Clean up the mqttManager
    delete ui;
    if (rfidProcess->state() == QProcess::Running) {
        rfidProcess->terminate();
        if (!rfidProcess->waitForFinished(3000)) {  // Wait up to 3 seconds
            rfidProcess->kill();  // Force kill if necessary
        }
    }
    delete rfidProcess;
    delete ui;
}


void MainWindow::connectToDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");

    if (!db.open()) {
        qDebug() << "Error: Unable to open SQLite database." << db.lastError().text();
        ui->statuslabel->setText("Disconnected from SQLite");
        ui->statuslabel->setStyleSheet("color: red;");
    } else {
        qDebug() << "Successfully connected to the SQLite database!";
        ui->statuslabel->setText("Connected to SQLite");
        ui->statuslabel->setStyleSheet("color: green;");
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS RFIDData ("
               "batch TEXT PRIMARY KEY, "  // UID as PRIMARY KEY
               "wafers INTEGER NOT NULL, "
               "process TEXT NOT NULL, "
               "location TEXT)");
}

void MainWindow::openDatabaseDialog() {
    DatabaseDialog *dbDialog = new DatabaseDialog(this);
    dbDialog->displayDatabaseContents();  // Show current entries
    dbDialog->exec();  // Show the dialog modally
}

void MainWindow::setupMqtt() {
    connect(mqttManager, &MqttManager::messageReceived, this, &MainWindow::handleIncomingMessage);
    connect(mqttManager->getClient(), &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state) {
        updateConnectionStatus(state == QMqttClient::Connected);
    });

    mqttManager->connectToBroker();
}

void MainWindow::handleIncomingMessage(const QString &message, const QMqttTopicName &topic) {
    qDebug() << "Received message:" << message << "on topic:" << topic.name();
    ui->messageLabel->setText(message);  // Update messageLabel with new message
}

void MainWindow::updateConnectionStatus(bool connected) {
    if (connected) {
        ui->mqttLabel->setText("Connected to MQTT");
        ui->mqttLabel->setStyleSheet("color: green;");
    } else {
        ui->mqttLabel->setText("Disconnected from MQTT");
        ui->mqttLabel->setStyleSheet("color: red;");
    }
}

void MainWindow::connectToMqttWithIpInput() {
    bool ok;
    QString ip = QInputDialog::getText(this, tr("MQTT Broker IP"),
                                       tr("Enter MQTT Broker IP:"), QLineEdit::Normal,
                                       "192.168", &ok);
    if (ok && !ip.isEmpty()) {
        mqttManager->getClient()->setHostname(ip); // Update MQTT client hostname
        mqttManager->connectToBroker(); // Connect to the new broker IP
    }
}

void MainWindow::startRFIDPolling() {
    if (rfidProcess->state() == QProcess::NotRunning) {
        rfidProcess->start("python3", QStringList() << "/home/nick/Downloads/RFID-Database/RFIDScan.py");
        if (!rfidProcess->waitForStarted()) {
            qDebug() << "Failed to start RFID scanning process.";
        } else {
            qDebug() << "RFID scanning process started successfully.";
        }
    }
}

void MainWindow::pollRFID() {
    QProcess process;
    process.start("python", QStringList() << "/home/nick/Downloads/RFID-Database/RFIDScan.py");
    process.waitForFinished();

    QString output = process.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        ui->messageLabel->setText("Tag UID: " + output);  // Update the QLabel with the UID
        qDebug() << "Detected RFID Tag UID:" << output;
    }
}

void MainWindow::handleRFIDOutput() {
    QString output = rfidProcess->readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        ui->messageLabel->setText("Tag UID: " + output);
        emit uidScanned(output);  // Emit UID to other parts of the program
        qDebug() << "Detected RFID Tag UID:" << output;
    }
}

void MainWindow::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "RFID process finished with code" << exitCode << "and status" << exitStatus;
    // Optionally restart the process if it unexpectedly stops
    if (exitStatus == QProcess::CrashExit) {
        qDebug() << "RFID process crashed. Restarting...";
        startRFIDPolling();
    }
}

void MainWindow::handleRFIDError() {
    QString error = rfidProcess->readAllStandardError().trimmed();
    if (!error.isEmpty()) {
        // Print the error to the debug console
        qDebug() << "RFID scanning process error:" << error;
    }
}

void MainWindow::startRFIDPythonScript() {
    // Create a QProcess instance
    QProcess *process = new QProcess(this);

    // Set the program and arguments
    process->setProgram("python3");
    process->setArguments({"/path/to/RFIDScan.py"});

    // Connect to QProcess signals to capture output
    connect(process, &QProcess::readyReadStandardOutput, [process, this]() {
        QString output = process->readAllStandardOutput();
        qDebug() << "Python Output:" << output; // Log output to the console
        // Optional: append to a QTextEdit or QLabel in the UI
        ui->textEdit->append(output);
    });

    connect(process, &QProcess::readyReadStandardError, [process, this]() {
        QString error = process->readAllStandardError();
        qDebug() << "Python Error:" << error; // Log errors to the console
        // Optional: append to a QTextEdit or QLabel in the UI
        ui->textEdit->append(error);
    });

    // Start the process and check if it starts successfully
    process->start();
    if (!process->waitForStarted()) {
        qDebug() << "Failed to start Python script.";
        ui->textEdit->append("Failed to start Python script.");
    }
}
