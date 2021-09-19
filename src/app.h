/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from main.h
 * @version 0.1
 * @date 2021-04-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef APP_H
#define APP_H

#include <Arduino.h>
/** Add you required includes after Arduino.h */

// LoRaWan functions
struct weather_data_s
{
	uint8_t data_flag1 = 0x08;	// 1
	uint8_t data_flag2 = 0x02;	// 2
	uint8_t batt_1 = 0;			// 3
	uint8_t batt_2 = 0;			// 4
	uint8_t data_flag3 = 0x07;	// 5
	uint8_t data_flag4 = 0x68;	// 6
	uint8_t humid_1 = 0;		// 7
	uint8_t data_flag5 = 0x02;	// 8
	uint8_t data_flag6 = 0x67;  // 9
	uint8_t temp_1 = 0;			// 10
	uint8_t temp_2 = 0;			// 11
	uint8_t data_flag7 = 0x06;  // 12
	uint8_t data_flag8 = 0x73;  // 13
	uint8_t press_1 = 0;		// 14
	uint8_t press_2 = 0;		// 15
	uint8_t data_flag9 = 0x05;	// 16
	uint8_t data_flag10 = 0x65; // 17
	uint8_t light_1 = 0;		// 18
	uint8_t light_2 = 0;		// 19
};
extern weather_data_s g_weather_data;
#define WEATHER_DATA_LEN 19		  // sizeof(g_weather_data) with BME680


#include <Wire.h>
/** Include the SX126x-API */
#include <WisBlock-API.h>
/** Application function definitions */
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);

/** Application stuff */
extern BaseType_t g_higher_priority_task_woken;

/** Sensor functions */
bool init_th(void);
void read_th(void);
bool init_press(void);
void read_press(void);
bool init_light(void);
void read_light();

/** Battery level uinion */
union batt_s
{
	uint16_t batt16 = 0;
	uint8_t batt8[2];
};

#endif