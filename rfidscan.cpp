#include "rfidscan.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <linux/types.h>
#include <stdint.h>
#include <cstring>
#include <stdio.h>

#define RSTPIN 25  // GPIO 25 (BCM), corresponds to physical pin 22
#define SPI_CHANNEL 0  // SPI channel 0
#define SPI_SPEED 1000000  // SPI speed in Hz

using namespace std;

/**
 * Constructor.
 * Prepares the output pins.
 */
RFIDScan::RFIDScan() {
    if (wiringPiSetupGpio() == -1) {
        printf("Failed to initialize GPIO. Use sudo.\n");
    }
    pinMode(RSTPIN, OUTPUT);
    digitalWrite(RSTPIN, LOW);

    // Set SPI bus to work with the RFID module
    setSPIConfig();
} // End constructor

void RFIDScan::setSPIConfig() {
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {
        printf("Failed to initialize SPI.\n");
    }
} // End setSPIConfig()

/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the RFID Module
/////////////////////////////////////////////////////////////////////////////////////
void RFIDScan::WriteRegister(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg & 0x7E;
    data[1] = value;
    wiringPiSPIDataRW(SPI_CHANNEL, data, 2);
} // End WriteRegister()

void RFIDScan::WriteRegister(uint8_t reg, uint8_t count, uint8_t *values) {
    for (uint8_t index = 0; index < count; index++) {
        WriteRegister(reg, values[index]);
    }
} // End WriteRegister()

uint8_t RFIDScan::ReadRegister(uint8_t reg) {
    uint8_t data[2];
    data[0] = 0x80 | (reg & 0x7E);
    data[1] = 0;  // Dummy byte to read
    wiringPiSPIDataRW(SPI_CHANNEL, data, 2);
    return data[1];
} // End ReadRegister()

void RFIDScan::SetRegisterBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = ReadRegister(reg);
    WriteRegister(reg, tmp | mask);
} // End SetRegisterBitMask()

void RFIDScan::ClearRegisterBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = ReadRegister(reg);
    WriteRegister(reg, tmp & (~mask));
} // End ClearRegisterBitMask()

void RFIDScan::Init() {
    if (digitalRead(RSTPIN) == LOW) {
        digitalWrite(RSTPIN, HIGH);
        delay(50);
    } else {
        Reset();
    }

    WriteRegister(TModeReg, 0x80);
    WriteRegister(TPrescalerReg, 0xA9);
    WriteRegister(TReloadRegH, 0x03);
    WriteRegister(TReloadRegL, 0xE8);

    WriteRegister(TxASKReg, 0x40);
    WriteRegister(ModeReg, 0x3D);
    AntennaOn();
} // End Init()

void RFIDScan::Reset() {
    WriteRegister(CommandReg, PCD_SoftReset);
    delay(50);
    while (ReadRegister(CommandReg) & (1 << 4)) {
        // Wait for reset to complete
    }
} // End Reset()

void RFIDScan::AntennaOn() {
    uint8_t value = ReadRegister(TxControlReg);
    if ((value & 0x03) != 0x03) {
        WriteRegister(TxControlReg, value | 0x03);
    }
} // End AntennaOn()

void RFIDScan::AntennaOff() {
    ClearRegisterBitMask(TxControlReg, 0x03);
} // End AntennaOff()

bool RFIDScan::PICC_IsNewCardPresent() {
    uint8_t bufferATQA[2];
    uint8_t bufferSize = sizeof(bufferATQA);

    uint8_t command[] = {PICC_CMD_REQA};
    uint8_t result = PCD_TransceiveData(command, 1, bufferATQA, &bufferSize);

    printf("PICC_IsNewCardPresent - Result: %d, BufferSize: %d\n", result, bufferSize);
    return (result == STATUS_OK && bufferSize == 2);
}


bool RFIDScan::PICC_ReadCardSerial() {
    uint8_t result = PICC_Select(&this->uid);
    return result == STATUS_OK;
}

/**
 * Transceives data with the PICC (transmit and receive).
 * This method sends data to the RFID reader and receives the response.
 */
uint8_t RFIDScan::PCD_TransceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen) {
    if (sendLen == 0 || sendData == nullptr || backData == nullptr || backLen == nullptr) {
        return STATUS_ERROR;
    }

    // Write data to FIFO buffer, send it, and read the response.
    WriteRegister(CommandReg, PCD_Idle);  // Stop any active command

    // Write sendData to FIFO
    for (uint8_t i = 0; i < sendLen; ++i) {
        WriteRegister(FIFODataReg, sendData[i]);
    }

    // Start the Transceive command
    WriteRegister(CommandReg, PCD_Transceive);
    SetRegisterBitMask(BitFramingReg, 0x80);  // Start send

    // Wait for completion
    uint8_t waitIrq = 0x30;
    for (uint16_t i = 0; i < 2000; ++i) {
        uint8_t irqVal = ReadRegister(ComIrqReg);
        if (irqVal & waitIrq) {
            ClearRegisterBitMask(BitFramingReg, 0x80);  // Stop send
            break;
        }
    }

    // Read received data from FIFO
    uint8_t fifoLevel = ReadRegister(FIFOLevelReg);
    if (fifoLevel > *backLen) {
        return STATUS_ERROR;
    }

    *backLen = fifoLevel;
    for (uint8_t i = 0; i < fifoLevel; ++i) {
        backData[i] = ReadRegister(FIFODataReg);
    }

    return STATUS_OK;
}

/**
 * Selects the PICC (card).
 * This function performs an anti-collision and selects the card.
 */
uint8_t RFIDScan::PICC_Select(Uid *uid) {
    uint8_t buffer[9] = {0};  // Initialize buffer to zero
    uint8_t bufferSize = sizeof(buffer);

    // Anti-collision command (cascade level 1)
    buffer[0] = PICC_CMD_SEL_CL1; // Using cascade level 1
    buffer[1] = 0x20;             // NVB (Number of Valid Bits)

    // Send anti-collision command
    uint8_t result = PCD_TransceiveData(buffer, 2, buffer, &bufferSize);
    if (result != STATUS_OK) {
        return result;
    }

    // Copy received UID data to uid struct
    if (bufferSize < 5) {
        return STATUS_ERROR;  // Invalid UID length
    }

    uid->size = bufferSize - 2;
    std::memcpy(uid->uidByte, buffer, uid->size);
    uid->sak = buffer[bufferSize - 1];

    return STATUS_OK;
}
