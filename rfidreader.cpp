#include "rfidreader.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <QDebug>
#include <QTimer>

#define SPI_CHANNEL 0
#define SPI_SPEED 500000

// RC522 Register Definitions
#define CommandReg           0x01
#define ComIEnReg            0x02
#define DivIEnReg            0x03
#define ComIrqReg            0x04
#define DivIrqReg            0x05
#define ErrorReg             0x06
#define Status1Reg           0x07
#define Status2Reg           0x08
#define FIFODataReg          0x09
#define FIFOLevelReg         0x0A
#define ControlReg           0x0C
#define BitFramingReg        0x0D
#define CollReg              0x0E
#define ModeReg              0x11
#define TxControlReg         0x14
#define TxASKReg             0x15
#define TModeReg             0x2A
#define TPrescalerReg        0x2B
#define TReloadRegH          0x2C
#define TReloadRegL          0x2D
#define CRCResultRegH        0x21
#define CRCResultRegL        0x22
#define AutoTestReg          0x36
#define VersionReg           0x37
#define RFCfgReg             0x26

RFIDReader::RFIDReader(QObject *parent) : QObject(parent) {
    mfrc522Initialized = false;
}

RFIDReader::~RFIDReader() {
}

void RFIDReader::initialize() {
    if (wiringPiSetupGpio() == -1) {
        qDebug() << "Failed to initialize GPIO.";
        return;
    }

    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {
        qDebug() << "Failed to initialize SPI.";
        return;
    }

    // Configure the Reset pin
    pinMode(25, OUTPUT); // RST pin on GPIO 25
    digitalWrite(25, HIGH); // Ensure the RC522 is powered on

    // Initialize the RC522
    reset(); // Perform a soft reset of the RFID reader

    // Configure Timer
    writeToRegister(TModeReg, 0x8D);       // TAuto=1; timers start automatically at the end of transmission
    writeToRegister(TPrescalerReg, 0x3E); // Timer prescaler
    writeToRegister(TReloadRegL, 30);     // Reload timer low byte
    writeToRegister(TReloadRegH, 0);      // Reload timer high byte
    // Configure Transmission
    writeToRegister(TxASKReg, 0x40);      // 100% ASK modulation (amplitude shift keying)
    // Configure Default Mode
    writeToRegister(ModeReg, 0x3D);       // CRC preset to 0x6363 (ISO/IEC 14443 standard)
    // Enable Receiver Gain
    writeToRegister(RFCfgReg, 0x30);      // Set RxGain to maximum value (48 dB)
    // Turn on the Antenna
    antennaOn();
    mfrc522Initialized = true;

    qDebug() << "RFID Reader initialized.";

    // Example: Reading and Writing to Version Register (should be 0x91 or 0x92 for RC522)
    uint8_t version = readFromRegister(VersionReg);
    qDebug() << "Version Register:" << QString::number(static_cast<int>(version), 16).toUpper();
    uint8_t txControl = readFromRegister(TxControlReg);
    if (txControl & 0x03) {
        qDebug() << "Antenna is enabled.";
    } else {
        qDebug() << "Antenna is not enabled. Attempting to enable.";
        antennaOn();
    }
    // Write and read back from a test register
    writeToRegister(ModeReg, 0x3D); // Example value
    uint8_t modeValue = readFromRegister(ModeReg);
    if (modeValue == 0x3D) {
        qDebug() << "ModeReg correctly configured.";
    } else {
        qDebug() << "Failed to configure ModeReg, read value:" << modeValue;
    }
    uint8_t rawData[16];
    for (int i = 0; i < 16; ++i) {
        rawData[i] = readFromRegister(FIFODataReg);
    }
    qDebug() << "Raw Data:" << QByteArray(reinterpret_cast<char*>(rawData), 16).toHex();
}

void RFIDReader::startPolling() {
    if (!mfrc522Initialized) {
        qDebug() << "RFID Reader not initialized.";
        return;
    }

    QTimer *pollingTimer = new QTimer(this);
    connect(pollingTimer, &QTimer::timeout, this, &RFIDReader::detectTag);
    pollingTimer->start(500); // Poll every 500 ms
}

bool RFIDReader::detectTag() {
    static uint8_t lastStatus = 0xFF; // Track the previous status
    uint8_t bufferATQA[2] = {0};
    uint8_t bufferSize = sizeof(bufferATQA);

    writeToRegister(FIFOLevelReg, 0x80); // Clear FIFO buffer

    qDebug() << "Sending REQA to check for tag...";
    uint8_t status;
    int retryCount = 3;
    do {
        status = communicateWithPICC(0x26, bufferATQA, &bufferSize);
        if (status == STATUS_K) {
            break;
        }
        qDebug() << "Retrying REQA...";
        QThread::msleep(50); // Wait before retrying
    } while (retryCount-- > 0);

    if (status == lastStatus) {
        return status == STATUS_K;
    }

    lastStatus = status;

    if (status == STATUS_K) {
        qDebug() << "REQA Status:" << status;
        qDebug() << "Tag detected!";
        emit tagDetected("RFID Tag Detected!");
        return true;
    } else {
        qDebug() << "No tag detected. Status:" << status;
        return false;
    }
}

void RFIDReader::reset() {
    writeToRegister(CommandReg, 0x0F); // Soft reset command
    QThread::msleep(50);              // Allow time for reset to complete
}

void RFIDReader::antennaOn() {
    uint8_t value = readFromRegister(TxControlReg);
    if (!(value & 0x03)) {
        writeToRegister(TxControlReg, value | 0x03);
    }
}

uint8_t RFIDReader::readFromRegister(uint8_t reg) {
    uint8_t buffer[2] = {static_cast<uint8_t>(((reg << 1) & 0x7E) | 0x80), 0}; // MSB for read, LSB ignored
    int result = wiringPiSPIDataRW(SPI_CHANNEL, buffer, sizeof(buffer));
    if (result == -1) {
        qDebug() << "SPI Read failed for register:" << reg;
    }
    return buffer[1];
}

void RFIDReader::writeToRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {static_cast<uint8_t>((reg << 1) & 0x7E), value}; // MSB for write
    int result = wiringPiSPIDataRW(SPI_CHANNEL, buffer, sizeof(buffer));
    if (result == -1) {
        qDebug() << "SPI Write failed for register:" << reg;
    }
}

uint8_t RFIDReader::communicateWithPICC(uint8_t command, uint8_t *data, uint8_t *dataLen) {
    writeToRegister(CommandReg, 0x00);  // Idle command
    writeToRegister(FIFOLevelReg, 0x80);  // Flush FIFO

    // Write data to FIFO
    for (uint8_t i = 0; i < *dataLen; i++) {
        writeToRegister(FIFODataReg, data[i]);
    }

    writeToRegister(CommandReg, command);  // Start communication
    writeToRegister(BitFramingReg, 0x80);  // StartSend = 1

    // Wait for completion
    for (int i = 0; i < 200; i++) {
        uint8_t irq = readFromRegister(ComIrqReg);
        if (irq & 0x01) {
            qDebug() << "Communication timed out.";
            return STATUS_TIMEOUT; // Timer interrupt
        }
        if (irq & 0x30) {
            break; // RxIRq or IdleIRq
        }
        QThread::msleep(1);
    }

    // Check for errors
    uint8_t error = readFromRegister(ErrorReg);
    if (error & 0x13) { // BufferOvfl, ParityErr, ProtocolErr
        qDebug() << "Communication error: ErrorReg value:" << error;
        return STATUS_ROR;
    }

    // Read received data
    uint8_t receivedLength = readFromRegister(FIFOLevelReg);
    if (receivedLength > *dataLen) {
        qDebug() << "Received data exceeds buffer size.";
        return STATUS_NO_ROOM;
    }

    for (uint8_t i = 0; i < receivedLength; i++) {
        data[i] = readFromRegister(FIFODataReg);
    }
    *dataLen = receivedLength;

    return STATUS_K;
}
