#if !defined TMP117_EXAMPLE_H
#define TMP117_EXAMPLE_H

#define TMP117_ALERT PIN_A11 // SAMD21G-PA11/MUX_PA11B_ADC_AIN19
#define Sensor_serviced(s) (1u << s)
#define All_sensors(s) ((1u << s) - 1)

enum par { T_NOW, T_MIN, T_MAX };         // TMP117 Data parameters: actual temperatur and lowest / highest temperatures

typedef enum {
  E_S0 = 0,                               // error reported by sensor0 driver
                                          // reserve e.g. E_S1, ...E_S7 for additional sensors

  E_NO_DATA = 8,                          // application did not receive data from all sensors
} nodeError_t; 

void StartTempSensor(void);
void TempSensorReady(void);
void Error(nodeError_t);

#endif // \TMP117_EXAMPLE_H