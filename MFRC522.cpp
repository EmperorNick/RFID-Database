#include "MFRC522.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <cstring>
#include <stdio.h>
#include <string>

using namespace std;

#define RSTPIN 22 // Physical pin 22 corresponds to GPIO 25

/**
 * Constructor.
 * Initializes WiringPi and prepares the output pins.
 */
MFRC522::MFRC522() {
    if (wiringPiSetup() < 0) {
        fprintf(stderr, "WiringPi setup failed: %s\n", strerror(errno));
        exit(1);
    }

    pinMode(RSTPIN, OUTPUT);
    digitalWrite(RSTPIN, LOW);
    setSPIConfig();
}

/**
 * Set SPI bus to work with MFRC522 chip.
 * Call this function if you have changed the SPI configuration.
 */
void MFRC522::setSPIConfig() {
    int spiChannel = 0; // SPI channel (0 or 1)
    int spiSpeed = 500000; // SPI speed in Hz

    if (wiringPiSPISetup(spiChannel, spiSpeed) < 0) {
        fprintf(stderr, "SPI setup failed: %s\n", strerror(errno));
        exit(1);
    }
}

/**
 * Writes a byte to the specified register in the MFRC522 chip.
 */
void MFRC522::PCD_WriteRegister(rfid_byte reg, rfid_byte value) {
    int spiChannel = 0;
    rfid_byte data[2] = {reg & 0x7E, value};
    wiringPiSPIDataRW(spiChannel, data, 2);
}

/**
 * Writes multiple bytes to the specified register in the MFRC522 chip.
 */
rfid_byte MFRC522::PICC_HaltA() {
    rfid_byte result;
    rfid_byte buffer[4];

    // Build command buffer
    buffer[0] = PICC_CMD_HLTA;
    buffer[1] = 0;

    // Calculate CRC_A
    result = PCD_CalculateCRC(buffer, 2, &buffer[2]);
    if (result != STATUS_OK) {
        return result;
    }

    // Transmit the buffer and receive the response
    // Use PCD_CommunicateWithPICC instead of PCD_Transceive
    result = PCD_CommunicateWithPICC(PCD_Transceive, 0x30, buffer, sizeof(buffer), NULL, NULL, NULL, 0, true);

    if (result == STATUS_TIMEOUT) {
        return STATUS_OK; // Timeout is interpreted as success
    }
    if (result == STATUS_OK) {
        return STATUS_ERROR; // Any response means failure in this context
    }

    return result;
}

/**
 * Reads a byte from the specified register in the MFRC522 chip.
 */
rfid_byte MFRC522::PCD_ReadRegister(rfid_byte reg) {
    int spiChannel = 0;
    rfid_byte data[2] = {static_cast<rfid_byte>(0x80 | (reg & 0x7E)), 0x00};
    wiringPiSPIDataRW(spiChannel, data, 2);
    return data[1];
}

/**
 * Reads multiple bytes from the specified register in the MFRC522 chip.
 */
void MFRC522::PCD_ReadRegister(rfid_byte reg, rfid_byte count, rfid_byte *values, rfid_byte rxAlign) {
    if (count == 0) {
        return;
    }

    int spiChannel = 0;
    rfid_byte buffer[count + 1];
    buffer[0] = 0x80 | (reg & 0x7E);

    wiringPiSPIDataRW(spiChannel, buffer, count + 1);

    for (int i = 0; i < count; i++) {
        values[i] = buffer[i + 1];
    }
}

/**
 * Resets the MFRC522 chip.
 */
void MFRC522::PCD_Reset() {
    digitalWrite(RSTPIN, LOW);
    delay(50); // Wait 50ms
    digitalWrite(RSTPIN, HIGH);
    delay(50); // Wait 50ms for the chip to stabilize
}

/**
 * Turns the antenna on.
 */
void MFRC522::PCD_AntennaOn() {
    rfid_byte value = PCD_ReadRegister(TxControlReg);
    if ((value & 0x03) != 0x03) {
        PCD_WriteRegister(TxControlReg, value | 0x03);
    }
}

/**
 * Turns the antenna off.
 */
void MFRC522::PCD_AntennaOff() {
    rfid_byte value = PCD_ReadRegister(TxControlReg);
    PCD_WriteRegister(TxControlReg, value & ~0x03);
}

/**
 * Initializes the MFRC522 chip.
 */
void MFRC522::PCD_Init() {
    PCD_Reset();

    // Set timer for communication timeout
    PCD_WriteRegister(TModeReg, 0x80); // TAuto=1
    PCD_WriteRegister(TPrescalerReg, 0xA9); // TPreScaler = 169
    PCD_WriteRegister(TReloadRegH, 0x03);
    PCD_WriteRegister(TReloadRegL, 0xE8);

    PCD_WriteRegister(TxASKReg, 0x40); // 100% ASK modulation
    PCD_WriteRegister(ModeReg, 0x3D); // CRC preset
    PCD_AntennaOn(); // Turn the antenna on
}

/**
 * Calculates a CRC_A.
 */
rfid_byte MFRC522::PCD_CalculateCRC(rfid_byte *data, rfid_byte length, rfid_byte *result) {
    PCD_WriteRegister(CommandReg, PCD_Idle);
    PCD_WriteRegister(DivIrqReg, 0x04); // Clear CRCIRq interrupt
    PCD_SetRegisterBitMask(FIFOLevelReg, 0x80); // Flush FIFO
    PCD_WriteRegister(FIFODataReg, length, data);
    PCD_WriteRegister(CommandReg, PCD_CalcCRC);

    for (int i = 5000; i > 0; i--) {
        rfid_byte n = PCD_ReadRegister(DivIrqReg);
        if (n & 0x04) { // CRCIRq bit set
            result[0] = PCD_ReadRegister(CRCResultRegL);
            result[1] = PCD_ReadRegister(CRCResultRegH);
            return STATUS_OK;
        }
    }

    return STATUS_TIMEOUT;
}

/**
 * Checks if a new card is present.
 */
bool MFRC522::PICC_IsNewCardPresent() {
    rfid_byte bufferATQA[2];
    rfid_byte bufferSize = sizeof(bufferATQA);
    rfid_byte result = PICC_REQA_or_WUPA(0x26, bufferATQA, &bufferSize); // REQA command
    return (result == STATUS_OK || result == STATUS_COLLISION);
}

/**
 * Reads the card's UID.
 */
bool MFRC522::PICC_ReadCardSerial() {
    rfid_byte validBits = 0;
    return (PICC_Select(&uid, validBits) == STATUS_OK);
}

rfid_byte MFRC522::PCD_CommunicateWithPICC(
    rfid_byte command,
    rfid_byte waitIRq,
    rfid_byte* sendData,
    rfid_byte sendLen,
    rfid_byte* backData,
    rfid_byte* backLen,
    rfid_byte* validBits,
    rfid_byte rxAlign,
    bool checkCRC
    ) {
    // Ensure the FIFO buffer is reset before communication
    PCD_ClearRegisterBitMask(FIFOLevelReg, 0x80);
    PCD_WriteRegister(FIFODataReg, sendLen, sendData);
    PCD_WriteRegister(BitFramingReg, (rxAlign << 4));
    PCD_WriteRegister(CommandReg, command);

    // Wait for the command to complete
    unsigned int i = 2000; // Timeout loop
    while (i--) {
        rfid_byte n = PCD_ReadRegister(ComIrqReg);
        if (n & waitIRq) break;
        if (n & 0x01) return STATUS_TIMEOUT; // Timer expired
    }

    // Handle errors and retrieve results
    rfid_byte errorReg = PCD_ReadRegister(ErrorReg);
    if (errorReg & 0x13) return STATUS_ERROR;

    if (backData && backLen) {
        rfid_byte length = PCD_ReadRegister(FIFOLevelReg);
        if (length > *backLen) return STATUS_NO_ROOM;
        *backLen = length;
        PCD_ReadRegister(FIFODataReg, length, backData, rxAlign);
    }

    return STATUS_OK;
}
