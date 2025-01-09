// MaintenanceController.cpp
#include "MaintenanceController.h"

#include <Arduino.h>

#include "SerialCommandHandler.h"
#include "config.h"

MaintenanceController::MaintenanceController(MovementController& movement)
    : movementController(movement),
      maintenanceStep(0),
      stepTimer(0),
      pressurePotActive(false),
      waterDiversionActive(false),
      stateManager(nullptr),
      primeDurationMs(5000),     // Default 5 seconds
      cleanDurationMs(3000),     // Default 3 seconds
      backWashDurationMs(5000),  // Default 5 seconds
      queuedCommand(""),
      commandQueueTime(0),
      serialHandler(nullptr)
{
}

void MaintenanceController::setup()
{
    // Initialize pressure pot relay pin
    pinMode(PRESSURE_POT_RELAY, OUTPUT);
    pinMode(WATER_DIVERSION_RELAY, OUTPUT);  // Initialize water diversion relay
    digitalWrite(WATER_DIVERSION_RELAY, HIGH);  // Ensure it starts deactivated

    // Check if the relay is already active (LOW)
    int relayState = digitalRead(PRESSURE_POT_RELAY);
    pressurePotActive = (relayState == LOW);

    if (pressurePotActive)
    {
        // If pot is already active, set the activation time to ensure
        // the 5-second requirement is met
        pressurePotActivationTime = millis() - 6000;  // 6 seconds ago
        Serial.println(F("Pressure pot was already active"));
    }

    pinMode(BACK_WASH_RELAY_PIN, OUTPUT);
    digitalWrite(BACK_WASH_RELAY_PIN, HIGH);  // Ensure it starts deactivated
}

void MaintenanceController::update()
{
    // Add debug logging
    if (queuedCommand.length() > 0)
    {
        Serial.print(F("Queued command pending: "));
        Serial.println(queuedCommand);
        Serial.print(F("Pressure pot active time: "));
        Serial.println(getPressurePotActiveTime());
    }

    // Don't process maintenance if pattern is executing
    if (stateManager && stateManager->getCurrentState() == EXECUTING_PATTERN)
    {
        return;
    }

    // Check for queued commands that need to be executed first
    if (queuedCommand.length() > 0 && isPressurePotActive() &&
        (millis() - pressurePotActivationTime >= 5000))
    {
        // Execute the queued command
        if (serialHandler)
        {
            serialHandler->handleSystemCommand(queuedCommand);
            queuedCommand = "";  // Clear the queue after execution
            return;              // Return to ensure command is fully processed
        }
    }

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
        case 3:  // Back wash sequence
            executeBackWashSequence();
            break;
    }
}

void MaintenanceController::togglePressurePot()
{
    pressurePotActive = !pressurePotActive;
    if (pressurePotActive)
    {
        pressurePotActivationTime = millis();
    }
    digitalWrite(PRESSURE_POT_RELAY, pressurePotActive ? LOW : HIGH);

    Serial.print(F("Pressure pot "));
    Serial.println(pressurePotActive ? F("activated") : F("deactivated"));
}

bool MaintenanceController::isPressurePotActive() const
{
    return pressurePotActive;
}

unsigned long MaintenanceController::getPressurePotActiveTime() const
{
    if (!pressurePotActive)
    {
        return 0;
    }
    return millis() - pressurePotActivationTime;
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
    setWaterDiversion(true);  // Activate water diversion when cleaning starts
    Serial.println(F("Starting cleaning sequence"));
}

bool MaintenanceController::isRunningMaintenance() const
{
    return maintenanceStep > 0;
}

void MaintenanceController::setPrimeDuration(unsigned long seconds)
{
    primeDurationMs = seconds * 1000;
    Serial.print(F("Prime duration set to "));
    Serial.print(seconds);
    Serial.println(F(" seconds"));
}

void MaintenanceController::setCleanDuration(unsigned long seconds)
{
    cleanDurationMs = seconds * 1000;
    Serial.print(F("Clean duration set to "));
    Serial.print(seconds);
    Serial.println(F(" seconds"));
}

void MaintenanceController::executePrimeSequence()
{
    static int primeStep = 0;

    if (primeStep == 0)
    {                                   // Move to prime position (x=0, y=15)
        Command moveX('M', 0, false);   // Move to X=0
        Command moveY('N', 15, false);  // Move to Y=15
        Command servoCmd('S', 135, false);  // Set servo to 135 degrees
        movementController.executeCommand(moveX);
        movementController.executeCommand(moveY);
        movementController.executeCommand(servoCmd);
        stepTimer = millis();
        primeStep = 1;
    }
    else if (primeStep == 1)
    {  // Wait for movement to complete
        if (!movementController.isMoving() && (millis() - stepTimer >= 1000))
        {
            Command sprayCmd(
                'P', 0, true);  // Changed from 'S' to 'P' to control spray only
            movementController.executeCommand(sprayCmd);
            stepTimer = millis();
            primeStep = 2;
        }
    }
    else if (primeStep == 2)
    {  // Prime for configured duration
        if (millis() - stepTimer >= primeDurationMs)
        {
            Command stopCmd(
                'P', 0,
                false);  // Changed from 'S' to 'P' to control spray only
            movementController.executeCommand(stopCmd);
            primeStep = 0;
            maintenanceStep = 0;  // Complete maintenance
            Serial.println(F("Prime sequence complete"));

            // Start homing sequence properly through HomingController
            if (stateManager)
            {
                homingController->startHoming();  // Actually start homing
            }
        }
    }
}

void MaintenanceController::executeCleanSequence()
{
    static int cleanStep = 0;

    if (cleanStep == 0)
    {
        Command moveX('M', 0, false);       // Move to X=0
        Command moveY('N', 20, false);      // Move to Y=20
        Command servoCmd('S', 135, false);  // Set servo to 135 degrees
        movementController.executeCommand(moveX);
        movementController.executeCommand(moveY);
        movementController.executeCommand(servoCmd);
        stepTimer = millis();
        cleanStep = 1;
    }
    else if (cleanStep == 1)
    {
        if (!movementController.isMoving() && (millis() - stepTimer >= 1000))
        {
            Command sprayCmd('P', 0, true);
            movementController.executeCommand(sprayCmd);
            stepTimer = millis();
            cleanStep = 2;
        }
    }
    else if (cleanStep == 2)
    {  // Clean for configured duration
        if (millis() - stepTimer >= cleanDurationMs)
        {
            Command stopCmd('P', 0, false);
            movementController.executeCommand(stopCmd);
            cleanStep = 0;
            maintenanceStep = 0;       // Complete maintenance
            setWaterDiversion(false);  // Deactivate water diversion
            Serial.println(F("Clean sequence complete"));

            // Start homing sequence properly through HomingController
            if (stateManager)
            {
                homingController->startHoming();  // Actually start homing
            }
        }
    }
}

void MaintenanceController::startBackWash()
{
    maintenanceStep = 3;
    stepTimer = millis();
    digitalWrite(BACK_WASH_RELAY_PIN, LOW);

    // Add servo command for back wash
    Command servoCmd('S', 135, false);
    movementController.executeCommand(servoCmd);

    Serial.println(F("Starting back wash sequence"));
}

void MaintenanceController::executeBackWashSequence()
{
    if (millis() - stepTimer >= backWashDurationMs)
    {
        digitalWrite(BACK_WASH_RELAY_PIN, HIGH);
        maintenanceStep = 0;  // Complete maintenance
        Serial.println(F("Back wash sequence complete"));

        // Return to previous state
        if (stateManager)
        {
            stateManager->setState(stateManager->getPreviousState());
        }
    }
}

void MaintenanceController::setBackWashDuration(unsigned long seconds)
{
    backWashDurationMs = seconds * 1000;
    Serial.print(F("Back wash duration set to "));
    Serial.print(seconds);
    Serial.println(F(" seconds"));
}

void MaintenanceController::setStateManager(StateManager* manager)
{
    stateManager = manager;
}

void MaintenanceController::setWaterDiversion(bool active)
{
    waterDiversionActive = active;
    digitalWrite(WATER_DIVERSION_RELAY, active ? LOW : HIGH);
    Serial.print(F("Water diversion "));
    Serial.println(active ? F("activated") : F("deactivated"));
}

void MaintenanceController::queueDelayedCommand(const String& command)
{
    queuedCommand = command;
    Serial.print(F("Command queued: "));
    Serial.println(command);
}

void MaintenanceController::executeQueuedCommand()
{
    if (queuedCommand.length() > 0)
    {
        Serial.print(F("Executing queued command: "));
        Serial.println(queuedCommand);
        if (serialHandler)
        {
            serialHandler->handleSystemCommand(queuedCommand);
        }
        queuedCommand = "";  // Clear the queue after execution
    }
}

void MaintenanceController::setSerialHandler(SerialCommandHandler* handler)
{
    serialHandler = handler;
}