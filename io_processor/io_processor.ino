//
// io_processor - io front end processor
// Copyright (C) 2013 Scott Finneran
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <ProtoThreads.h>
#include <Timer.h>
#include <Wire.h>

enum module_types
{
    MODULE_QUAD_RELAY_OUT = 1,
};

#define NUM_OUTPUTS 4

bool currentState[NUM_OUTPUTS];
const int ioPin[NUM_OUTPUTS] = {A3, A2, A1, A0};

const int statusPin = 11;

const int A0Pin = 10;
const int A1Pin = 9;

uint8_t selectedRegister = 0xFF;

struct pt ptOutputDriver;
struct pt ptStatus;
Timer heartbeatTimer(500); // 500 millisecond timer.
struct pt_sem outputUpdate;

// Registers have the type in the upper nibble.
#define REG_TYPE(X) (((X) & 0xF0) >> 4)

// Registers have the index/number in the lower nibble.
#define REG_NUM(X) ((X) & 0x0F)

enum register_types
{
    REG_ID            = 0x00,
    REG_OUTPUT        = 0x01,
    REG_OUTPUT_BITMAP = 0x02,
    REG_INVALID       = 0x0F,
};

void setup()
{
    for (int i = 0; i < NUM_OUTPUTS; i++)
    {
        currentState[i] = false;
        pinMode(ioPin[i], OUTPUT);
    }

    pinMode(statusPin, OUTPUT);
    pinMode(A0Pin, INPUT);
    pinMode(A1Pin, INPUT);

    Wire.onReceive(receiveEvent); // register event
    Wire.onRequest(requestEvent); // register event
    Wire.begin(getBoardNumber()); // join i2c bus with address

    PT_INIT(&ptOutputDriver);
    PT_INIT(&ptStatus);
    PT_SEM_INIT(&outputUpdate, 1); // Set to cause an initial update.

    Serial.begin(9600);           // start serial for debug output
    Serial.print("\r\nQUAD-RELAY OUTPUT CAPE STARTED\r\n");
    Serial.print("Board Number: ");
    Serial.print(getBoardNumber());
    Serial.print("\r\n");

    Serial.print("A0: ");
    Serial.print(digitalRead(A0Pin));
    Serial.print("\r\n");

    Serial.print("A1: ");
    Serial.print(digitalRead(A1Pin));
    Serial.print("\r\n");

}

PT_THREAD(outputDriver(struct pt *pt))
{
    PT_BEGIN(pt);

    for (;;)
    {
        // Wait until there is something to do.
        PT_SEM_WAIT(pt, &outputUpdate);

        // Loop through the outputs and update them.
        // Most of the time they will remain unchanged.
        for (int i = 0; i < NUM_OUTPUTS; i++)
        {
            digitalWrite(ioPin[i], currentState[i]);
        }
    }
    PT_END(pt);
}

PT_THREAD(statusThread(struct pt *pt))
{
    PT_BEGIN(pt);

    // Wait until a fixed period. May need to speed this up.
    PT_WAIT_UNTIL(pt, heartbeatTimer.timerExpired());
    heartbeatTimer.timerReset();

    // Toggle the heartbeat/status LED.
    digitalWrite(statusPin, !digitalRead(statusPin));

    PT_END(pt);
}

void loop()
{
    outputDriver(&ptOutputDriver);
    statusThread(&ptStatus);
}

uint8_t getBoardNumber()
{
    bool A0 = digitalRead(A0Pin);
    bool A1 = digitalRead(A1Pin);

    uint8_t address = 0;
    if (A0) { address += 1; }
    if (A1) { address += 2; }

    return address + 1;
}

void flushInput()
{
    while (Wire.available() > 0) // loop through all
    {
        (void)Wire.read(); // flush the buffer
    }
}

// 1 is 10 A1
// 2 is 11 A0

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
    (void)howMany;

    selectedRegister = Wire.read();

    // The second byte contains a new value for the selected register.
    if (Wire.available() > 0)
    {
        uint8_t outputNumber = 0xFF;
        uint8_t newBitmap = 0xFF;
        uint8_t mask = 0x00;

        switch(REG_TYPE(selectedRegister))
        {
        case REG_OUTPUT:
            outputNumber = REG_NUM(selectedRegister);
            Serial.print("outputNumber: ");
            Serial.print(outputNumber);
            Serial.print("\r\n");
            if (outputNumber < NUM_OUTPUTS)
            {
                currentState[outputNumber] = (bool)Wire.read();
                PT_SEM_SIGNAL(0, &outputUpdate);
            }

            break;

        case REG_OUTPUT_BITMAP:
            Serial.print("BITMAP: ");

            newBitmap = (uint8_t)Wire.read();
            mask = 0x01;
            for (int i = 0; i < NUM_OUTPUTS; i++)
            {
                currentState[i] = (bool)(newBitmap & mask);
                    mask <<= 1;
            }

            PT_SEM_SIGNAL(0, &outputUpdate);

            Serial.print(newBitmap);
            Serial.print("\r\n");

            break;

        case REG_ID:
            Serial.print("REG_ID\r\n");
            // Register is read-only so flush and wait for read-request
            break;

        default:
            // Nothing to do
            Serial.print("default\r\n");
            selectedRegister = REG_INVALID;
            flushInput();
            break;
        }
    }
}

void requestEvent(void)
{
    uint8_t channelNumber = REG_NUM(selectedRegister);
    uint8_t currentValue = 0xFF;
    Serial.print("requestEvent()\r\n");

    switch (REG_TYPE(selectedRegister))
    {
    case REG_OUTPUT:
        if (channelNumber < NUM_OUTPUTS)
        {
            currentValue = (uint8_t)currentState[channelNumber];
        }
        Wire.write(currentValue);
        break;

    case REG_ID:
        Wire.write((uint8_t)MODULE_QUAD_RELAY_OUT);
        break;

    default:
        break;
    }

    // Transaction complete. Any subsequent request should be preceeded by a
    // receive containing the register number.
    selectedRegister = REG_INVALID;
}
