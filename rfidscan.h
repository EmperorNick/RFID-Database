#ifndef RFIDSCAN_H
#define RFIDSCAN_H

#include <stdint.h>
#include <string>

using namespace std;

class RFIDScan {
public:
    // Struct definition for Uid must be before any usage in member functions
    struct Uid {
        uint8_t size;        // Number of bytes in the UID
        uint8_t uidByte[10]; // UID bytes
        uint8_t sak;         // SAK byte returned from the PICC after successful selection
    };

    // Constructor
    RFIDScan();

    // Basic interface functions for communicating with the RFID Module
    void WriteRegister(uint8_t reg, uint8_t value);
    void WriteRegister(uint8_t reg, uint8_t count, uint8_t *values);
    uint8_t ReadRegister(uint8_t reg);
    void ReadRegister(uint8_t reg, uint8_t count, uint8_t *values, uint8_t rxAlign = 0);

    void SetRegisterBitMask(uint8_t reg, uint8_t mask);
    void ClearRegisterBitMask(uint8_t reg, uint8_t mask);
    void setSPIConfig();

    // Functions for interacting with the RFID
    void Init();
    void Reset();
    void AntennaOn();
    void AntennaOff();

    bool PICC_IsNewCardPresent();
    bool PICC_ReadCardSerial();
    uint8_t PICC_Select(Uid *uid);  // Now the `Uid` type is properly declared before use

    // Getter for uid
    Uid getUid() const { return uid; }

    // Register constants as per the datasheet
    enum PCD_Register {
        CommandReg = 0x01,
        ComIEnReg = 0x02,
        DivIEnReg = 0x03,
        ComIrqReg = 0x04,
        DivIrqReg = 0x05,
        ErrorReg = 0x06,
        Status1Reg = 0x07,
        Status2Reg = 0x08,
        FIFODataReg = 0x09,
        FIFOLevelReg = 0x0A,
        WaterLevelReg = 0x0B,
        ControlReg = 0x0C,
        BitFramingReg = 0x0D,
        CollReg = 0x0E,
        // Page 1: Command
        ModeReg = 0x11,
        TxModeReg = 0x12,
        RxModeReg = 0x13,
        TxControlReg = 0x14,
        TxASKReg = 0x15,
        TxSelReg = 0x16,
        RxSelReg = 0x17,
        RxThresholdReg = 0x18,
        DemodReg = 0x19,
        MfTxReg = 0x1C,
        MfRxReg = 0x1D,
        SerialSpeedReg = 0x1F,
        // Page 2: Configuration
        CRCResultRegH = 0x21,
        CRCResultRegL = 0x22,
        ModWidthReg = 0x24,
        RFCfgReg = 0x26,
        GsNReg = 0x27,
        CWGsPReg = 0x28,
        ModGsPReg = 0x29,
        TModeReg = 0x2A,
        TPrescalerReg = 0x2B,
        TReloadRegH = 0x2C,
        TReloadRegL = 0x2D,
        TCounterValueRegH = 0x2E,
        TCounterValueRegL = 0x2F,
        // Page 3: Test Registers
        TestSel1Reg = 0x31,
        TestSel2Reg = 0x32,
        TestPinEnReg = 0x33,
        TestPinValueReg = 0x34,
        TestBusReg = 0x35,
        AutoTestReg = 0x36,
        VersionReg = 0x37,
        AnalogTestReg = 0x38,
        TestDAC1Reg = 0x39,
        TestDAC2Reg = 0x3A,
        TestADCReg = 0x3B
    };

    enum PCD_Command {
        PCD_Idle = 0x00,
        PCD_Mem = 0x01,
        PCD_GenerateRandomID = 0x02,
        PCD_CalcCRC = 0x03,
        PCD_Transmit = 0x04,
        PCD_NoCmdChange = 0x07,
        PCD_Receive = 0x08,
        PCD_Transceive = 0x0C,
        PCD_MFAuthent = 0x0E,
        PCD_SoftReset = 0x0F
    };

    enum PICC_Command {
        PICC_CMD_REQA = 0x26,      // Request command, Type A
        PICC_CMD_WUPA = 0x52,      // Wake-Up command, Type A
        PICC_CMD_SEL_CL1 = 0x93,   // Anti-collision/Select, Cascade Level 1
        PICC_CMD_SEL_CL2 = 0x95,   // Anti-collision/Select, Cascade Level 2
        PICC_CMD_SEL_CL3 = 0x97,   // Anti-collision/Select, Cascade Level 3
        PICC_CMD_HLTA = 0x50       // Halt command
    };

    enum StatusCode {
        STATUS_OK = 1,
        STATUS_ERROR = 2,
        // Other status codes can be added here if needed
    };

private:
    static const uint8_t RSTPIN = 25;        // GPIO 25 (BCM), physical pin 22
    static const uint8_t SPI_CHANNEL = 0;    // SPI channel 0
    static const uint32_t SPI_SPEED = 1000000; // SPI speed in Hz

    Uid uid;  // Define a member variable of type `Uid` for card reading

    uint8_t PCD_TransceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen);
};

#endif // RFIDSCAN_H
