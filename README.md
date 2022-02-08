# TMP117 Driver 'Lite' - Example

Purpose of this example is to demonstrate the use of the TMP117 driver 'Lite' in a (multi-)sensor application.

The TMP117 driver is a compact, powerful, light-weight and power-efficient software library for the Texas Instruments TMP117
high-accuracy, low-power, digital temperature sensor with I<sup>2</sup>C interface.

The TMP117 Driver 'Lite' provides basic functionality for temperature measurements at minimum power consumption.
TMP117 Lite is inspired by [Nils Minor's TMP117-Arduino](https://github.com/NilsMinor/TMP117-Arduino).

## TMP117 'Lite' Features

- Uses shutdown mode to minimize power consumption (250nA)
- Supports Power-On Reset configuration, eliminating MCU/I<sup>2</sup>C initialization overhead;
- Interrupt-driven sensor data handling through Data Ready callback;
- Provides 16-bit actual, lowest and highest temperature data with a resolution of 7.8125 m°C per increment
- Temperature data available after a One-Shot conversion:
  - Actual temperature
  - Lowest / Highest temperature(s), when changed
- Capturing lowest *'ever'* and highest *'ever'* temperatures in the device's EEPROM
- Error feedback after EEPROM write failure

## Example Program

The example uses delay loops instead of (HW) timers and sleep mode. Using timers and
sleep mode are highly recommended to take advantage of the low-power capabilities of TMP117 Lite.
  
Hardware used for this example:

- BlueDot TMP117 sensor board (comes with on-board 4.7k pullups on all signal pins)
- SODAQ SFF dev board (SAMD21G18 MCU with ARM M0+)

Signal wiring between BlueDot and SODAQ board:

- BlueDot TMP117 I2C <==> SODAQ SDA/SCL
- BlueDot TMP117 Alert ---> SODAQ A11

## Initialization

Two initialization functions are available:

```cpp
  <sensor>.initSetup(
                TMP117_mod mode,
                TMP117_avg averaging,
                const bool save_min_max_in_eeprom,
                uint8_t sensor_id
                )
```

```cpp
  <sensor>.init(
                const bool save_min_max_in_eeprom,
                uint8_t sensor_id
                )
```

|  *function (parameter)*  | *description*
---------------------------|-------------------------------------------------------------------------------
| `initSetup()`            | perform SW setup of TMP117 using `mode` and `averaging` values
| `init()`                 | use Power-Up Reset configuration stored in EEPROM, no CPU/I<sup>2</sup>C overhead
| `mode`                   | recommended mode: TMP117::shutdown
| `averaging`              | recommended value: TMP117::avg8
| `save_min_max_in_eeprom` | (only after sensor read-out by `<sensor>.readSensor()`)
| | 0: do not update lowest/highest temperatures in EEPROM
| | 1: update lowest/highest temperatures in EEPROM, when changed > 0.047°C
| `sensor_id`              | assign id to sensor (0-31)

### Power-On Reset Programming

The TMP117 has the ability to store a Power-Up Reset (POR) setting in its EEPROM. This POR value is loaded into the
configuration register during the TMP117 power-up cycle and does not require initialization by software.  
To program a POR configuration, initialize TMP117 with the desired setting and store this in EEPROM:

```cpp
  // Set TMP117 to shutdown-and-avg8 mode, do not write lo/hi temperature to EEPROM, sensor ID = 0
  <sensor>.initSetup(TMP117::shutdown, TMP117::avg8, 0, 0)

  // write TMP117 configuration to EEPROM, reset lo/hi EEPROM registers to factory values
  // returns true on failure, false if ok
  <sensor>.initPowerUpSettings()                  
```

When done, change this to:

```cpp
  // use POR initialization only (mode/averaging are ignored), no EEPROM updates of lo/hi temperature, sensor ID = 0
  <sensor>.init(0, 0)
```

POR programming using the Example program:

1. In `tmp117_example.cpp`, make sure `programTMP117` is set to `true` and build, load and run `tmp117_example`
2. Program prints out the lowest/highest temperature values before they are reset and stops after programming the EEPROM  (`"... Program ends here..."`).
3. Change the value of `programTMP117` to `false` and build, load and run the example again. No SW initialization is done; the TMP117 uses the stored POR configuration setting.

### Capturing Lowest/Highest Temperatures

Driver initialization:

```cpp
  <sensor>.init(1, 0) // enable writing lowest/highest temperature to EEPROM
  //            |  |
  //            |  +-- sensor ID
  //            +-- enable write lo/hi temp to EEPROM

```

The driver keeps track of the lowest and highest temperatures. It also stores these values in TMP117 EEPROM when
`save_min_max_in_eeprom` is `1` and values have changed at least 0.047°C (to limit the number of EEPROM writes).
This feature can be used to capture the lowest and highest temperatures over a longer measurement period
without requiring any external data logging.

Before enabeling EEPROM writes, make sure the lowest/highest EEPROM values have been reset first.  
A reset is done by `<sensor>.initPowerUpSettings()` (above). When the measurements must be done at another location, make sure
the device is turned off/unplugged before temperature measurement begins after enabling writing lo/hi to EEPROM.  
Otherwise the reset values of lo/hi will be overwritten by the temperature at the programming location causing
lo/hi readings at the measurement site become useless.  
This can be avoided by adding a 10 second delay before starting the temperature readings.
