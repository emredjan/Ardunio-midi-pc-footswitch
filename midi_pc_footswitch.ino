#include <MIDI.h>
#include <JC_Button.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const byte BUTTON_RIGHT_PIN = 2;
const byte BUTTON_LEFT_PIN = 3;
// RGB Led on Pins 9-11 (PWM)
const byte LED_RED_PIN = 9;
const byte LED_GREEN_PIN = 10;
const byte LED_BLUE_PIN = 11;

const unsigned long LONG_PRESS = 1500;

const unsigned long FLICKER_NONE = 300;
const unsigned long FLICKER_SLOW = 60;
const unsigned long FLICKER_FAST = 30;
const byte LED_INTENSITY = 255;

const int MIN_PATCH = 0;
const int MAX_PATCH = 3;

Button buttonRight(BUTTON_RIGHT_PIN);
Button buttonLeft(BUTTON_LEFT_PIN);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);

    MIDI.begin(MIDI_CHANNEL_OMNI);

    buttonLeft.begin();
    buttonRight.begin();

    // Only enable Serial for USB MIDI debugging
    Serial.begin(9600);

    // Send a patch reset on start
    MIDI.sendProgramChange(0, 1);

    ledTurnOff();
}

void loop()
{

    enum states_t
    {
        WAIT,
        R_SHORT,
        L_SHORT,
        TO_RL_LONG,
        RL_LONG,
        TO_R_LONG,
        R_LONG,
        TO_L_LONG,
        L_LONG,
    };
    static states_t STATE;
    static int
        patchNum,
        lastPatchNum,
        singlePatch;
    static bool
        errorState(false),
        singlePatchState(false),
        singlePatchSend(false),
        resetState(false),
        resetSend(false);

    buttonRight.read();
    buttonLeft.read();

    if (errorState)
        ledFlicker(FLICKER_FAST, 10, LED_INTENSITY, 0, 0);

    if (singlePatchState)
        ledFlicker(FLICKER_SLOW, 4, LED_INTENSITY, 0, LED_INTENSITY);

    if (singlePatchSend)
        MIDI.sendProgramChange(singlePatch, 1);

    if (resetState)
        ledFlicker(FLICKER_SLOW, 4, LED_INTENSITY, LED_INTENSITY, 0);

    if (patchNum != lastPatchNum)
    {
        if (patchNum > lastPatchNum)
            ledFlicker(FLICKER_NONE, 1, 0, LED_INTENSITY, 0);
        else
            ledFlicker(FLICKER_NONE, 1, 0, 0, LED_INTENSITY);

        MIDI.sendProgramChange(patchNum, 1);

        lastPatchNum = patchNum;
    }

    switch (STATE)
    {
    case WAIT:
        errorState = false;
        singlePatchState = false;
        singlePatchSend = false;
        resetState = false;
        if (buttonRight.wasReleased())
            STATE = R_SHORT;
        else if (buttonLeft.wasReleased())
            STATE = L_SHORT;
        else if (buttonRight.pressedFor(LONG_PRESS))
            STATE = TO_R_LONG;
        else if (buttonLeft.pressedFor(LONG_PRESS))
            STATE = TO_L_LONG;
        break;

    case R_SHORT:
        ++patchNum;
        if (patchNum > MAX_PATCH)
        {
            patchNum = min(patchNum, MAX_PATCH);
            errorState = true;
        }
        STATE = WAIT;
        break;

    case L_SHORT:
        --patchNum;
        if (patchNum < MIN_PATCH)
        {
            patchNum = max(patchNum, MIN_PATCH);
            errorState = true;
        }
        STATE = WAIT;
        break;

    case TO_RL_LONG:
        resetState = false;
        singlePatchState = true;
        if (buttonRight.wasReleased() || buttonLeft.wasReleased())
            STATE = RL_LONG;
        break;

    case RL_LONG:
        singlePatch = 124;
        singlePatchSend = true;
        STATE = WAIT;
        break;

    case TO_R_LONG:
        if (buttonLeft.pressedFor(LONG_PRESS))
            STATE = TO_RL_LONG;
        else
        {
            singlePatchState = true;
            if (buttonRight.wasReleased())
                STATE = R_LONG;
        }
        break;

    case R_LONG:
        singlePatch = 123;
        singlePatchSend = true;
        STATE = WAIT;
        break;

    case TO_L_LONG:
        if (buttonRight.pressedFor(LONG_PRESS))
            STATE = TO_RL_LONG;
        else
        {
            resetState = true;
            if (buttonLeft.wasReleased())
                STATE = L_LONG;
        }
        break;

    case L_LONG:
        patchNum = 0;
        STATE = WAIT;
        break;
    }
}

void ledSetColor(byte redIntensity, byte greenIntensity, byte blueIntensity)
{
    analogWrite(LED_RED_PIN, redIntensity);
    analogWrite(LED_GREEN_PIN, greenIntensity);
    analogWrite(LED_BLUE_PIN, blueIntensity);

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
    byte _redIntensity;
    byte _greenIntensity;
    byte _blueIntensity;

    unsigned long previousBlink = 0;
    bool ledState = false;
    int timesFlickered = 0;

    while (timesFlickered <= flickerCount)
    {
        if (millis() - previousBlink >= flickerTime)
        {
            ledState = !ledState;
            if (ledState)
            {
                _redIntensity = redIntensity;
                _greenIntensity = greenIntensity;
                _blueIntensity = blueIntensity;
            }
            else
            {
                _redIntensity = 0;
                _greenIntensity = 0;
                _blueIntensity = 0;
            }

            previousBlink = millis();

            ledSetColor(_redIntensity, _greenIntensity, _blueIntensity);
            timesFlickered++;
        }
    }
    ledTurnOff();
}
