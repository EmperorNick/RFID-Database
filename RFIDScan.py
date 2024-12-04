#!/usr/bin/env python3
# -*- coding: utf8 -*-
#
#    Copyright 2018 Daniel Perron
#
#    Base on Mario Gomez <mario.gomez@teubi.co>   MFRC522-Python
#
#    This file use part of MFRC522-Python
#    MFRC522-Python is a simple Python implementation for
#    the MFRC522 NFC Card Reader for the Raspberry Pi.
#
#    MFRC522-Python is free software:
#    you can redistribute it and/or modify
#    it under the terms of
#    the GNU Lesser General Public License as published by the
#    Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    MFRC522-Python is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the
#    GNU Lesser General Public License along with MFRC522-Python.
#    If not, see <http://www.gnu.org/licenses/>.
#

import MFRC522
import signal
import sys

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

# Create an object of the class MFRC522
MIFAREReader = MFRC522.MFRC522()
#print("RFID Scanner Ready. Press Ctrl-C to stop.") #Removed as no need from

# Main loop
while continue_reading:
    status, TagType = MIFAREReader.MFRC522_Request(MIFAREReader.PICC_REQIDL)
    if status == MIFAREReader.MI_OK:
        status, uid = MIFAREReader.MFRC522_SelectTagSN()
        if status == MIFAREReader.MI_OK:
            print(f"{uidToString(uid)}")
        else:
            print("Failed to read UID.")

