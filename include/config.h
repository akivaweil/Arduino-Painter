// config.h
#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
const int X_STEP_PIN = 22;
const int X_DIR_PIN = 23;
const int Y_STEP_PIN = 18;
const int Y_DIR_PIN = 19;
const int ROTATION_STEP_PIN = 15;
const int PRESSURE_POT_RELAY = 27;
const int ROTATION_DIR_PIN = 4;
const int PAINT_RELAY_PIN = 26;
const int X_HOME_SENSOR_PIN = 33;
const int Y_HOME_SENSOR_PIN = 32;
const int ROTATION_HOME_SENSOR_PIN = 25;
const int WATER_DIVERSION_RELAY = 14;  // Using pin 14 for water diversion relay

// Steps per unit - Using integers for exact step counts
const int X_STEPS_PER_INCH = 32;     // X-axis calibration
const int Y_STEPS_PER_INCH = 85;     // Y-axis calibration
const int STEPS_PER_ROTATION = 400;  // Steps for full rotation

// Motor speeds and acceleration
// Using extern to avoid multiple definition errors while keeping
// configurability
extern int X_SPEED;         // Steps per second
extern int Y_SPEED;         // Steps per second
extern int ROTATION_SPEED;  // Steps per second
extern int X_ACCEL;         // Steps per second per second
extern int Y_ACCEL;         // Steps per second per second
extern int ROTATION_ACCEL;  // Steps per second per second
extern int HOMING_SPEED;    // Steps per seconds
extern int ROTATION_HOMING_SPEED;

// Default values for speeds and acceleration
// These should be defined in ONE cpp file (e.g., CNCController.cpp)
/*
int X_SPEED = 1000;
int Y_SPEED = 1000;
int ROTATION_SPEED = 1000;
int X_ACCEL = 5000;
int Y_ACCEL = 5000;
int ROTATION_ACCEL = 500;
*/

#endif