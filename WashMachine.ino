/* 
 *  Inepro PayMatic Modification using Arduino Logic board
 *  
 * Buttons are connected to ground thus by pressing them the value becomes LOW
 * 
 * License: BSD Licence
 * 
 * Rick van der Zwet <info@rickvanderzwet.nl>
 */
#include <Arduino.h>
#include <TM1637Display.h>
#include <Timer.h>
#include <EEPROM.h>
#include <avr/eeprom.h>
#include "WashMachine.h"

// Module connection pins (Digital Pins)
#define LCD_CLK A4
#define LCD_DIO A5

#define COIN_PIN A0
#define RELAY_PIN A1

#define COUNT_UP_PIN 10
#define COUNT_DOWN_PIN 11
#define DOOR_PIN 12
#define SERVICE_PIN 13

// Amount of seconds to change if change is requested
#define COIN_ALTER_DELTA 300
#define DOOR_ALTER_DELTA 10

// The amount of time (in seconds) between updates of display
#define TIMER_CLOCK 1

// Relay state behaves diffently as expected (LOW = active) so make some shortcuts
#define ON LOW
#define OFF HIGH

#define SCHEMA 0x0002
#define SCHEMA_ADDR 0
#define SETTINGS_ADDR 1

States systemState = RUNNING;

struct settings_t
{
  uint32_t runtime_per_coin;        // Amount of time (in seconds) which is to be added per coin
  uint32_t runtime_per_doorevent;   // Amount of time (in seconds) the door should be opened if the door open button is pressed
} settings;

const uint8_t SEG_COIN[] = {
	SEG_A | SEG_D | SEG_E | SEG_F,                  // C
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
	SEG_E | SEG_F,                                  // I
	SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,          // N
	};

const uint8_t CHAR_h = SEG_C | SEG_E | SEG_F | SEG_G;

const uint32_t runtime_per_coin = 0;

// Timer function to aid in clock updates
Timer t;
// 4 digits 7-segments displa
TM1637Display lcd(LCD_CLK, LCD_DIO);

// Amount of time (in seconds) left before attached system is shutdown
int runtime = 0;
boolean lcd_dots = false;
int relayState = OFF;

/* Pin State Storage */
long lastDebounceTime = 0;
const long debounceDelay = 50;
boolean buttonState = HIGH;
boolean coinPinState = HIGH;
boolean doorPinState = HIGH;
boolean servPinState = HIGH;
boolean countUpPinState = HIGH;
boolean countDownPinState = HIGH;



void updateDisplay(int sec, boolean sep) {
  // Selectively set different digits  
  int hours, minutes, seconds;
  uint8_t data[] = {0, 0, 0, 0};
  
  if (sec == 0) {
    lcd.setSegments(SEG_COIN);
  } else { 
    hours = sec / 3600;
    minutes = (sec / 60) % 60;
    seconds = (sec % 60);
    
    // Display has 4 digits so less <1h mm:ss else h:mm
    if (sec < 3600) {
      data[0] = lcd.encodeDigit(minutes / 10);   
      data[1] = lcd.encodeDigit(minutes % 10);
      data[2] = lcd.encodeDigit(seconds / 10);
      data[3] = lcd.encodeDigit(seconds % 10);
    } else {
      data[0] = lcd.encodeDigit(hours % 10);
      data[1] = CHAR_h;
      data[2] = lcd.encodeDigit(minutes / 10);
      data[3] = lcd.encodeDigit(minutes % 10);
    }
    
    if (sep)
      data[1] |= 0b10000000;
    lcd.setSegments(data);
  }
}



// Relay output is reversed. Switched on if relay is pin is LOW
int setRelay(int state) {
   digitalWrite(RELAY_PIN, (state == OFF) ? HIGH : LOW);
  return state;
}


void updateRuntime() {
  switch (systemState) {
   case RUNNING: 
     if (runtime < TIMER_CLOCK)
       runtime = 0;
     else
       runtime -= TIMER_CLOCK;
     
     if ((runtime == 0) && (relayState == ON))
       relayState = setRelay(OFF);
     else if ((runtime > 0) && (relayState == OFF))
       relayState = setRelay(ON);
      
     lcd_dots = !lcd_dots;
     updateDisplay(runtime, lcd_dots); 
     break;
  }   
}  



void changeState(States newState) {
  systemState = newState;
  switch(newState) {
    case SERVICE_COIN:
      updateDisplay(settings.runtime_per_coin, true);
      break;
    case SERVICE_DOOR:
      updateDisplay(settings.runtime_per_doorevent, true);
      break;
  }
}



void coinEvent() {
  runtime += settings.runtime_per_coin;
  updateRuntime();
}



void doorEvent() {
  switch (systemState) {
    case RUNNING:
      // Door can only be opened if no time is left aka machine is off.
      // To avoid people pressing the button multiple times and getting a free-ride.
      if (runtime == 0) {
        runtime = settings.runtime_per_doorevent;
      } 
      break;
    case SERVICE_COIN:
      changeState(SERVICE_DOOR);
      break;
    case SERVICE_DOOR:
      changeState(SERVICE_COIN);
      break;
  }
}



void countUpEvent() {
  switch (systemState) {
    case SERVICE_COIN:
      settings.runtime_per_coin += COIN_ALTER_DELTA;
      updateDisplay(settings.runtime_per_coin, true);
      break;
    case SERVICE_DOOR:
      settings.runtime_per_doorevent += DOOR_ALTER_DELTA;
      updateDisplay(settings.runtime_per_doorevent, true);
      break;
  }
}



void countDownEvent() {
  switch (systemState) {
    case SERVICE_COIN:
      settings.runtime_per_coin -= COIN_ALTER_DELTA;
      updateDisplay(settings.runtime_per_coin, true);
      break;
    case SERVICE_DOOR:
      settings.runtime_per_doorevent -= DOOR_ALTER_DELTA;
      updateDisplay(settings.runtime_per_doorevent, true);
      break;
  }
}



void serviceEvent(boolean pinState) {
  if (pinState == LOW) {
      changeState(SERVICE_COIN);
  } else {
      // Save configuration
      eeprom_write_block((void *)&settings, (void *)SETTINGS_ADDR, sizeof(settings));
      changeState(RUNNING);
  }
}



void setup()
{
  Serial.begin(9600);
  lcd.setBrightness(0xff);
    
  t.every(TIMER_CLOCK * 1000, updateRuntime);
  
  pinMode(RELAY_PIN, OUTPUT);
  relayState = setRelay(OFF);
  
  pinMode(COIN_PIN, INPUT);
  digitalWrite(COIN_PIN, HIGH);
   
  pinMode(DOOR_PIN, INPUT);
  digitalWrite(DOOR_PIN, HIGH);
  
  pinMode(SERVICE_PIN, INPUT);
  digitalWrite(SERVICE_PIN, HIGH);
  
  pinMode(COUNT_UP_PIN, INPUT);
  digitalWrite(COUNT_UP_PIN, HIGH);
  
  pinMode(COUNT_DOWN_PIN, INPUT);
  digitalWrite(COUNT_DOWN_PIN, HIGH);
  
  
  // Avoid triggering Ax PINs on power-up
  delay(2000);
  
  // Store default values if new schema is created, no migration plan
  if (EEPROM.read(SCHEMA_ADDR) != SCHEMA) {
    EEPROM.write(SCHEMA_ADDR, SCHEMA);
    settings.runtime_per_doorevent = 120;
    settings.runtime_per_coin = 3600;
    eeprom_write_block((void *)&settings, (void *)SETTINGS_ADDR, sizeof(settings));
  } else {
    eeprom_read_block((void *)&settings, (void *)SETTINGS_ADDR, sizeof(settings));
  }
}


void loop()
{
  t.update();   
    
  // Check if coin is inserted (with debouncing of button)
  buttonState = digitalRead(COIN_PIN);
  if (buttonState != coinPinState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      coinPinState = buttonState;    
    
      if (coinPinState == LOW) {
        Serial.println("Coin inserted");
        coinEvent();
      }
    }
  }

  // Check if door open is pressed (with debouncing of button)
  buttonState = digitalRead(DOOR_PIN);
  if (buttonState != doorPinState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      doorPinState = buttonState;    
    
      if (doorPinState == LOW) {
        Serial.println("Door open time / Service Change requested");
        doorEvent();
      }
    }
  }
  
  // Check if service mode is pressed (with debouncing of button)
  buttonState = digitalRead(SERVICE_PIN);
  if (buttonState != servPinState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      servPinState = buttonState;    

      // Deal with both UP and DOWN event, since it is a key which is turned
      Serial.println((servPinState == LOW) ? "Entering Service Mode" : "Exit Service Mode");
      serviceEvent(servPinState);
    }
  }
  
  
  // Check if count down is pressed (with debouncing of button)
  buttonState = digitalRead(COUNT_DOWN_PIN);
  if (buttonState != countDownPinState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      countDownPinState = buttonState;    
    
      if (countDownPinState == LOW) {
        Serial.println("Count Down requested");
        countDownEvent();
      }
    }
  }
  
  // Check if count up is pressed (with debouncing of button)
  buttonState = digitalRead(COUNT_UP_PIN);
  if (buttonState != countUpPinState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      countUpPinState = buttonState;    
    
      if (countUpPinState == LOW) {
        Serial.println("Count Up requested");
        countUpEvent();
      }
    }
  } 

}
