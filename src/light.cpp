/**
 * @file light.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read data from OPT3001 sensor
 * @version 0.1
 * @date 2021-09-19
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "app.h"
#include <ClosedCube_OPT3001.h>

ClosedCube_OPT3001 opt3001;
#define OPT3001_ADDRESS 0x44

/**
 * @brief Initialize the Light sensor
 *
 * @return true if sensor found and configuration success
 * @return false if error occured
 */
bool init_light(void)
{
	if (opt3001.begin(OPT3001_ADDRESS) != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not initialize SHTC3");
		return false;
	}
	OPT3001_Config newConfig;

	newConfig.RangeNumber = B1100;
	newConfig.ConvertionTime = B0;
	newConfig.Latch = B1;
	newConfig.ModeOfConversionOperation = B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
		return false;
	}
	return true;
}

/**
 * @brief Read value from light sensor
 *
 */
void read_light()
{
	MYLOG("LIGHT", "Reading OPT3001");
	OPT3001 result = opt3001.readResult();
	if (result.error == NO_ERROR)
	{
		MYLOG("LIGHT", "L: %.2f", result.lux);

		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, (uint32_t)(result.lux));
	}
	else
	{
		MYLOG("LIGHT", "Error reading OPT3001");
		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, 0);
	}
}
