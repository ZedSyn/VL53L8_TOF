/*******************************************************************************
* Copyright (c) 2020, STMicroelectronics - All Rights Reserved
*
* This file, part of the VL53L8CX Ultra Lite Driver,
* is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
*******************************************************************************/

/***********************************/
/*   VL53L8CX ULD basic example    */
/***********************************/
/*
* This example is the most basic. It initializes the VL53L8CX ULD, and starts
* a ranging to capture 10 frames.
*
* By default, ULD is configured to have the following settings :
* - Resolution 4x4
* - Ranging period 1Hz
*
* In this example, we also suppose that the number of target per zone is
* set to 1 , and all output are enabled (see file platform.h).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "vl53l8cx_api.h"


#define SPI_NUMBER 0
#define SPI_CHANNEL 0 // SPI channel (0 or 1)
#define SPI_SPEED 3000000 // SPI speed in Hz
#define SPI_MODE 3
#define PWREN_PIN 7

#define PICO_IP "192.168.4.1"
#define PICO_PORT 12345

int udp_socket;
struct sockaddr_in pico_addr;

void init_udp_socket(const char *ip, int port) {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("socket creation failed");
        exit(1);
    }

    memset(&pico_addr, 0, sizeof(pico_addr));
    pico_addr.sin_family = AF_INET;
    pico_addr.sin_port = htons(port);
    pico_addr.sin_addr.s_addr = inet_addr(ip);
}

void send_vibration_command(int16_t *values) {
    ssize_t sent = sendto(udp_socket, values, 16 * sizeof(int16_t), 0,
                          (struct sockaddr *)&pico_addr, sizeof(pico_addr));
    if (sent < 0) {
        perror("UDP send failed");
    }
}

int main(void)
{

	/*********************************/
	/*   VL53L8CX ranging variables  */
	/*********************************/

	uint8_t 				status, isAlive, isReady, j;
	VL53L8CX_Configuration 	Dev;			/* Sensor configuration */
	VL53L8CX_ResultsData 	Results;		/* Results data from VL53L8CX */


	/*********************************/
	/*      Customer platform        */
	/*********************************/

	/* Fill the platform structure with customer's implementation. For this
	* example, only the I2C address is used.
	*/
	//Dev.platform.address = VL53L8CX_DEFAULT_I2C_ADDRESS;

	/* (Optional) Reset sensor toggling PINs (see platform, not in API) */
	//VL53L8CX_Reset_Sensor(&(Dev.platform));

	/* (Optional) Set a new I2C address if the wanted address is different
	* from the default one (filled with 0x20 for this example).
	*/
	//status = vl53l8cx_set_i2c_address(&Dev, 0x20);

	wiringPiSetup();
	pinMode(PWREN_PIN, OUTPUT);
	digitalWrite(PWREN_PIN, HIGH);	

	if(wiringPiSPIxSetupMode(SPI_NUMBER, SPI_CHANNEL, SPI_SPEED, SPI_MODE) == -1)  //maybe add the spi mode also
	{
		printf("SPI setup failed!\n");
		return 1;
	}

	printf("SPI setup successful!\n");

	/*********************************/
	/*   Power on sensor and init    */
	/*********************************/

	/* (Optional) Check if there is a VL53L8CX sensor connected */
	status = vl53l8cx_is_alive(&Dev, &isAlive);
	if(!isAlive || status)
	{
		printf("VL53L8CX not detected at requested address\n");
		return status;
	}

	/* (Mandatory) Init VL53L8CX sensor */
	status = vl53l8cx_init(&Dev);
	if(status)
	{
		printf("VL53L8CX ULD Loading failed\n");
		return status;
	}

	printf("VL53L8CX ULD ready ! (Version : %s)\n",
			VL53L8CX_API_REVISION);
	
	status = vl53l8cx_set_ranging_frequency_hz(&Dev, 30);
	if (status) {
		printf("Failed to set ranging frequency\n");
		return status;
	}

	init_udp_socket(PICO_IP, PICO_PORT);  // IP and port of the Pico
	printf("Connected to pico!\n");

	/*********************************/
	/*         Ranging loop          */
	/*********************************/

	status = vl53l8cx_start_ranging(&Dev);

	int loop = 0;
	while(loop < 2000)
	{
		/* Use polling function to know when a new measurement is ready.
		 * Another way can be to wait for HW interrupt raised on PIN A1
		 * (INT) when a new measurement is ready */
 
		status = vl53l8cx_check_data_ready(&Dev, &isReady);

		if(isReady)
		{
			vl53l8cx_get_ranging_data(&Dev, &Results);
			/* As the sensor is set in 4x4 mode by default, we have a total 
			 * of 16 zones to print. For this example, only the data of first zone are 
			 * print */

			// Send intensity values to the Pico W
			send_vibration_command(Results.distance_mm);

            		loop++;
		}

		/* Wait a few ms to avoid too high polling (function in platform
		 * file, not in API) */
		VL53L8CX_WaitMs(&(Dev.platform), 5);
	}

	status = vl53l8cx_stop_ranging(&Dev);
	// Ensure all motors are turned off at the end
	int16_t end[16];
	for (int i = 0; i < 16; ++i) end[i] = 4000;
	send_vibration_command(end);
    	printf("End of ULD demo\n");
    	close(udp_socket);
    	digitalWrite(PWREN_PIN, LOW);
    	wiringPiSPIClose(SPI_CHANNEL);

	return status;
}
