#ifndef RFIDREADER_H
#define RFIDREADER_H

#include <QObject>
#include <QThread>
#include <QTimer>

// Status codes
#define STATUS_K            0
#define STATUS_ROR         1
#define STATUS_COLLISION     2
#define STATUS_TIMEOUT       3
#define STATUS_NO_ROOM       4

class RFIDReader : public QObject {
    Q_OBJECT

public:
    explicit RFIDReader(QObject *parent = nullptr);
    ~RFIDReader();

    void initialize();  // Initializes the RC522 reader
    void startPolling();  // Starts the polling loop for detecting tags
    bool detectTag();

signals:
    void tagDetected(QString tagId);  // Signal emitted when a tag is detected

private:
    void reset();  // Resets the RC522
    void antennaOn();  // Turns the antenna on
    uint8_t readFromRegister(uint8_t reg);  // Reads a value from an RC522 register
    void writeToRegister(uint8_t reg, uint8_t value);  // Writes a value to an RC522 register
    uint8_t communicateWithPICC(uint8_t command, uint8_t *data, uint8_t *dataLen);  // Communicates with the PICC

    bool mfrc522Initialized;  // Tracks whether the RC522 has been initialized
};

#endif // RFIDREADER_H
