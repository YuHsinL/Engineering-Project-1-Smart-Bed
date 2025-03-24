/*
Circuit:
  * DS3231:
    RTC GND pin to Arduino GND
    RTC VCC pin to Arduino 5v
    RTC SDA pin to Arduino A4
    RTC SCL pin to Arduino A5
  * LCD:
    RTC GND pin to Arduino GND
    RTC VCC pin to Arduino 5v
    RTC SDA pin to Arduino A4
    RTC SCL pin to Arduino A5
  * Motor Controller:
    E1 to Arduino D6
    M1 to Arduino D7
    E2 to Arduino D5
    M2 to Arduino D4
  * Buttons:
    Set Time Button to Arduino D11
    Set Alarm Button to Arduino D10
    Add Button to Arduino D9
    Minus Button to Arduino D8

*/

#include <Wire.h>
#include <DS3231.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 2);

DS3231 myRTC;
bool h12Flag = false;
bool pmFlag = false;
byte hour, minute, aHour, aMinute;
byte A2Day, A2Hour, A2Minute, AlarmBits;
bool A2Dy, A2h12, A2PM;

const int SetTimeButtonPin = 11;
const int SetAlarmButtonPin = 10;
const int AddButtonPin = 9;
const int MinusButtonPin = 8;

int SetTimeButtonState = 0;
int SetAlarmButtonState = 0;
int AddButtonState = 0;
int MinusButtonState = 0;

bool setTimeHour = false;
bool setTimeMinute = false;
bool setAlarmHour = false;
bool setAlarmMinute = false;

const int E1 = 6;
const int M1 = 7;
const int E2 = 5;
const int M2 = 4;

bool startMotor = false;

void setup() {

  Wire.begin();

  // Initialize LCD
  lcd.begin();
  lcd.backlight();

  // Setup button pin
  pinMode(SetTimeButtonPin, INPUT);
  pinMode(SetAlarmButtonPin, INPUT);
  pinMode(AddButtonPin, INPUT);
  pinMode(MinusButtonPin, INPUT);

  // Initialize RTC
  myRTC.setClockMode(false);

  // Setup motor controller pin
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);

	// Start the serial interface for debugging
	Serial.begin(57600);

}

void loop() {

  // Read button state
  SetTimeButtonState = digitalRead(SetTimeButtonPin);
  SetAlarmButtonState = digitalRead(SetAlarmButtonPin);
  AddButtonState = digitalRead(AddButtonPin);
  MinusButtonState = digitalRead(MinusButtonPin);

  if (SetTimeButtonState == HIGH) {
    Serial.println("SetTimeButtonState is HIGH");
    if (setAlarmHour == false && setAlarmMinute == false) {
      if (setTimeHour == false && setTimeMinute == false) {
        setTimeHour = true;
      } else if (setTimeHour == true && setTimeMinute == false) {
        setTimeHour = false;
        setTimeMinute = true;
      } else if (setTimeHour == false && setTimeMinute == true) {
        setTimeMinute = false;
      }
    }
    delay(500);
  }

  if (SetAlarmButtonState == HIGH) {
    Serial.println("SetAlarmButtonState is HIGH");
    if (setTimeHour == false && setTimeMinute == false) {
      if (setAlarmHour == false && setAlarmMinute == false) {
        setAlarmHour = true;
      } else if (setAlarmHour == true && setAlarmMinute == false) {
        setAlarmHour = false;
        setAlarmMinute = true;
      } else if (setAlarmHour == false && setAlarmMinute == true) {
        setAlarmMinute = false;
      }
    }
    delay(500);
  }

  // Read RTC
  hour = myRTC.getHour(h12Flag, pmFlag);
  minute = myRTC.getMinute();
  myRTC.getA2Time(A2Day, aHour, aMinute, AlarmBits, A2Dy, A2h12, A2PM);

  if (AddButtonState == HIGH) {
    Serial.println("AddButtonState is HIGH");
    if (setTimeHour == true) {
      if (hour == 23) { hour = 0; } else { hour++; }
    } else if (setTimeMinute == true) {
      if (minute == 59) { minute = 0; } else { minute++; }
    } else if (setAlarmHour == true) {
      if (aHour == 23) { aHour = 0; } else { aHour++; }
    } else if (setAlarmMinute == true) {
      if (aMinute == 59) { aMinute = 0; } else { aMinute++; }
    }
  }

  if (MinusButtonState == HIGH) {
    Serial.println("MinusButtonState is HIGH");
    if (setTimeHour == true) {
      if (hour == 0) { hour = 23; } else { hour--; }
    } else if (setTimeMinute == true) {
      if (minute == 0) { minute = 59; } else { minute--; }
    } else if (setAlarmHour == true) {
      if (aHour == 0) { aHour = 23; } else { aHour--; }
    } else if (setAlarmMinute == true) {
      if (aMinute == 0) { aMinute = 59; } else { aMinute--; }
    }
  }

  myRTC.setHour(hour);
  myRTC.setMinute(minute);
  myRTC.setA2Time(0, aHour, aMinute, 0b01000000, true, false, false);

  // Show time and alarm on lcd
  lcd.setCursor(0, 0);
  if (myRTC.getHour(h12Flag, pmFlag) < 10) {
    lcd.print("0");
    lcd.print(myRTC.getHour(h12Flag, pmFlag), DEC);
  } else {
    lcd.print(myRTC.getHour(h12Flag, pmFlag), DEC);
  }
  lcd.print(":");
  if (myRTC.getMinute() < 10) {
    lcd.print("0");
    lcd.print(myRTC.getMinute(), DEC);
  } else {
    lcd.print(myRTC.getMinute(), DEC);
  }
  
  myRTC.getA2Time(A2Day, A2Hour, A2Minute, AlarmBits, A2Dy, A2h12, A2PM);

  lcd.setCursor(0, 1);
  lcd.print("Alarm ");
  if (A2Hour < 10) {
    lcd.print("0");
    lcd.print(A2Hour);
  } else {
    lcd.print(A2Hour);
  }
  lcd.print(":");
  if (A2Minute < 10) {
    lcd.print("0");
    lcd.print(A2Minute);
  } else {
    lcd.print(A2Minute);
  }

  if (setTimeHour) {
    lcd.setCursor(1, 0);
    lcd.cursor();
  } else if (setTimeMinute) {
    lcd.setCursor(4, 0);
    lcd.cursor();
  } else if (setAlarmHour) {
    lcd.setCursor(7, 1);
    lcd.cursor();
  } else if (setAlarmMinute) {
    lcd.setCursor(10, 1);
    lcd.cursor();
  } else {
    lcd.noCursor();
  }

  startMotor = false;
  if (myRTC.getHour(h12Flag, pmFlag) == A2Hour && myRTC.getMinute() == A2Minute) {
    startMotor = true;
    Serial.write("Motor Activated");
  }

  // Start motor
  if (startMotor == true) {
    digitalWrite(M1, HIGH);
    digitalWrite(M2, HIGH);
    analogWrite(E1, 200);
    analogWrite(E2, 200);
  } else {
    analogWrite(E1, 0);
    analogWrite(E2, 0);
  }
  
  delay(200);

}
