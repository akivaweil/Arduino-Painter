// MaintenanceController.cpp
#include "MaintenanceController.h"

#include <Arduino.h>

#include "config.h"

MaintenanceController::MaintenanceController(MovementController& movement)
    : movementController(movement),
      maintenanceStep(0),
      stepTimer(0),
      pressurePotActive(false)
{
}

void MaintenanceController::setup()
{
    // Initialize pressure pot relay pin
    pinMode(PRESSURE_POT_RELAY, OUTPUT);
    digitalWrite(PRESSURE_POT_RELAY, HIGH);  // Start deactivated
}

void MaintenanceController::update()
{
    if (!isRunningMaintenance())
    {
        return;
    }

    switch (maintenanceStep)
    {
        case 1:  // Priming sequence
            executePrimeSequence();
            break;
        case 2:  // Cleaning sequence
            executeCleanSequence();
            break;
    }
}

void MaintenanceController::togglePressurePot()
{
    pressurePotActive = !pressurePotActive;
    digitalWrite(PRESSURE_POT_RELAY, pressurePotActive ? LOW : HIGH);

    Serial.print(F("Pressure pot "));
    Serial.println(pressurePotActive ? F("activated") : F("deactivated"));
}

bool MaintenanceController::isPressurePotActive() const
{
    return pressurePotActive;
}

void MaintenanceController::startPriming()
{
    maintenanceStep = 1;
    stepTimer = millis();
    Serial.println(F("Starting priming sequence"));
}

void MaintenanceController::startCleaning()
{
    maintenanceStep = 2;
    stepTimer = millis();
    Serial.println(F("Starting cleaning sequence"));
}

bool MaintenanceController::isRunningMaintenance() const
{
    return maintenanceStep > 0;
}

void MaintenanceController::executePrimeSequence()
{
    static int primeStep = 0;

    if (primeStep == 0)
    {                                   // Move to prime position (x=0, y=15)
        Command moveX('M', 0, false);   // Move to X=0
        Command moveY('N', 15, false);  // Move to Y=15
        movementController.executeCommand(moveX);
        movementController.executeCommand(moveY);
        stepTimer = millis();
        primeStep = 1;
    }
    else if (primeStep == 1)
    {  // Wait for movement to complete
        if (!movementController.isMoving() && (millis() - stepTimer >= 1000))
        {
            Command sprayCmd('S', 0, true);
            movementController.executeCommand(sprayCmd);
            stepTimer = millis();
            primeStep = 2;
        }
    }
    else if (primeStep == 2)
    {  // Prime for 5 seconds
        if (millis() - stepTimer >= 5000)
        {
            Command stopCmd('S', 0, false);
            movementController.executeCommand(stopCmd);
            primeStep = 0;
            maintenanceStep = 0;  // Complete maintenance
            Serial.println(F("Prime sequence complete"));
        }
    }
}

void MaintenanceController::executeCleanSequence()
{
    static int cleanStep = 0;

    if (cleanStep == 0)
    {                                   // Move to clean position (x=0, y=20)
        Command moveX('M', 0, false);   // Move to X=0
        Command moveY('N', 20, false);  // Move to Y=20
        movementController.executeCommand(moveX);
        movementController.executeCommand(moveY);
        stepTimer = millis();
        cleanStep = 1;
    }
    else if (cleanStep == 1)
    {  // Wait for movement to complete
        if (!movementController.isMoving() && (millis() - stepTimer >= 1000))
        {
            Command sprayCmd('S', 0, true);
            movementController.executeCommand(sprayCmd);
            stepTimer = millis();
            cleanStep = 2;
        }
    }
    else if (cleanStep == 2)
    {  // Spray cleaner for 10 seconds
        if (millis() - stepTimer >= 3000)
        {
            Command stopCmd('S', 0, false);
            movementController.executeCommand(stopCmd);
            stepTimer = millis();
            cleanStep = 3;
        }
    }
    else if (cleanStep == 3)
    {  // Wait 5 seconds
        if (millis() - stepTimer >= 5000)
        {
            cleanStep = 0;
            maintenanceStep = 0;  // Complete maintenance
            Serial.println(F("Clean sequence complete"));
        }
    }
}