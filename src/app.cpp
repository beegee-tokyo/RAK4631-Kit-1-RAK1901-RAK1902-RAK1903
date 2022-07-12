/**
 * @file app.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Application specific functions. Mandatory to have init_app(), 
 *        app_event_handler(), ble_data_handler(), lora_data_handler()
 *        and lora_tx_finished()
 * @version 0.1
 * @date 2021-04-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "app.h"

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK-WEA";

/** Flag showing if TX cycle is ongoing */
bool lora_busy = false;

/** Required for give semaphore from ISR */
BaseType_t g_higher_priority_task_woken = pdTRUE;

/** Send Fail counter **/
uint8_t send_fail = 0;

/** Flag for low battery protection */
bool low_batt_protection = false;

/** LoRaWAN packet */
weather_data_s g_weather_data;

/** Battery level uinion */
batt_s batt_level;

/**
 * @brief Application specific setup functions
 * 
 */
void setup_app(void)
{
	// Enable BLE
	g_enable_ble = true;
}

/**
 * @brief Application specific initializations
 * 
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	MYLOG("APP", "init_app");

	Wire.begin();
	Wire.setClock(400000);

	for (int adr = 0; adr < 127; adr++)
	{
		Wire.beginTransmission(adr);
		if (Wire.endTransmission() == 0)
		{
			MYLOG("APP", "Found device on adresse %0X", adr);
		}
	}
	if (!init_th())
	{
		MYLOG("APP", "SHTC3 error");
		return false;
	}

	read_th();

	if (!init_press())
	{
		MYLOG("APP", "LPS22HB error");
		return false;
	}

	read_press();

	if (!init_light())
	{
		MYLOG("APP", "OPT3001 error");
		return false;
	}

	read_light();

	return true;
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		// If BLE is enabled, restart Advertising
		if (g_enable_ble)
		{
			restart_advertising(15);
		}

		if (lora_busy)
		{
			MYLOG("APP", "LoRaWAN TX cycle not finished, skip this event");
			if (g_ble_uart_is_connected)
			{
				g_ble_uart.println("LoRaWAN TX cycle not finished, skip this event");
			}
		}
		else
		{
			if (!low_batt_protection)
			{

				// Read temperature and humidity
				read_th();
				// Read air pressure
				read_press();
				// Read luminosity
				read_light();
			}

			// Get battery level
			batt_level.batt16 = read_batt() / 10;
			g_weather_data.batt_1 = batt_level.batt8[1];
			g_weather_data.batt_2 = batt_level.batt8[0];

			// Protection against battery drain
			if (batt_level.batt16 < 290)
			{
				// Battery is very low, change send time to 1 hour to protect battery
				low_batt_protection = true;						   // Set low_batt_protection active
				api_timer_restart(1 * 60 * 60 * 1000); // Set send time to one hour
				MYLOG("APP", "Battery protection activated");
			}
			else if ((batt_level.batt16 > 410) && low_batt_protection)
			{
				// Battery is higher than 4V, change send time back to original setting
				low_batt_protection = false;
				api_timer_restart(g_lorawan_settings.send_repeat_time);
				MYLOG("APP", "Battery protection deactivated");
			}

			lmh_error_status result = send_lora_packet((uint8_t *)&g_weather_data, WEATHER_DATA_LEN);
			switch (result)
			{
			case LMH_SUCCESS:
				MYLOG("APP", "Packet enqueued");
				/// \todo set a flag that TX cycle is running
				lora_busy = true;
				if (g_ble_uart_is_connected)
				{
					g_ble_uart.println("Packet enqueued");
				}
				break;
			case LMH_BUSY:
				MYLOG("APP", "LoRa transceiver is busy");
				if (g_ble_uart_is_connected)
				{
					g_ble_uart.println("LoRa transceiver is busy");
				}
				break;
			case LMH_ERROR:
				MYLOG("APP", "Packet error, too big to send with current DR");
				if (g_ble_uart_is_connected)
				{
					g_ble_uart.println("Packet error, too big to send with current DR");
				}
				break;
			}
		}
	}
}

/**
 * @brief Handle BLE UART data
 * 
 */
void ble_data_handler(void)
{
	if (g_enable_ble)
	{
		// BLE UART data handling
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			MYLOG("AT", "RECEIVED BLE");
			/** BLE UART data arrived */
			g_task_event_type &= N_BLE_DATA;

			while (g_ble_uart.available() > 0)
			{
				at_serial_input(uint8_t(g_ble_uart.read()));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
		}
	}
}

/**
 * @brief Handle received LoRa Data
 * 
 */
void lora_data_handler(void)
{

	// LoRa Join finished handling
	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
	{
		g_task_event_type &= N_LORA_JOIN_FIN;
		if (g_join_result)
		{
			MYLOG("APP", "Successfully joined network");
		}
		else
		{
			MYLOG("APP", "Join network failed");
			/// \todo here join could be restarted.
			lmh_join();

			// If BLE is enabled, restart Advertising
			if (g_enable_ble)
			{
				restart_advertising(15);
			}
		}
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		MYLOG("APP", "LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");
		if (g_ble_uart_is_connected)
		{
			g_ble_uart.printf("LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");
		}

		if (!g_rx_fin_result)
		{
			// Increase fail send counter
			send_fail++;

			if (send_fail == 10)
			{
				// Too many failed sendings, reset node and try to rejoin
				delay(100);
				api_reset();
			}
		}
		/// \todo reset flag that TX cycle is running
		lora_busy = false;
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRa data arrived
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");
		char log_buff[g_rx_data_len * 3] = {0};
		uint8_t log_idx = 0;
		for (int idx = 0; idx < g_rx_data_len; idx++)
		{
			sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
			log_idx += 3;
		}
		lora_busy = false;
		MYLOG("APP", "%s", log_buff);

		if (g_ble_uart_is_connected && g_enable_ble)
		{
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				g_ble_uart.printf("%02X ", g_rx_lora_data[idx]);
			}
			g_ble_uart.println("");
		}
	}
}
