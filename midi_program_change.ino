// midi.controller
// Sends midi program change
// Inspired by Aaron Lyon April 2018

#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

byte patchNum = 0;

const int buttonRight = 2;
const int buttonLeft = 3;
// RGB Led on Pins 9-11 (PWM)
const int ledRed = 9;
const int ledGreen = 10;
const int ledBlue = 11;

const long delayButton = 300;
const long flickerSlow = 75;
const long flickerFast = 30;
const int maxPatches = 16;

void setup()
{
    pinMode(buttonRight, INPUT_PULLUP);
    pinMode(buttonLeft, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ledRed, OUTPUT);
    pinMode(ledGreen, OUTPUT);
    pinMode(ledBlue, OUTPUT);

    MIDI.begin(MIDI_CHANNEL_OMNI);
    Serial.begin(9600);

    // Send a patch reset on start
    MIDI.sendProgramChange(patchNum, 1);

    ledTurnOff();
}

void ledSetColor(byte redIntensity, byte greenIntensity, byte blueIntensity)
{
    analogWrite(ledRed, redIntensity);
    analogWrite(ledGreen, greenIntensity);
    analogWrite(ledBlue, blueIntensity);

    // temp for internal led
    digitalWrite(LED_BUILTIN, HIGH);
}

void ledTurnOff()
{
    ledSetColor(0, 0, 0);

    // temp for internal led
    digitalWrite(LED_BUILTIN, LOW);
}

void ledFlicker(long flickerTime, int flickerCount, byte redIntensity, byte greenIntensity, byte blueIntensity)
{
    // Flicker the LED to indicate end of range
    for (size_t i = 0; i < flickerCount; i++)
    {
        ledSetColor(redIntensity, greenIntensity, blueIntensity);
        delay(flickerTime);
        ledTurnOff();
        delay(flickerTime);
    }
}

void loop()
{
    if (digitalRead(buttonRight) == LOW)
    {

        if (patchNum < maxPatches)
        {
            // Green for Next Program
            ledSetColor(0, 255, 0);

            // Next Program
            patchNum++;
            MIDI.sendProgramChange(patchNum, 1);

            delay(delayButton);
            ledTurnOff();
        }
        else
        {
            // Red flicker for end of range
            ledFlicker(flickerFast, 5, 255, 0, 0);
        }
    }

    if (digitalRead(buttonLeft) == LOW)
    {
        if (patchNum >= 1)
        {
            // Blue for Previous Program
            ledSetColor(0, 0, 255);

            // Previous Program
            patchNum--;
            MIDI.sendProgramChange(patchNum, 1);

            delay(delayButton);
            ledTurnOff();
        }
        else
        {
            // Red flicker for end of range
            ledFlicker(flickerFast, 5, 255, 0, 0);
        }
    }

    // Reset Program when both buttons are pressed
    if (digitalRead(buttonRight) == LOW && digitalRead(buttonLeft) == LOW)
    {
        // Yellow flicker for reset
        ledFlicker(flickerSlow, 10, 255, 255, 0);

        // Reset Patch to Zero
        patchNum = 0;
        MIDI.sendProgramChange(patchNum, 1);
    }
}
