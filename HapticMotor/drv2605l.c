#include <stdio.h>          // Standard I/O functions
#include <wiringPiI2C.h>    // WiringPi library for I2C communication
#include <stdint.h>         // Standard integer types
#include <unistd.h>         // POSIX API for sleep and other functions
#include <stdlib.h>         // Standard library for general utilities

#include "drv2605l.h"       // Header file for DRV2605L-specific definitions

int drv_fd = -1;            // File descriptor for I2C communication

int drv2605_init() {
    drv_fd = wiringPiI2CSetup(DRV2605_ADDR); // Initialize I2C communication with the DRV2605L
    if (drv_fd == -1) {                      // Check if I2C setup failed
        printf("Failed to open DRV2605 on I2C\n");
        return -1;                           // Return error code -1
    }

    uint8_t status = wiringPiI2CReadReg8(drv_fd, REG_STATUS); // Read the status register
    uint8_t device_id = (status >> 5) & 0x07;                 // Extract the device ID
    if (device_id != 3 && device_id != 7) {                   // Validate the device ID
        printf("Invalid DRV2605 chip ID: %d\n", device_id);
        return -2;                                            // Return error code -2
    }

    wiringPiI2CWriteReg8(drv_fd, REG_MODE, 0x00);             // Set mode to standby
    wiringPiI2CWriteReg8(drv_fd, REG_RTPIN, 0x00);            // Disable real-time playback
    wiringPiI2CWriteReg8(drv_fd, REG_WAVESEQ1, 1);            // Set first waveform slot to effect 1
    for (int i = 5; i <= 0x0B; i++)                           // Clear waveform slots 2-7
        wiringPiI2CWriteReg8(drv_fd, i, 0);

    wiringPiI2CWriteReg8(drv_fd, REG_OVERDRIVE, 0);           // Set overdrive register to 0
    wiringPiI2CWriteReg8(drv_fd, REG_SUSTAINPOS, 0);          // Set positive sustain time to 0
    wiringPiI2CWriteReg8(drv_fd, REG_SUSTAINNEG, 0);          // Set negative sustain time to 0
    wiringPiI2CWriteReg8(drv_fd, REG_BREAK, 0);               // Set brake time to 0
    wiringPiI2CWriteReg8(drv_fd, REG_AUDIOMAX, 0x64);         // Set maximum audio-to-vibe gain

    drv2605_use_erm();                                        // Configure the device to use ERM motor

    uint8_t ctrl3 = wiringPiI2CReadReg8(drv_fd, REG_CONTROL3); // Read control register 3
    wiringPiI2CWriteReg8(drv_fd, REG_CONTROL3, ctrl3 | 0x20);  // Enable open-loop mode

    drv2605_set_mode(MODE_INTTRIG);                           // Set mode to internal trigger
    drv2605_set_library(LIB_TS2200A);                         // Set waveform library to TS2200A
    return 0;                                                 // Return success
}

void drv2605_use_erm() {
    uint8_t feedback = wiringPiI2CReadReg8(drv_fd, REG_FEEDBACK); // Read feedback register
    wiringPiI2CWriteReg8(drv_fd, REG_FEEDBACK, feedback & 0x7F);  // Clear LRA bit to use ERM
}

void drv2605_use_lra() {
    uint8_t feedback = wiringPiI2CReadReg8(drv_fd, REG_FEEDBACK); // Read feedback register
    wiringPiI2CWriteReg8(drv_fd, REG_FEEDBACK, feedback | 0x80);  // Set LRA bit to use LRA
}

void drv2605_set_mode(uint8_t mode) {
    wiringPiI2CWriteReg8(drv_fd, REG_MODE, mode & 0x07);          // Set the mode register
}

uint8_t drv2605_get_mode() {
    return wiringPiI2CReadReg8(drv_fd, REG_MODE);                 // Read and return the mode register
}

void drv2605_set_library(uint8_t lib) {
    wiringPiI2CWriteReg8(drv_fd, REG_LIBRARY, lib & 0x07);        // Set the library register
}

uint8_t drv2605_get_library() {
    return wiringPiI2CReadReg8(drv_fd, REG_LIBRARY) & 0x07;       // Read and return the library register
}

void drv2605_set_waveform(uint8_t slot, uint8_t effect_id) {
    if (slot > 7) return;                                        // Validate slot number
    wiringPiI2CWriteReg8(drv_fd, REG_WAVESEQ1 + slot, effect_id); // Set the effect ID in the specified slot
}

void drv2605_play() {
    wiringPiI2CWriteReg8(drv_fd, REG_GO, 1);                     // Start playback
}

void drv2605_stop() {
    wiringPiI2CWriteReg8(drv_fd, REG_GO, 0);                     // Stop playback
}

void drv2605_set_realtime_value(int8_t val) {
    wiringPiI2CWriteReg8(drv_fd, REG_RTPIN, val);                // Set real-time playback value
}
