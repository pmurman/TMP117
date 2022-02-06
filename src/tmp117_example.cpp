/*!
 * @author  Peter Murman
 * @date    2 Feb 2022
 * 
 * @brief   Example using TMP117 Temperature Sensor
 *
 * @license MIT License (see license.txt)
 * 
 * v1.0.0   - Initial version
 * 
 * The purpose of this example is to demonstrate the use of the TMP117 'Lite' driver functionality
 * in an IOT (multi-)sensor application. 
 * 
 * For detailed information please see README.md.
 */

#include <Arduino.h>
#include "tmp117_example.h"
#include "TMP117.h"

static uint8_t sensorCount = 0;           // keep track of # available sensors
static uint32_t sensorsServiced;          // sensor n sets bit[n] when serviced after issuing sensor ready interrupt
static int16_t temperature;
static uint32_t startTime;

TMP117 TempSensor(ADD0_TO_VCC,            // use default Bluedot configuration
                  TMP117_ALERT,           // interrupt wiring: TMP117-Alert -> SAMD21G-PA11/MUX_PA11B_ADC_AIN19
                  TempSensorReady,        // callback to handle sensor ready interrupt
                  Error                   // callback (optional)
                  );

void setup() {
  SerialUSB.begin(115200);
  while (!SerialUSB) ;
  SerialUSB.println("-- Temperature measurement using TMP117 --");

  pinMode(LED_BLUE,OUTPUT);
  digitalWrite(LED_BLUE, HIGH); // off

  // initialize sensor
  TempSensor.init(por,                    // if 1: use Power-Up Reset configuration stored in EEPROM - if 0: use supplied configuration values 
                  TMP117::shutdown,   // mode
                  TMP117::avg8,       // averaging
                  0,                      // if 1: store lowest/highest temp. values in EEPROM
                  sensorCount++           // assign unique id to sensor (0-31)
                 );
  
  // read lowest/highest temperatures stored in the sensor's EEPROM
  int16_t tempMin = TempSensor.getTemperature(T_MIN);
  int16_t tempMax = TempSensor.getTemperature(T_MAX);
  SerialUSB.print("Min/Max temperatures stored in TMP117: Tmin=");
  SerialUSB.print(tempMin * TMP117_RES, 2);
  SerialUSB.print(", Tmax=");
  SerialUSB.print(tempMax * TMP117_RES, 2);
  SerialUSB.println("°C");

// See 'README2.md'
  if (!por) {
    if (TempSensor.initPowerUpSettings())
      SerialUSB.println("Error writing configuration to TMP117 EEPROM");
    else {
      SerialUSB.println("TMP117 configuration saved in EEPROM.\n"
                        "Set 'por' to 1 in 'tmp117_example.h' and rebuild program.\n"
                        "Program ends here...");
      while (true == true) ;
    }
  }

// Delete the next line to wait a minute for the first temperature reading.
// This gives the opportunity to turn off the device and prevent lo/hi values ​​from this location
// from being written to EEPROM when the temp. measurement is to be performed at another location.
  StartTempSensor(); 
}

/**
 * Just looping and waiting, simulating timers and (deep) sleep mode
 */
static bool timerOn;
static bool sleeping;
void loop() {
  static uint32_t interval;

  // (... woke up after interrupt) check if all sensors ready
  if (sensorsServiced == All_sensors(sensorCount)) {
    // all sensors ready
    SerialUSB.print("temperature ");
    SerialUSB.print(temperature * TMP117_RES, 2);
    SerialUSB.println("°C");
    timerOn = false;
  }
  
  // do other stuff (or go into sleep mode...)
  sleeping = true;
  while (sleeping) {
    if (millis() - interval > 60 * 1000) {
      // 'wake up' for next measurement cycle
      interval = millis();
      StartTempSensor(); // start next conversion
    }

    if (timerOn && (millis() - startTime > 200)) { // timeout after 200ms (TMP117 conversion takes 124ms in this example)
      // 'wake up' after timeout
      timerOn = false;
      Error(E_NO_DATA);
      digitalWrite(LED_BLUE, HIGH); // off
    }
  }
}

/**
 * Start TMP117 temperature conversion
 */
void StartTempSensor(void) {
  TempSensor.startConversion();
  startTime = millis();
  timerOn = true; // start timer
  digitalWrite(LED_BLUE, LOW); // on
}

/**
 * TMP117 Data Ready interrupt - read sensor data
 */
void TempSensorReady(void) {
  sleeping = false;
  temperature = TempSensor.readSensor(&sensorsServiced);
  digitalWrite(LED_BLUE, HIGH); // off
}

/**
 * Error handling
 */
void Error(nodeError_t e) {
  switch (e) {
  // place sensor related errors here, referred to by sensor#
    case E_S0: // E_S1 ... etc.
      SerialUSB.print("Error at sensor ");
      SerialUSB.println(e);
      break;
  
  // other errors
    case E_NO_DATA:
      SerialUSB.print("No sensor data - error status: ");
      SerialUSB.println(sensorsServiced ^ All_sensors(sensorCount), BIN);
      TempSensor.softReset(); // small chance this will solve the problem...
      break;
  }
}