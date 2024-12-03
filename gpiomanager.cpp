#include "gpiomanager.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <iostream>
#include <chrono>
#include <thread>

#define SPI_CHANNEL 0 // Use CE0 (Chip Enable 0) for SPI communication
#define SPI_SPEED 1000000 // Set the SPI speed (4Mhz normally)

// Define the static instance pointer
GPIOManager* GPIOManager::instance = nullptr;  // Static instance initialization

// Constructor for GPIOManager
GPIOManager::GPIOManager(QObject *parent) : QObject(parent), irqPin(-1) {

    initRFIDReader(25); //Calls function and assigns BCM pin 25 as IRQ
    setupResetPin(24);
    resetMFRC522(24);

    //Error Code
    if (!setupSPI()) {
        std::cerr << "Failed to initialize SPI communication for RFID reader!" << std::endl;
    }

    // Set static instance
    instance = this;  // Set the current instance of GPIOManager

   /* // Start the polling loop in a new thread
    std::thread pollingThread(&GPIOManager::pollForRFIDTags, this);
    pollingThread.detach(); // Detach the thread so it runs independently*/
}

// Destructor
GPIOManager::~GPIOManager() {
    // Cleanup if needed (no explicit cleanup for wiringPi)
}

// Static interrupt handler (called when a tag is detected)
void GPIOManager::handleInterrupt() {
    std::cout << "Interrupt triggered!" << std::endl;

    // Access the GPIOManager instance and call the non-static readTagUID method
    if (instance->readTagUID()) {
        std::cout << "Tag UID read successfully!" << std::endl;
    } else {
        std::cout << "Failed to read tag UID!" << std::endl;
    }

    emit instance->tagDetected();  // Emit signal when IRQ pin is triggered
}

/*bool GPIOManager::isTagDetected() {
    unsigned char buffer[1] = {0x00};  // Command to check for tag presence
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 1);  // Send command and read response

    std::cout << "Tag detected" << std::endl;
    return true;  // Tag detected
}*/

/*void GPIOManager::pollForRFIDTags() {
    while (true) {
        // Use interrupt-based or polling-based tag detection
        if (isTagDetected()) {
            std::cout << "Tag detected!" << std::endl;
            // Handle the detected tag, e.g., read data from the RFID
        }

        // Sleep for a short interval (avoid 100% CPU usage)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Reduced polling interval
    }
}*/

bool GPIOManager::setupSPI() {
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
        std::cerr << "SPI Setup failed!" << std::endl;
        return false;
    }
    return true;
}

// Setup the reset pin (GPIO24)
void GPIOManager::setupResetPin(int rstPin) {
    this->rstPin = rstPin;
    pinMode(rstPin, OUTPUT);  // Set GPIO24 as output
    digitalWrite(rstPin, HIGH);  // Initially set reset pin HIGH

    std::cout << "RST Pin " << rstPin << " state: " << (digitalRead(rstPin) == HIGH ? "HIGH" : "LOW") << std::endl;
}

// Setup the interrupt pin (GPIO25)
void GPIOManager::initRFIDReader(int irqPin) {
    this->irqPin = irqPin;
    pinMode(irqPin, INPUT);  // IRQ pin should be an input
    pullUpDnControl(irqPin, PUD_UP);  // Enable pull-up resistor on IRQ pin

    // Connect interrupt handler for falling edge (detect when IRQ goes LOW)
    wiringPiISR(irqPin, INT_EDGE_FALLING, &GPIOManager::handleInterrupt);

    std::cout << "IRQ Pin " << irqPin << " state: " << (digitalRead(irqPin) == HIGH ? "HIGH" : "LOW") << std::endl;
    std::cout << "Starting to monitor IRQ pin..." << std::endl;
}

// Reset MFRC522 by setting RST pin LOW and then HIGH
void GPIOManager::resetMFRC522(int rstPin) {
    std::cout << "Resetting MFRC522...\n";
    this->rstPin = rstPin;
    digitalWrite(rstPin, LOW);   // Pull the RST pin low (reset)
    delay(100);               // Wait 100ms for reset
    digitalWrite(rstPin, HIGH);  // Release reset (set pin high)
}

void GPIOManager::simulateIRQ() {
    // Simulate the IRQ being triggered (set the IRQ pin to LOW)
    digitalWrite(irqPin, LOW);  // Simulate an interrupt
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Wait a bit
    digitalWrite(irqPin, HIGH); // Set it back to HIGH
    std::cout << "Simulated IRQ trigger!" << std::endl;

    // Trigger interrupt handler
    handleInterrupt(); // This will call the interrupt handler to simulate the tag detection
}

bool GPIOManager::readTagUID() {
    unsigned char buffer[12];  // Buffer to store the UID (MFRC522 can return up to 10 bytes for the UID)

    // Send the request to read the tag UID
    unsigned char request[] = {0x50, 0x00};  // MFRC522 command to request a tag UID (0x50 is the "Request ID" command)
    wiringPiSPIDataRW(SPI_CHANNEL, request, sizeof(request));

    // Wait for the reader to respond (you can adjust the delay if needed)
    delay(100);

    // Read the response from the RFID reader (UID)
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, sizeof(buffer));

    // Check the response, e.g., check the first byte to see if we have a valid tag
    if (buffer[0] == 0x00) {  // Typically, 0x00 indicates a successful read
        std::cout << "Tag UID: ";
        for (int i = 1; i < 5; ++i) {  // Loop through the UID bytes (assuming it's 4 bytes long)
            std::cout << std::hex << (int)buffer[i] << " ";
        }
        std::cout << std::endl;
        return true;  // Successfully read the tag UID
    } else {
        return false;  // Failed to read the tag UID
    }
}
