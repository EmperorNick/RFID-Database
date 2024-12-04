#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <QObject>
//#include <QMqttClient>
#include <QtMqtt/QMqttClient>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>

class MqttManager : public QObject {
    Q_OBJECT

public:
    explicit MqttManager(const QString &host, quint16 port, QObject *parent = nullptr);
    void connectToBroker();
    void publishMessage(const QString &topic, const QString &message);
    void subscribeToTopic(const QString &topic);
    void startPeriodicPublishing();
    QMqttClient* getClient() const { return client; }
    void publishDatabaseEntry(const QString &topic, const QString &data);
    void handleSubscribedData(const QString &topic, const QByteArray &message);

signals: // Add this signals section
    void messageReceived(const QString &message, const QMqttTopicName &topic); // Signal declaration
    void statusChanged(const QString &status);
    void logMessage(const QString &message);

private slots:
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic); // Updated parameter type
    void onConnected();
    void publishDatabaseData(); // Periodic publishing function


private:
    QMqttClient *client; // Pointer to the MQTT client
    QTimer *publishTimer; // The timer for publishing data
};

#endif // MQTTMANAGER_H
