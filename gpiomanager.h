#ifndef GPIOMANAGER_H
#define GPIOMANAGER_H

#include <QObject>

class GPIOManager : public QObject
{
    Q_OBJECT

public:
    explicit GPIOManager(QObject *parent = nullptr);
    ~GPIOManager();

    void initRFIDReader(int irqPin);
    void setupInterrupt();
    static void handleInterrupt();
    void pollForRFIDTags();
    void resetMFRC522(int rstPin);
    void setupResetPin(int rstPin);
    void simulateIRQ();
    bool readTagUID();

signals:
    void tagDetected();  // Signal when a tag is detected

private:
    static GPIOManager *instance;  // Static pointer to the current instance
    int irqPin;
    int rstPin;
    bool isTagDetected();
    static const int POLLING_INTERVAL_MS = 100; // Polling interval in milliseconds
    bool setupSPI();
};

#endif // GPIOMANAGER_H
