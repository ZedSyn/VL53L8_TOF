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
#define SPI_SPEED 500000//0 // SPI speed in Hz
#define SPI_MODE 3
#define PWREN_PIN 7

#define PICO_IP "192.168.4.1"
#define PICO_PORT 80

int send_vibration_command(int dist[16]) {
    char payload[256];
    char http_request[512];

    snprintf(payload, sizeof(payload),
             "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             dist[0], dist[1], dist[2], dist[3], dist[4], dist[5], dist[6], dist[7],
             dist[8], dist[9], dist[10], dist[11], dist[12], dist[13], dist[14], dist[15]);

    snprintf(http_request, sizeof(http_request),
             "POST / HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Content-Length: %zu\r\n"
             "Content-Type: text/plain\r\n"
             "\r\n"
             "%s",
             PICO_IP,
             strlen(payload),
             payload);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(PICO_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(PICO_PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect error\n");
        close(sock);
        return 1;
    }

    if (send(sock, http_request, strlen(http_request), 0) < 0) {
        printf("Send failed\n");
        close(sock);
        return 1;
    }

    // Optionally receive server response (can be removed if not needed)
    char response[256];
    int len = recv(sock, response, sizeof(response) - 1, 0);
    if (len > 0) {
        response[len] = '\0';
        printf("Response: %s\n", response);
    }

    close(sock);
    return 0;
}

int main(void)
{

	/*********************************/
	/*   VL53L8CX ranging variables  */
	/*********************************/

	uint8_t 				status, loop, isAlive, isReady, j;
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
	/*********************************/
	/*         Ranging loop          */
	/*********************************/

	status = vl53l8cx_start_ranging(&Dev);

	loop = 0;
	while(loop < 255)
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
			printf("Print data no : %3u\n", Dev.streamcount);

			int distances[16];
			for(j = 0; j < 16; j++)
			{
				distances[j] = Results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * j];
                printf("Zone : %3d, Status : %3u, Distance : %4d mm\n",
                       j,
                       Results.target_status[VL53L8CX_NB_TARGET_PER_ZONE * j],
                       distances[j]);
			}
			printf("\n");
			// Send intensity values to the Pico W
			//send_vibration_command(distances);

            loop++;
		}

		/* Wait a few ms to avoid too high polling (function in platform
		 * file, not in API) */
		VL53L8CX_WaitMs(&(Dev.platform), 5);
	}

	status = vl53l8cx_stop_ranging(&Dev);
	// Ensure all motors are turned off at the end
	//int end[16] = {0};
	//send_vibration_command(end);
    printf("End of ULD demo\n");

    digitalWrite(PWREN_PIN, LOW);
    wiringPiSPIClose(SPI_CHANNEL);

	return status;
}
