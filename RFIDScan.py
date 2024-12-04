#!/usr/bin/env python3
# -*- coding: utf8 -*-

import MFRC522
import signal
import sys
import time

sys.stdout.reconfigure(line_buffering=True)
continue_reading = True

# Function to read UID and convert it to a string
def uidToString(uid):
    return ''.join(format(i, '02X') for i in uid)

# Cleanup function for SIGINT and SIGTERM
def end_read(signal, frame):
    global continue_reading
    continue_reading = False
    print("Stopped reading and cleaned up.")
    sys.exit(0)

# Hook the SIGINT and SIGTERM
signal.signal(signal.SIGINT, end_read)
signal.signal(signal.SIGTERM, end_read)

# Initialize the RFID reader
def initialize_reader():
    try:
        reader = MFRC522.MFRC522()
        print("RFID reader initialized.")
        return reader
    except Exception as e:
        print(f"Error initializing RFID reader: {e}")
        sys.exit(1)

# Create an object of the class MFRC522
MIFAREReader = initialize_reader()

# Main loop
while continue_reading:
    try:
        status, TagType = MIFAREReader.MFRC522_Request(MIFAREReader.PICC_REQIDL)

        if status == MIFAREReader.MI_OK:
            status, uid = MIFAREReader.MFRC522_SelectTagSN()

            if status == MIFAREReader.MI_OK:
                print(f"{uidToString(uid)}")
            else:
                print("Failed to read UID.")
        else:
            time.sleep(0.1)  # Slight delay to prevent CPU overuse

    except KeyboardInterrupt:
        print("KeyboardInterrupt detected. Stopping...")
        break
    except IOError as e:
        print(f"I/O error occurred: {e}. Reinitializing RFID reader...")
        MIFAREReader = initialize_reader()
    except Exception as e:
        print(f"Unexpected error: {e}. Retrying...")
        time.sleep(1)

print("Program terminated.")
