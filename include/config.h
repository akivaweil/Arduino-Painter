// config.h
#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
const int X_STEP_PIN = 5;
const int X_DIR_PIN = 6;
const int Y_STEP_PIN = 11;
const int Y_DIR_PIN = 10;
const int ROTATION_STEP_PIN = 2;
const int ROTATION_DIR_PIN = 3;
const int PAINT_RELAY_PIN = 4;
const int X_HOME_SENSOR_PIN = 12;
const int Y_HOME_SENSOR_PIN = 8;

// System Configuration
const int X_STEPS_PER_INCH = 127;  // X-axis calibration
const int Y_STEPS_PER_INCH = 169;  // Y-axis calibration
const int STEPS_PER_ROTATION = 800;
int X_SPEED = 1000;
int Y_SPEED = 1000;
int ROTATION_SPEED = 1000;
int X_ACCEL = 5000;
int Y_ACCEL = 5000;
int ROTATION_ACCEL = 100;

#endif