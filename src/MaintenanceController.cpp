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
    // Don't process maintenance if pattern is executing
    if (stateManager && stateManager->getCurrentState() == EXECUTING_PATTERN)
    {
        return;
    }

    // Check for queued commands that need to be executed first
    if (queuedCommand.length() > 0 && isPressurePotActive() &&
        (millis() - pressurePotActivationTime >=
         pressurePotDelay))  // Use configurable delay
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
    if (!isRunningMaintenance())
    {
        Serial.println(F("=== Clean Settings ==="));
        Serial.print(F("Clean duration: "));
        Serial.print(cleanDurationMs / 1000);
        Serial.println(F(" seconds"));
        Serial.print(F("Clean position - X: "));
        Serial.print(cleanPosition.x);
        Serial.print(F(", Y: "));
        Serial.print(cleanPosition.y);
        Serial.print(F(", Angle: "));
        Serial.println(cleanPosition.angle);

        maintenanceStep = 2;  // Change this to 2 to indicate cleaning sequence
        setWaterDiversion(true);
        Serial.println(F("Starting cleaning sequence"));

        if (stateManager)
        {
            stateManager->setState(CLEANING);
        }
    }
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

void MaintenanceController::setPrimePosition(float x, float y, float angle)
{
    primePosition.x = x;
    primePosition.y = y;
    primePosition.angle = angle;
}

void MaintenanceController::setCleanPosition(float x, float y, float angle)
{
    cleanPosition.x = x;
    cleanPosition.y = y;
    cleanPosition.angle = angle;

    // Add debug logging
    Serial.println(F("Clean position set to:"));
    Serial.print(F("X: "));
    Serial.print(x);
    Serial.print(F(" Y: "));
    Serial.print(y);
    Serial.print(F(" Angle: "));
    Serial.println(angle);
}

void MaintenanceController::getPrimePosition(float& x, float& y,
                                             float& angle) const
{
    x = primePosition.x;
    y = primePosition.y;
    angle = primePosition.angle;
}

void MaintenanceController::getCleanPosition(float& x, float& y,
                                             float& angle) const
{
    x = cleanPosition.x;
    y = cleanPosition.y;
    angle = cleanPosition.angle;
}

void MaintenanceController::executePrimeSequence()
{
    static uint8_t primeStep = 1;  // Add this to track prime sequence steps

    switch (primeStep)
    {
        case 1:  // Move to prime position
        {
            Command moveX('M', primePosition.x, false);
            Command moveY('N', primePosition.y, false);
            Command rotate('R', primePosition.angle, false);

            movementController.executeCommand(moveX);
            movementController.executeCommand(moveY);
            movementController.executeCommand(rotate);

            primeStep = 2;
            stepTimer = millis();
            Serial.println(F("Moving to prime position"));
        }
        break;

        case 2:  // Wait for movement to complete
            if (!movementController.isMoving() &&
                (millis() - stepTimer >= 1000))
            {
                Command sprayCmd('P', 0, true);
                movementController.executeCommand(sprayCmd);
                stepTimer = millis();
                primeStep = 3;
                Serial.println(F("Starting prime spray"));
            }
            break;

        case 3:  // Prime for configured duration
            if (millis() - stepTimer >= primeDurationMs)
            {
                Command stopCmd('P', 0, false);
                movementController.executeCommand(stopCmd);
                maintenanceStep = 0;  // Exit maintenance mode
                primeStep = 1;        // Reset prime step for next time
                Serial.println(F("Prime sequence complete"));

                // Start homing sequence if homingController is available
                if (homingController != nullptr)
                {
                    homingController->startHoming();
                }
                else
                {
                    Serial.println(F("Warning: HomingController not set"));
                }
            }
            break;
    }
}

void MaintenanceController::executeCleanSequence()
{
    static bool movementStarted = false;
    static bool sprayStarted = false;
    static uint8_t cleanStep = 1;  // Add this to track clean sequence steps

    switch (cleanStep)  // Use cleanStep instead of maintenanceStep
    {
        case 1:  // Move to clean position
            if (!movementStarted)
            {
                Serial.println(F("Moving to clean position"));
                Serial.print(F("Target - X: "));
                Serial.print(cleanPosition.x);
                Serial.print(F(" Y: "));
                Serial.print(cleanPosition.y);
                Serial.print(F(" Angle: "));
                Serial.println(cleanPosition.angle);

                movementController.executeCommand(
                    MOVETO_X(cleanPosition.x, false));
                movementController.executeCommand(
                    MOVETO_Y(cleanPosition.y, false));
                movementController.executeCommand(ROTATE(cleanPosition.angle));
                movementStarted = true;
                stepTimer = millis();
            }
            else if (!movementController.isMoving() &&
                     (millis() - stepTimer > 1000))
            {
                cleanStep = 2;  // Move to spray step
                movementStarted = false;
                stepTimer = millis();
            }
            break;

        case 2:  // Spray step
            if (!sprayStarted)
            {
                movementController.executeCommand(SPRAY_ON());
                Serial.print(F("Starting clean spray for "));
                Serial.print(cleanDurationMs / 1000);
                Serial.println(F(" seconds"));
                sprayStarted = true;
                stepTimer = millis();
            }
            else if (millis() - stepTimer >= cleanDurationMs)
            {
                movementController.executeCommand(SPRAY_OFF());
                Serial.println(F("Clean sequence complete"));
                setWaterDiversion(false);
                maintenanceStep = 0;  // Exit maintenance mode
                sprayStarted = false;
                cleanStep = 1;  // Reset clean step for next time
                if (stateManager)
                {
                    stateManager->setState(HOMED);
                }
            }
            break;
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

void MaintenanceController::setPressurePotDelay(unsigned long milliseconds)
{
    pressurePotDelay = milliseconds;
    Serial.print(F("Pressure pot delay set to: "));
    Serial.print(pressurePotDelay);
    Serial.println(F(" ms"));
}