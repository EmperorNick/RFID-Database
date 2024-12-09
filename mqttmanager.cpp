#include "mqttmanager.h"
#include <QDebug>
#include <QtMqtt/QMqttClient>
#include <QSqlQuery>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

MqttManager::MqttManager(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), client(new QMqttClient(this)), publishTimer(new QTimer(this)) {
    client->setHostname(host);
    client->setPort(port);

    connect(publishTimer, &QTimer::timeout, this, &MqttManager::publishDatabaseData);
    connect(client, &QMqttClient::messageReceived, this, &MqttManager::onMessageReceived);
    connect(client, &QMqttClient::connected, this, &MqttManager::onConnected);
    connect(client, &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state) { //Shows status of MQTT on the GUI
        QString status;
        switch (state) {
        case QMqttClient::Connected:
            status = "Connected";
            break;
        case QMqttClient::Connecting:
            status = "Connecting...";
            break;
        case QMqttClient::Disconnected:
            status = "Disconnected";
            break;
        default:
            break;
        }
        emit statusChanged(status);
    });
}

void MqttManager::connectToBroker() {  //Simple function to connect to the MQTT Broker
    client->connectToHost();
}

void MqttManager::publishMessage(const QString &topic, const QString &message) { //Function that can be called to publish info
    client->publish(topic, message.toUtf8());
}

void MqttManager::subscribeToTopic(const QString &topic) { //Debug to show if subscribe worked
    auto result = client->subscribe(topic);
    if (result) {
        qDebug() << "Subscribed to topic:" << topic;
    } else {
        qDebug() << "Failed to subscribe to topic:" << topic;
    }
}

void MqttManager::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic) { //Lets me know it has recieved information from a subscribe
    QString receivedData = QString::fromUtf8(message);
    QString receivedTopic = topic.name();
    emit logMessage(QString("Received data on topic '%1': %2").arg(receivedTopic, receivedData));
    handleSubscribedData(receivedTopic, message);
}

void MqttManager::onConnected() { //Debug function used to print that MQTT successfully connected and subscribes to test/update
    qDebug() << "Connected to MQTT broker";
    subscribeToTopic("test/update");
}

void MqttManager::startPeriodicPublishing() { //Seperate function to control the publish timer, 60000 is 60 seconds
    publishTimer->start(60000);
}

void MqttManager::publishDatabaseData() { //Function to query the database to get the initial data
    QSqlQuery query("SELECT batch, wafers, process, location FROM RFIDData");
    QJsonArray dataArray;

    while (query.next()) {
        QJsonObject record;
        record["batch"] = query.value("batch").toString();
        record["wafers"] = query.value("wafers").toInt();
        record["process"] = query.value("process").toString();
        record["location"] = query.value("location").toString();
        dataArray.append(record);
    }

    QJsonDocument doc(dataArray);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    QString topic = "database/updates";  //Debug to check database publishes correctly
    if (client->publish(topic, payload) == -1) {
        qDebug() << "Failed to publish database data.";
    } else {
        qDebug() << "Published database data to topic:" << topic;
    }
}

void MqttManager::publishDatabaseEntry(const QString &topic, const QString &data) { //Will only start trying to publish data once MQTT and SQL is connected
    if (client && client->state() == QMqttClient::Connected) {
        client->publish(topic, data.toUtf8());
    } else {
        emit logMessage("MQTT is not connected. Unable to publish data.");
    }
}

void MqttManager::handleSubscribedData(const QString &topic, const QByteArray &message) {
    // Handle specific topics if necessary
    if (topic == "some/specific/topic") {
        // Process the message for a specific topic
    } else {
        emit logMessage(QString("Unhandled topic: %1 with data: %2").arg(topic, QString::fromUtf8(message)));
    }
}
