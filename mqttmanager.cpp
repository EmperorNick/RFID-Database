#include "mqttmanager.h"
#include <QDebug>
#include <QtMqtt/QMqttClient>
#include <QSqlQuery>

MqttManager::MqttManager(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), client(new QMqttClient(this)) {
    client->setHostname(host);  // Set the MQTT broker host
    client->setPort(port);      // Set the MQTT broker port

    // Connect the message received signal
    connect(client, &QMqttClient::messageReceived, this, &MqttManager::onMessageReceived);

    connect(client, &QMqttClient::messageReceived, this, &MqttManager::onMessageReceived);

    // Connect the connected signal to the onConnected slot
    connect(client, &QMqttClient::connected, this, &MqttManager::onConnected);

    connect(client, &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state) {
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
        emit statusChanged(status);  // Emit the statusChanged signal with the updated status
    });
}

void MqttManager::connectToBroker() {
    client->connectToHost();
}

void MqttManager::publishMessage(const QString &topic, const QString &message) {
    client->publish(topic, message.toUtf8());
}

void MqttManager::subscribeToTopic(const QString &topic) {
    auto result = client->subscribe(topic);
    if (result) {
        qDebug() << "Subscribed to topic:" << topic;
    } else {
        qDebug() << "Failed to subscribe to topic:" << topic;
    }
}

void MqttManager::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic) {
    QString receivedData = QString::fromUtf8(message);
    QString receivedTopic = topic.name();
    emit logMessage(QString("Received data on topic '%1': %2").arg(receivedTopic, receivedData));
    handleSubscribedData(receivedTopic, message);
}

void MqttManager::onConnected() {
    qDebug() << "Connected to MQTT broker";
    subscribeToTopic("test/update");  // Subscribe to the "test/update" topic
    //startPeriodicPublishing(); // Start publishing once connected
}

void MqttManager::startPeriodicPublishing() {
    publishTimer->start(60000); // Publish every 60 seconds
}

void MqttManager::publishDatabaseData() {
    QSqlQuery query("SELECT * FROM people");  // Ensure your database connection is open
    while (query.next()) {
        QString name = query.value("name").toString();
        int age = query.value("age").toInt();
        QString message = QString("Name: %1, Age: %2").arg(name).arg(age);
        publishMessage("database/people", message);
    }
}

void MqttManager::publishDatabaseEntry(const QString &topic, const QString &data) {
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
