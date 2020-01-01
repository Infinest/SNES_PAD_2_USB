#include <Joystick.h>

/*
  SNES PAD to USB with SFC NTT Data PAD support
*/
const byte BUTTON_B                = 0,
           BUTTON_Y                = 1,
           BUTTON_SELECT           = 2,
           BUTTON_START            = 3,
           BUTTON_UP               = 4,
           BUTTON_DOWN             = 5,
           BUTTON_LEFT             = 6,
           BUTTON_RIGHT            = 7,
           BUTTON_A                = 8,
           BUTTON_X                = 9,
           BUTTON_L                = 10,
           BUTTON_R                = 11,
           BUTTON_UNASSIGNED_1     = 12,
           BUTTON_IS_NTT_DATAPAD   = 13,
           BUTTON_UNASSIGNED_2     = 14,
           BUTTON_UNASSIGNED_3     = 15,
           BUTTON_NTT_0            = 16,
           BUTTON_NTT_1            = 17,
           BUTTON_NTT_2            = 18,
           BUTTON_NTT_3            = 19,
           BUTTON_NTT_4            = 20,
           BUTTON_NTT_5            = 21,
           BUTTON_NTT_6            = 22,
           BUTTON_NTT_7            = 23,
           BUTTON_NTT_8            = 24,
           BUTTON_NTT_9            = 25,
           BUTTON_NTT_STAR         = 26,
           BUTTON_NTT_HASH         = 27,
           BUTTON_NTT_DOT          = 28,
           BUTTON_NTT_C            = 29,
           BUTTON_NTT_UNASSIGNED_1 = 30,
           BUTTON_NTT_END_COM      = 31;

const byte BUTTONS_TO_IGNORE[] = {BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_UNASSIGNED_1, BUTTON_IS_NTT_DATAPAD, BUTTON_UNASSIGNED_2, BUTTON_UNASSIGNED_3, BUTTON_NTT_UNASSIGNED_1};
const byte BUTTON_BIT_TO_USB_BUTTON[] = {
  0,  // BUTTON_B
  1,  // BUTTON_Y
  2,  // BUTTON_SELECT
  3,  // BUTTON_START
  -1, // BUTTON_UP
  -1, // BUTTON_DOWN
  -1, // BUTTON_LEFT
  -1, // BUTTON_RIGHT
  4,  // BUTTON_A
  5,  // BUTTON_X
  6,  // BUTTON_L
  7,  // BUTTON_R
  -1, // BUTTON_UNASSIGNED_1
  -1, // BUTTON_IS_NTT_DATAPAD
  -1, // BUTTON_UNASSIGNED_2
  -1, // BUTTON_UNASSIGNED_3
  8,  // BUTTON_NTT_0
  9,  // BUTTON_NTT_1
  10, // BUTTON_NTT_2
  11, // BUTTON_NTT_3
  12, // BUTTON_NTT_4
  13, // BUTTON_NTT_5
  14, // BUTTON_NTT_6
  15, // BUTTON_NTT_7
  16, // BUTTON_NTT_8
  17, // BUTTON_NTT_9
  18, // BUTTON_NTT_STAR
  19, // BUTTON_NTT_HASH
  20, // BUTTON_NTT_DOT
  21, // BUTTON_NTT_C
  -1, // BUTTON_NTT_UNASSIGNED_1
  22, // BUTTON_NTT_END_COM
};

const byte PIN_CLOCK = 4,
           PIN_LATCH = 3,
           PIN_DATA  = 2;

const byte COUNT_BITS_STANDARD = 16,
           COUNT_BITS_NTT      = 32;

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   23, 0,                  // Button Count, Hat Switch Count
                   true, true, false,      // X and Y, but no Z Axis
                   false, false, false,    // No Rx, Ry, or Rz
                   false, false,           // No rudder or throttle
                   false, false, false);   // No accelerator, brake, or steering

byte bitsToRead = COUNT_BITS_STANDARD;
bool hAxisHeld = false;
bool vAxisHeld = false;

void setup() {
  // initialize controller I/O
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_DATA, INPUT);

  // Initialize joystick
  Joystick.begin();
  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
}

void loop() {
  digitalWrite(PIN_CLOCK, HIGH);
  digitalWrite(PIN_LATCH, HIGH);
  delayMicroseconds(12);
  digitalWrite(PIN_LATCH, LOW);
  hAxisHeld = false;
  vAxisHeld = false;
  for (int i = 0; i < bitsToRead; i++) {
    delayMicroseconds(6);
    digitalWrite(PIN_CLOCK, LOW);

    bool state = i < bitsToRead ? !((bool)digitalRead(PIN_DATA)) : false;
    if (!inArray(i, BUTTONS_TO_IGNORE, sizeof(BUTTONS_TO_IGNORE) / sizeof(BUTTONS_TO_IGNORE[0]))) {
      Joystick.setButton(BUTTON_BIT_TO_USB_BUTTON[i], state);
    }
    else if (i == BUTTON_IS_NTT_DATAPAD) {
      if (updateBitsToRead(state)) { // returns true if NTT DATAPAD has been plugged in or unplugged. We start the whole loop over.
        break;
      }
    }
    else if (i == BUTTON_UP && state) {
      vAxisHeld = true;
      Joystick.setYAxis(-1);
    }
    else if (i == BUTTON_DOWN && !vAxisHeld) {
      Joystick.setYAxis(state ? 1 : 0);
    }
    else if (i == BUTTON_LEFT && state) {
      hAxisHeld = true;
      Joystick.setXAxis(-1);
    }
    else if (i == BUTTON_RIGHT && !hAxisHeld) {
      Joystick.setXAxis(state ? 1 : 0);
    }
    delayMicroseconds(6);
    digitalWrite(PIN_CLOCK, HIGH);
  }
}

bool updateBitsToRead(byte state) {
  byte oldBitsToRead = bitsToRead;
  bitsToRead = state ? COUNT_BITS_NTT : COUNT_BITS_STANDARD;
  if (oldBitsToRead != bitsToRead) { // If an NTT DATAPAD has been plugged in our unplugged, reset all buttons and return true
    resetButtons();
    return true;
  }
  return false;
}

void resetButtons() {
  for (int i = 0; i < 23; i++) {
    Joystick.setButton(i, 0);
  }
  Joystick.setYAxis(0);
  Joystick.setXAxis(0);
}

bool inArray(byte needle, byte haystack[], int haystackSize) {
  for (int i = 0; i < haystackSize; i++) {
    if (haystack[i] == needle) {
      return true;
    }
  }
  return false;
}
