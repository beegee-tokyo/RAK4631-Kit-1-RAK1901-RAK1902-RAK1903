/**
 * @file pressure.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read values from the LPS22HB sensor
 * @version 0.1
 * @date 2021-09-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "app.h"
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>

Adafruit_LPS22 lps22hb;

bool init_press(void)
{
	if (!lps22hb.begin_I2C(0x5c))
	{
		MYLOG("PRESS", "Could not initialize SHTC3");
		return false;
	}

	lps22hb.setDataRate(LPS22_RATE_ONE_SHOT); // LPS22_RATE_ONE_SHOT
	return true;
}

void read_press(void)
{
	sensors_event_t temp_event;
	sensors_event_t pressure_event;
	lps22hb.setDataRate(LPS22_RATE_75_HZ); // LPS22_RATE_ONE_SHOT
	MYLOG("PRESS", "Reading LPS22HB");
	lps22hb.getEvent(&pressure_event, &temp_event);
	delay(1000);

	lps22hb.getEvent(&pressure_event, &temp_event);

	uint16_t press_int = (uint16_t)(pressure_event.pressure * 10);

	MYLOG("PRESS", "P: %.2f", (float)press_int / 10.0);

	// g_weather_data.press_1 = (uint8_t)(press_int >> 8);
	// g_weather_data.press_2 = (uint8_t)(press_int);

	g_solution_data.addBarometricPressure(LPP_CHANNEL_PRESS_2, pressure_event.pressure);

	lps22hb.setDataRate(LPS22_RATE_ONE_SHOT); // LPS22_RATE_ONE_SHOT
}