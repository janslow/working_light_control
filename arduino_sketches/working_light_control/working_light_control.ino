/*
 *  Working Light Control for O'Reilly Theatre
 *  Copyright (C) 2014 Jay Anslow (jay@anslow.me.uk)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// Mappings between pins and components.
#define CONTACT_ON_PIN 7
#define CONTACT_OFF_PIN 8
#define LED_ON_PIN 6
#define LED_OFF_PIN 5
#define LED_POWER_PIN 9
#define SWITCH_ON_PIN 2
#define SWITCH_OFF_PIN 3

// External Interrupts
#define ON_INTERRUPT 0
#define OFF_INTERRUPT 1

// LED values (0 is full brightness, 255 is off)
#define LED_ON_VALUE_FULL 0
#define LED_ON_VALUE_DIM 100
#define LED_ON_VALUE_OFF 255
#define LED_OFF_VALUE_FULL 0
#define LED_OFF_VALUE_DIM 240
#define LED_OFF_VALUE_OFF 255
#define LED_POWER_VALUE_FULL 0
#define LED_POWER_VALUE_DIM 220
#define LED_POWER_VALUE_OFF 255

// Program States
#define STATE_NEUTRAL 0
#define STATE_ON_TEMPORARY 1
#define STATE_ON_LOCKING 2
#define STATE_ON_LOCKED 3
#define STATE_ON_WARNING 4
#define STATE_ON_EXPIRING 5
#define STATE_OFF_TEMPORARY -1
#define STATE_OFF_LOCKING -2
#define STATE_OFF_LOCKED -3
#define STATE_OFF_WARNING -4
#define STATE_OFF_EXPIRING -5

// Times
#define TIME_LOCKING 3000
#define TIME_WARNING 15000
#define TIME_EXPIRING 5000
#define TIME_EXPIRY 3000

// Flash/Relay intervals
#define INTERVAL_CONTACT 20
#define INTERVAL_LED_WARNING 1000
#define INTERVAL_LED_EXPIRING 100

volatile int state;
volatile unsigned long stateTime;

void setup() {
  pinMode(CONTACT_ON_PIN, OUTPUT);
  pinMode(CONTACT_OFF_PIN, OUTPUT);
  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_OFF_PIN, OUTPUT);
  pinMode(LED_POWER_PIN, OUTPUT);
  
  pinMode(SWITCH_ON_PIN, INPUT);
  pinMode(SWITCH_OFF_PIN, INPUT);
  
  Serial.begin(9600);
  Serial.println("Starting");
  
  analogWrite(LED_POWER_PIN, LED_POWER_VALUE_FULL);
  analogWrite(LED_ON_PIN, LED_ON_VALUE_FULL);
  analogWrite(LED_OFF_PIN, LED_OFF_VALUE_FULL);
  
  delay(1000);
  
  analogWrite(LED_POWER_PIN, LED_POWER_VALUE_DIM);
  analogWrite(LED_ON_PIN, LED_ON_VALUE_OFF);
  analogWrite(LED_OFF_PIN, LED_OFF_VALUE_OFF);
  
  state = STATE_NEUTRAL;
  attachInterrupt(ON_INTERRUPT, inputChangeHandler, CHANGE);
  attachInterrupt(OFF_INTERRUPT, inputChangeHandler, CHANGE);
  
  Serial.println("Ready");
}

void printState(unsigned long now) { 
  Serial.print(now);
  Serial.print(": ");
  Serial.print(state);
  Serial.print(" @ ");
  Serial.print(stateTime);
  Serial.println("ms");
}
void loop() {
  unsigned long now = millis();
  
  // Update the current state if necessary
  // ON_TEMPORARY --wait(TIME_LOCKING)-> ON_LOCKING
  if (state == STATE_ON_TEMPORARY && (stateTime + TIME_LOCKING) <= now) {
    setState(STATE_ON_LOCKING);
  // ON_LOCKED --wait(TIME_WARNING)-> ON_WARNING
  } else if (state == STATE_ON_LOCKED && (stateTime + TIME_WARNING) <= now) {
    setState(STATE_ON_WARNING);
  } // ON_WARNING --wait(TIME_EXPIRING)-> ON_EXPIRING
  else if (state == STATE_ON_WARNING && (stateTime + TIME_EXPIRING) <= now)
    setState(STATE_ON_EXPIRING);
  
  // OFF_TEMPORARY --wait(TIME_LOCKING)-> OFF_LOCKING
  else if (state == STATE_OFF_TEMPORARY && (stateTime + TIME_LOCKING) <= now)
    setState(STATE_OFF_LOCKING);
  // OFF_LOCKED --wait(TIME_WARNING)-> OFF_WARNING
  else if (state == STATE_OFF_LOCKED && (stateTime + TIME_WARNING) <= now)
    setState(STATE_OFF_WARNING);
  // OFF_WARNING --wait(TIME_EXPIRING)-> OFF_EXPIRING
  else if (state == STATE_OFF_WARNING && (stateTime + TIME_EXPIRING) <= now)
    setState(STATE_OFF_EXPIRING);
    
  // EXPIRING --wait(TIME_EXPIRY)-> NEUTRAL
  else if ((state == STATE_ON_EXPIRING || state == STATE_OFF_EXPIRING) && stateTime + TIME_EXPIRY <= now)
    setState(STATE_NEUTRAL);
  
  // Set the output values depending on the current state.
  switch (state) {
    // Do nothing.
    case STATE_NEUTRAL:
      setOuts(LED_ON_VALUE_OFF, LOW, LED_OFF_VALUE_OFF, LOW);
      break;

    // "On" states.
    case STATE_ON_TEMPORARY:
      setOuts(LED_ON_VALUE_FULL, HIGH, LED_OFF_VALUE_OFF, LOW);
      break;
    case STATE_ON_LOCKING:
      setOuts(LED_ON_VALUE_DIM, clock(INTERVAL_CONTACT) ? 1 : 0, LED_OFF_VALUE_OFF, LOW);
      break;
    case STATE_ON_LOCKED:
      setOuts(LED_ON_VALUE_DIM, clock(INTERVAL_CONTACT) ? 1 : 0, LED_OFF_VALUE_OFF, LOW);
      break;
    case STATE_ON_WARNING:
      setOuts(clock(INTERVAL_LED_WARNING) ? LED_ON_VALUE_FULL : LED_ON_VALUE_DIM, clock(INTERVAL_CONTACT), LED_OFF_VALUE_OFF, LOW);
      break;
    case STATE_ON_EXPIRING:
      setOuts(clock(INTERVAL_LED_EXPIRING) ? LED_ON_VALUE_FULL : LED_ON_VALUE_DIM, clock(INTERVAL_CONTACT), LED_OFF_VALUE_OFF, LOW);
      break;
     
    // "Off" states.
    case STATE_OFF_TEMPORARY:
      setOuts(LED_ON_VALUE_OFF, LOW, LED_OFF_VALUE_FULL, HIGH);
      break;
    case STATE_OFF_LOCKING:
      setOuts(LED_ON_VALUE_OFF, LOW, LED_OFF_VALUE_DIM, clock(INTERVAL_CONTACT) ? 1 : 0);
      break;
    case STATE_OFF_LOCKED:
      setOuts(LED_ON_VALUE_OFF, LOW, LED_OFF_VALUE_DIM, clock(INTERVAL_CONTACT) ? 1 : 0);
      break;
    case STATE_OFF_WARNING:
      setOuts(LED_ON_VALUE_OFF, LOW, clock(INTERVAL_LED_WARNING) ? LED_OFF_VALUE_FULL : LED_OFF_VALUE_DIM, clock(INTERVAL_CONTACT));
      break;
    case STATE_OFF_EXPIRING:
      setOuts(LED_ON_VALUE_OFF, LOW, clock(INTERVAL_LED_EXPIRING) ? LED_OFF_VALUE_FULL : LED_OFF_VALUE_DIM, clock(INTERVAL_CONTACT));
      break;
  }
  
  // Flash LEDs in expiring states.
  if (state == STATE_ON_EXPIRING || state == STATE_OFF_EXPIRING)
    analogWrite(LED_POWER_PIN, clock(INTERVAL_LED_EXPIRING) ? LED_POWER_VALUE_FULL : LED_POWER_VALUE_DIM);
  else
    analogWrite(LED_POWER_PIN, LED_POWER_VALUE_DIM);
  
}

// Produce an oscillator of period = 2*interval
boolean clock(int interval) {
  return (millis() % (2 * interval)) > interval;
}

// Update the current state.
void setState(int newState) {
  state = newState;
  stateTime = millis();
}

// Set the LED and relay outputs.
void setOuts(int ledOn, int contactOn, int ledOff, int contactOff) {
  analogWrite(LED_ON_PIN, ledOn);
  analogWrite(LED_OFF_PIN, ledOff);
  digitalWrite(CONTACT_ON_PIN, contactOn);
  digitalWrite(CONTACT_OFF_PIN, contactOff);
}

// Handle a button being pressed.
void inputChangeHandler() {
  boolean on = digitalRead(SWITCH_ON_PIN), off = digitalRead(SWITCH_OFF_PIN);
  // If NEUTRAL or any OFF and inputOn then go to ON_TEMPORARY
  if (on && state <= STATE_NEUTRAL)
    setState(STATE_ON_TEMPORARY);
  // If NEUTRAL or any ON and inputOff then go to OFF_TEMPORARY
  else if (off && state >= STATE_NEUTRAL)
    setState(STATE_OFF_TEMPORARY);
  // If TEMPORARY and input is released, go to NEUTRAL
  else if ((!on && state == STATE_ON_TEMPORARY) || (!off && state == STATE_OFF_TEMPORARY))
      setState(STATE_NEUTRAL);
  // Else if in any ON
  else if (state > STATE_NEUTRAL) {
    if (on && state > STATE_ON_LOCKING)
      setState(STATE_ON_LOCKING);
    else if (!on && state == STATE_ON_LOCKING)
      setState(STATE_ON_LOCKED);
  } else if (state < STATE_NEUTRAL) {
    if (off && state < STATE_OFF_LOCKING)
      setState(STATE_OFF_LOCKING);
    else if (!off && state == STATE_OFF_LOCKING)
      setState(STATE_OFF_LOCKED);
  }
}
