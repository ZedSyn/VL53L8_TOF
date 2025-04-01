/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


#include "platform.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <unistd.h> // for usleep

#define SPI_CHANNEL 0 // SPI channel (0 or 1)
#define SPI_SPEED 500000 // SPI speed in Hz

uint8_t VL53L8CX_RdByte(
		VL53L8CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_value)
{
	uint8_t status = 0;
	uint8_t tx_buffer[3] = { (RegisterAdress >> 8) & 0xFF, RegisterAdress & 0xFF, 0x00 };
	uint8_t rx_buffer[3] = { 0 };

	if (wiringPiSPIDataRW(SPI_CHANNEL, tx_buffer, 3) == -1) {
		status = 255; // Error
	} else {
		*p_value = tx_buffer[2];
	}

	return status;
}

uint8_t VL53L8CX_WrByte(
		VL53L8CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t value)
{
	uint8_t status = 0;
	uint8_t tx_buffer[3] = { (RegisterAdress >> 8) & 0xFF, RegisterAdress & 0xFF, value };

	if (wiringPiSPIDataRW(SPI_CHANNEL, tx_buffer, 3) == -1) {
		status = 255; // Error
	}

	return status;
}

uint8_t VL53L8CX_WrMulti(
		VL53L8CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
	uint8_t status = 0;
	uint8_t tx_buffer[size + 2];
	tx_buffer[0] = (RegisterAdress >> 8) & 0xFF;
	tx_buffer[1] = RegisterAdress & 0xFF;
	memcpy(&tx_buffer[2], p_values, size);

	if (wiringPiSPIDataRW(SPI_CHANNEL, tx_buffer, size + 2) == -1) {
		status = 255; // Error
	}

	return status;
}

uint8_t VL53L8CX_RdMulti(
		VL53L8CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
	uint8_t status = 0;
	uint8_t tx_buffer[size + 2];
	uint8_t rx_buffer[size + 2];
	tx_buffer[0] = (RegisterAdress >> 8) & 0xFF;
	tx_buffer[1] = RegisterAdress & 0xFF;
	memset(&tx_buffer[2], 0, size);

	if (wiringPiSPIDataRW(SPI_CHANNEL, tx_buffer, size + 2) == -1) {
		status = 255; // Error
	} else {
		memcpy(p_values, &tx_buffer[2], size);
	}

	return status;
}

uint8_t VL53L8CX_Reset_Sensor(
		VL53L8CX_Platform *p_platform)
{
	uint8_t status = 0;

	// Assuming GPIO pins are used for power control
	pinMode(0, OUTPUT); // Example GPIO pin for LPN
	digitalWrite(0, LOW);
	usleep(100000); // 100 ms delay
	digitalWrite(0, HIGH);
	usleep(100000); // 100 ms delay

	return status;
}

void VL53L8CX_SwapBuffer(
		uint8_t 		*buffer,
		uint16_t 	 	 size)
{
	uint32_t i, tmp;

	for (i = 0; i < size; i = i + 4) {
		tmp = (
		  buffer[i] << 24)
		| (buffer[i + 1] << 16)
		| (buffer[i + 2] << 8)
		| (buffer[i + 3]);

		memcpy(&(buffer[i]), &tmp, 4);
	}
}

uint8_t VL53L8CX_WaitMs(
		VL53L8CX_Platform *p_platform,
		uint32_t TimeMs)
{
	delay(TimeMs); // WiringPi delay in milliseconds
	return 0;
}
