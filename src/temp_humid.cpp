/**
 * @file temp_humid.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read data from SHTC3 sensor
 * @version 0.1
 * @date 2021-09-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "app.h"
#include "SparkFun_SHTC3.h"

SHTC3 shtc3;

bool init_th(void)
{
	if (shtc3.begin() != SHTC3_Status_Nominal)
	{
		MYLOG("T_H", "Could not initialize SHTC3");
		return false;
	}
	return true;
}

void read_th(void)
{
	MYLOG("T_H", "Reading SHTC3");
	shtc3.update();

	if (shtc3.lastStatus == SHTC3_Status_Nominal)
	{
		int16_t temp_int = (int16_t)(shtc3.toDegC() * 10.0);
		uint16_t humid_int = (uint16_t)(shtc3.toPercent() * 2);

		MYLOG("T_H", "T: %.2f H: %.2f", (float)temp_int / 10.0, (float)humid_int / 2.0);

		g_weather_data.humid_1 = (uint8_t)(humid_int);
		g_weather_data.temp_1 = (uint8_t)(temp_int >> 8);
		g_weather_data.temp_2 = (uint8_t)(temp_int);
	}
	else
	{
		MYLOG("T_H", "Reading SHTC3 failed");
	}
}
