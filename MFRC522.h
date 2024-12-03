#ifndef MFRC522_H
#define MFRC522_H

#include <stdint.h>

#define MF_KEY_SIZE 6 // Key size for MIFARE cards

// Status codes
#define STATUS_OK            0
#define STATUS_ERROR         1
#define STATUS_COLLISION     2
#define STATUS_TIMEOUT       3
#define STATUS_NO_ROOM       4
#define STATUS_INTERNAL_ERROR 5
#define STATUS_INVALID       6
#define STATUS_CRC_WRONG     7
#define STATUS_MIFARE_NACK   8

// PICC Commands
#define PICC_CMD_REQA        0x26
#define PICC_CMD_WUPA        0x52
#define PICC_CMD_CT          0x88
#define PICC_CMD_SEL_CL1     0x93
#define PICC_CMD_SEL_CL2     0x95
#define PICC_CMD_SEL_CL3     0x97
#define PICC_CMD_MF_AUTH_KEY_A 0x60
#define PICC_CMD_MF_AUTH_KEY_B 0x61
#define PICC_CMD_MF_READ     0x30
#define PICC_CMD_MF_WRITE    0xA0
#define PICC_CMD_UL_WRITE    0xA2
#define PICC_CMD_MF_INCREMENT 0xC1
#define PICC_CMD_MF_DECREMENT 0xC0
#define PICC_CMD_MF_RESTORE  0xC2
#define PICC_CMD_MF_TRANSFER 0xB0
#define PICC_CMD_HLTA        0x50

// MFRC522 Registers
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

typedef uint8_t rfid_byte;

// Uid struct to hold card data
typedef struct {
    rfid_byte size;               // Number of bytes in the UID (4, 7, or 10)
    rfid_byte uidbyte[10];        // UID bytes (up to 10)
    rfid_byte sak;                // The SAK byte (Select Acknowledge)
} Uid;

// Key structure
typedef struct {
    rfid_byte keybyte[MF_KEY_SIZE];
} MIFARE_Key;

class MFRC522 {
public:
    Uid uid; // Stores the UID of the detected PICC

    MFRC522(); // Constructor

    void PCD_Init(); // Initializes the MFRC522 chip
    void PCD_Reset(); // Resets the MFRC522 chip
    void PCD_WriteRegister(rfid_byte reg, rfid_byte value);
    void PCD_WriteRegister(rfid_byte reg, rfid_byte count, rfid_byte *values);
    rfid_byte PCD_ReadRegister(rfid_byte reg);
    void PCD_ReadRegister(rfid_byte reg, rfid_byte count, rfid_byte *values, rfid_byte rxAlign = 0);
    void PCD_SetRegisterBitMask(rfid_byte reg, rfid_byte mask);
    void PCD_ClearRegisterBitMask(rfid_byte reg, rfid_byte mask);
    void PCD_AntennaOn();
    void PCD_AntennaOff();
    void setSPIConfig();
    rfid_byte PICC_Select(Uid *uid, rfid_byte validBits = 0);

    // Functions for communicating with PICCs
    rfid_byte PICC_RequestA(rfid_byte *bufferATQA, rfid_byte *bufferSize);
    bool PICC_IsNewCardPresent();
    bool PICC_ReadCardSerial();

    // Functions for MIFARE Classic PICCs
    rfid_byte MIFARE_Read(rfid_byte blockAddr, rfid_byte *buffer, rfid_byte *bufferSize);
    rfid_byte MIFARE_Write(rfid_byte blockAddr, rfid_byte *buffer, rfid_byte bufferSize);

    // Functions causing errors
    rfid_byte PICC_REQA_or_WUPA(rfid_byte command, rfid_byte *bufferATQA, rfid_byte *bufferSize);
    rfid_byte PICC_HaltA();
    rfid_byte PCD_CalculateCRC(rfid_byte *data, rfid_byte length, rfid_byte *result);
    rfid_byte PCD_CommunicateWithPICC(
        rfid_byte command,
        rfid_byte waitIRq,
        rfid_byte* sendData,
        rfid_byte sendLen,
        rfid_byte* backData,
        rfid_byte* backLen,
        rfid_byte* validBits,
        rfid_byte rxAlign,
        bool checkCRC
        );



    // Add PCD_Command enum
    enum PCD_Command {
        PCD_Idle            = 0x00,
        PCD_Mem             = 0x01,
        PCD_GenerateRandomID= 0x02,
        PCD_CalcCRC         = 0x03,
        PCD_Transmit        = 0x04,
        PCD_Receive         = 0x08,
        PCD_Transceive      = 0x0C,
        PCD_MFAuthent       = 0x0E,
        PCD_SoftReset       = 0x0F
    };



private:
    void PCD_ResetFIFO(); // Helper to reset FIFO buffer
    };

#endif // MFRC522_H
