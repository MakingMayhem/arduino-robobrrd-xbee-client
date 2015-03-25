// 
// RoboBrrd_XBee_Client.ino
// Copyright (C) 2014  David L Kinney <david@kinney.io>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION

// Flag to control whether XBee serial communication uses Serial or 
// SoftwareSerial. It is useful to use SoftwareSerial (e.g., with the SparkFun 
// XBee Shield) while debugging so that status messages can be printed to the 
// USB console. It is recommended that the hardware-supported Serial interface 
// be used after debugging is complete. 
// 
// 0 -> XBee uses Serial (pins 0,1)
// 1 -> XBee uses SoftwareSerial (pins 2,3)
#define XBEE_SOFTWARE_SERIAL 0

// -- SERVO VALUES -- //

#define WING_RIGHT_UP       40
#define WING_RIGHT_MIDDLE   90
#define WING_RIGHT_DOWN    107

#define WING_LEFT_UP       125
#define WING_LEFT_MIDDLE    75
#define WING_LEFT_DOWN      67

#define BEAK_OPEN           80
#define BEAK_CLOSED        167

////////////////////////////////////////////////////////////////////////////////
// INCLUDES

// Standard Arduino libraries
#include <Servo.h>

// Third-party libraries
#include <XBee.h>

////////////////////////////////////////////////////////////////////////////////
// CONSTANTS - PINS

// -- SERVOS -- //

// label your servo pin numbers here
#define PIN_WING_RIGHT  8
#define PIN_WING_LEFT   9
#define PIN_BEAK       10

// -- LED PINS -- //

#define PIN_LED  13

// the rgb pins of your eyes

#define PIN_EYES_RED    3
#define PIN_EYES_GREEN  5
#define PIN_EYES_BLUE   6

// -- MISC -- //

#define PIN_SPEAKER       7
#define PIN_LDR_LEFT     A0 // LDR = light dependent resistor
#define PIN_LDR_RIGHT    A1 // LDR = light dependent resistor
#define PIN_TEMPERATURE  A2

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES

Servo rightWingServo, leftWingServo, beakServo;

XBee xbee = XBee();
ZBRxResponse zbRx = ZBRxResponse();

////////////////////////////////////////////////////////////////////////////////
// SAMPLE COMMAND STREAM

// -- Exercises each command -- //

//static uint8_t const sample[] = {
//  // PLAY SHORT SONG
//  0x02, 0x01, 0x04, 0x00, 0x46, // PLAY TONE: 260Hz for 70ms
//  0x02, 0x01, 0x18, 0x00, 0x46, // PLAY TONE: 280Hz for 70ms
//  0x02, 0x01, 0x2c, 0x00, 0x46, // PLAY TONE: 300HZ for 70ms
//  0xff, 0x00, 0x64,       // DELAY: 100ms
//  0x02, 0x01, 0x04, 0x00, 0x46, // PLAY TONE: 260Hz for 70ms
//  0x02, 0x01, 0x18, 0x00, 0x46, // PLAY TONE: 280Hz for 70ms
//  0x02, 0x01, 0x2c, 0x00, 0x46, // PLAY TONE: 300HZ for 70ms
//  0xff, 0x00, 0x64,       // DELAY: 100ms
//  0x02, 0x01, 0x04, 0x00, 0x46, // PLAY TONE: 260Hz for 70ms
//  0x02, 0x01, 0x18, 0x00, 0x46, // PLAY TONE: 280Hz for 70ms
//  0x02, 0x01, 0x2c, 0x00, 0x46, // PLAY TONE: 300HZ for 70ms
//  0xff, 0x00, 0x64,       // DELAY: 100ms
//  
//  // CYCLE EYES RED, GREEN, BLUE
//  0x01, 0xff, 0x00, 0x00, // EYE COLOR: 0xff0000
//  0xff, 0x01, 0xf4,       // DELAY: 500ms
//  0x01, 0x00, 0xff, 0x00, // EYE COLOR: 0x00ff00
//  0xff, 0x01, 0xf4,       // DELAY: 500ms
//  0x01, 0x00, 0x00, 0xff, // EYE COLOR: 0x0000ff
//  0xff, 0x01, 0xf4,       // DELAY: 500ms
//  0x01, 0x00, 0x00, 0x00, // EYE COLOR: off
//  
//  // BLINK ARDUINO LED 3 TIMES
//  0x03, 0x03, 0x01, 0x4d, // BLINK LED: 3 times for 333ms each time
//  
//  // SHOW EXCITEMENT
//  0x10, 0xff,             // SET BEAK: full open
//  0x1e, 0xff,             // LEFT WING UP
//  0x1f, 0x00,             // RIGHT WING DOWN
//  0xff, 0x00, 0xfa,       // DELAY: 250 ms
//  0x1e, 0x00,             // LEFT WING DOWN
//  0x1f, 0xff,             // RIGHT WING UP
//  0xff, 0x00, 0xfa,       // DELAY: 250 ms
//  0x10, 0x00,             // SET BEAK: full closed
//  0x1c, 0x1d,             // LEFT WING MIDDLE, RIGHT WING MIDDLE
//  0xff, 0x03, 0xe8,       // DELAY: 1000 ms
//  0x10, 0x7f,             // SET BEAK: half open
//  0xe0, 0x04,             // FLAP WINGS: 4 times
//  0x10, 0x00,             // SET BEAK: full closed
//};
//static uint8_t const sampleLength = sizeof(sample);

////////////////////////////////////////////////////////////////////////////////
// SETUP & LOOP

void setup() {
  // -- XBee
  Serial.begin(9600);
  xbee.setSerial(Serial);
  
  // -- servos
  rightWingServo.attach(PIN_WING_RIGHT);
  leftWingServo.attach(PIN_WING_LEFT);
  beakServo.attach(PIN_BEAK); 
  
  // -- LEDs
  pinMode(PIN_LED, OUTPUT);
  
  // -- eyes
  pinMode(PIN_EYES_RED, OUTPUT);
  pinMode(PIN_EYES_GREEN, OUTPUT);
  pinMode(PIN_EYES_BLUE, OUTPUT);

  // -- misc
  pinMode(PIN_SPEAKER, OUTPUT);
  
  // -- sensors
  pinMode(PIN_LDR_LEFT, INPUT);
  pinMode(PIN_LDR_RIGHT, INPUT);
  pinMode(PIN_TEMPERATURE, INPUT);
  
  // -- initial state
  setBeak(0);
  centerLeftWing();
  centerRightWing();
}

void loop() {
  xbee.readPacket(1000);
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // copy data out of xbee object because next call to readPacket() destroys it
      xbee.getResponse().getZBRxResponse(zbRx);
      uint8_t dataLength = zbRx.getDataLength();
      uint8_t *data = zbRx.getData();
      
      // process data
      uint8_t commandIndex = 0;
      processCommandStream(data, dataLength, &commandIndex);
      
      // prepare zbRx for reuse
      zbRx.reset();
    }
  } else if (xbee.getResponse().isError()) {
    // TODO: handle error somehow
  }
}

////////////////////////////////////////////////////////////////////////////////
// ROBOBRRD COMMANDS

// -- Eyes -- //

void changeEyeColor(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(PIN_EYES_RED,   red);
  analogWrite(PIN_EYES_GREEN, green);
  analogWrite(PIN_EYES_BLUE,  blue);
}

// -- LED -- //

void blinkLED(uint8_t repetitions, uint16_t duration) {
  uint16_t halfDuration = duration >> 1;
  for (int i = 0; i < repetitions; ++i) {
    digitalWrite(PIN_LED, HIGH);
    delay(halfDuration);
    digitalWrite(PIN_LED, LOW);
    delay(halfDuration);
  }
}

// -- Speaker -- //

void playTone(uint16_t frequency, uint16_t duration) {
  tone(PIN_SPEAKER, frequency, duration);
  delay(duration);
  noTone(PIN_SPEAKER);
}

// -- Beak -- //

void setBeak(uint8_t value) {
  static int16_t const beakTotalAngle = BEAK_OPEN - BEAK_CLOSED;
  int16_t angle = BEAK_CLOSED + (beakTotalAngle * value / 255);
  beakServo.write(angle);
}

// -- Left Wing -- //

void centerLeftWing() {
  leftWingServo.write(WING_LEFT_MIDDLE);
}

void setLeftWing(uint8_t value) {
  static int16_t const wingTotalAngle = WING_LEFT_UP - WING_LEFT_DOWN;
  int16_t angle = WING_LEFT_DOWN + (wingTotalAngle * value / 255);
  leftWingServo.write(angle);
}

// -- Right Wing -- //

void centerRightWing() {
  rightWingServo.write(WING_RIGHT_MIDDLE);
}

void setRightWing(uint8_t value) {
  static int16_t const wingTotalAngle = WING_RIGHT_UP - WING_RIGHT_DOWN;
  int16_t angle = WING_RIGHT_DOWN + (wingTotalAngle * value / 255);
  rightWingServo.write(angle);
}

// -- Compound Operations -- //

void flapWings(uint8_t repetitions) {
  for (uint8_t i = 0; i < repetitions; ++i) {
    leftWingServo.write(WING_LEFT_UP);
    rightWingServo.write(WING_RIGHT_UP);
    
    delay(333);
    
    leftWingServo.write(WING_LEFT_DOWN);
    rightWingServo.write(WING_RIGHT_DOWN);
    
    delay(333);
  }
}

////////////////////////////////////////////////////////////////////////////////
// COMMAND PROCESSING

// -- Helpers to read values from buffer -- //

uint8_t _readUInt8(uint8_t const * const buffer, 
                   uint8_t const length, 
                   uint8_t *indexPtr) {
    // sanity checks
    if (NULL == buffer) return 0;
    if (*indexPtr > length - 1) return 0;  // there must be 1 more bytes to read
    
    uint8_t value = buffer[*indexPtr];
    ++ *indexPtr;
    
    return value;
}

uint16_t _readUInt16(uint8_t const * const buffer, 
                     uint8_t const length, 
                     uint8_t *indexPtr) {
    // sanity checks
    if (NULL == buffer) return 0;
    if (*indexPtr > length - 2) return 0;  // there must be 2 more bytes to read
    
    uint8_t upper8 = buffer[*indexPtr];
    uint8_t lower8 = buffer[*indexPtr + 1];
    *indexPtr += 2;
    
    uint16_t upperBits = (uint16_t)upper8;
    uint16_t lowerBits = (uint16_t)lower8;
    
    uint16_t value = (upperBits << 8) + lowerBits;
    
    return value;
}

// -- Handlers for commands that take arguments -- //

void _handleBlinkLED(uint8_t const * const buffer, 
                     uint8_t const length, 
                     uint8_t *indexPtr) {
  uint8_t repetitions = _readUInt8(buffer, length, indexPtr);
  uint16_t duration = _readUInt16(buffer, length, indexPtr);
  blinkLED(repetitions, duration);
}

void _handleChangeEyeColor(uint8_t const * const buffer, 
                           uint8_t const length, 
                           uint8_t *indexPtr) {
    uint8_t r = _readUInt8(buffer, length, indexPtr);
    uint8_t g = _readUInt8(buffer, length, indexPtr);
    uint8_t b = _readUInt8(buffer, length, indexPtr);
    changeEyeColor(r, g, b);
}

void _handleDelay(uint8_t const * const buffer, 
                  uint8_t const length, 
                  uint8_t *indexPtr) {
    uint16_t millis = _readUInt16(buffer, length, indexPtr);
    delay(millis);
}

void _handleFlapWings(uint8_t const * const buffer, 
                      uint8_t const length, 
                      uint8_t *indexPtr) { 
  uint8_t repetitions = _readUInt8(buffer, length, indexPtr);
  flapWings(repetitions);
}

void _handlePlayTone(uint8_t const * const buffer, 
                     uint8_t const length, 
                     uint8_t *indexPtr) {
  uint16_t frequency = _readUInt16(buffer, length, indexPtr);
  uint16_t duration  = _readUInt16(buffer, length, indexPtr);
  playTone(frequency, duration);
}

void _handleSetBeak(uint8_t const * const buffer, 
                    uint8_t const length, 
                    uint8_t *indexPtr) {
    uint8_t value = _readUInt8(buffer, length, indexPtr);
    setBeak(value);
}

void _handleSetLeftWing(uint8_t const * const buffer, 
                        uint8_t const length, 
                        uint8_t *indexPtr) {
    uint8_t value = _readUInt8(buffer, length, indexPtr);
    setLeftWing(value);
}

void _handleSetRightWing(uint8_t const * const buffer, 
                         uint8_t const length, 
                         uint8_t *indexPtr) {
    uint8_t value = _readUInt8(buffer, length, indexPtr);
    setRightWing(value);
}

// -- Command processing -- //

void processCommand(uint8_t const * const buffer, 
                    uint8_t const length, 
                    uint8_t *indexPtr) {
    // sanity checks
    if (NULL == buffer) return;
    if (*indexPtr > length - 1) return;  // there must be 1 more bytes to read
    
    uint8_t command = _readUInt8(buffer, length, indexPtr);
    
    switch (command) {
      case 0x00:
        // NOOP
        break;
        
      case 0x01: 
        _handleChangeEyeColor(buffer, length, indexPtr);
        break;
      
      case 0x02: 
        _handlePlayTone(buffer, length, indexPtr);
        break;
        
      case 0x03:
        _handleBlinkLED(buffer, length, indexPtr);
        break;
        
      case 0x10:
        _handleSetBeak(buffer, length, indexPtr);
        break;
        
      case 0x1c:
        centerLeftWing();
        break;
        
      case 0x1d: 
        centerRightWing();
        break;
        
      case 0x1e: 
        _handleSetLeftWing(buffer, length, indexPtr);
        break;
        
      case 0x1f: 
        _handleSetRightWing(buffer, length, indexPtr);
        break;
        
      case 0xe0: 
        _handleFlapWings(buffer, length, indexPtr);
        break;
        
      case 0xff: 
        _handleDelay(buffer, length, indexPtr);
        break;
        
      default:
        break;
    }
}

void processCommandStream(uint8_t const * const buffer, 
                          uint8_t const length, 
                          uint8_t *indexPtr) {
  while (*indexPtr < length) {
    processCommand(buffer, length, indexPtr);
  }
}

