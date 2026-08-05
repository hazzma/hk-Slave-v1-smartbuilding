#ifndef CONFIG_PINS_H
#define CONFIG_PINS_H

#include <Arduino.h>

// I2C Bus Pins
#define PIN_I2C_SDA      8
#define PIN_I2C_SCL      7

// RS485 Pins
#define PIN_RS485_DIR    2    // MAX3485 CTRL / DE+RE (LOW = Rx, HIGH = Tx)
#define PIN_RS485_RX     20
#define PIN_RS485_TX     21

// Universal GPIO Ports
#define PIN_PORT_0       0
#define PIN_PORT_1       1
#define PIN_PORT_3       3
#define PIN_PORT_4       4

// IR Combo Profile Pin Mapping
#define PIN_IR_AC_1             PIN_PORT_0
#define PIN_IR_AC_2             PIN_PORT_1
#define PIN_IR_PROJECTOR_A      PIN_PORT_3
#define PIN_IR_PROJECTOR_B      PIN_PORT_4

// DHT22 Profile Pin Mapping
#define PIN_DHT22_1             PIN_PORT_0
#define PIN_DHT22_2             PIN_PORT_1
#define PIN_DHT22_3             PIN_PORT_3
#define PIN_DHT22_4             PIN_PORT_4

// Presence Profile Pin Mapping
#define PIN_PRESENCE_1          PIN_PORT_0
#define PIN_PRESENCE_2          PIN_PORT_1
#define PIN_PRESENCE_3          PIN_PORT_3
#define PIN_PRESENCE_4          PIN_PORT_4

// Relay Profile Pin Mapping
#define PIN_RELAY_1             PIN_PORT_0
#define PIN_RELAY_2             PIN_PORT_1

// CO2 Fan Pin Mapping
#define PIN_CO2_FAN             10

#endif // CONFIG_PINS_H
