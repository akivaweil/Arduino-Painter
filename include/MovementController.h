#ifndef MOVEMENT_CONTROLLER_H
#define MOVEMENT_CONTROLLER_H

#include <AccelStepper.h>

#include "Command.h"
#include "StateManager.h"

class MovementController
{
   public:
    MovementController();
    void setup();
    void update();

    bool executeCommand(const Command& cmd);
    bool isMoving() const;
    void stop();

    // Position control methods
    void stopMovement();               // New method for immediate stop
    void setXPosition(long position);  // New method to set X position
    void setYPosition(long position);  // New method to set Y position

    // Position query methods
    long getCurrentXSteps() const;
    long getCurrentYSteps() const;

    // Unit conversion methods
    float stepsToInches(long steps, long stepsPerInch) const;
    float stepsToAngle(long steps) const;

    void setXSpeed(float acceleration);
    void setYSpeed(float acceleration);

    void setRotationPosition(long position);
    void setRotationSpeed(float speed);
    long getCurrentRotationSteps() const;
    float getCurrentRotationAngle() const;
    void setStateManager(StateManager* manager) { stateManager = manager; }

    void setPatternSpeed(const String& pattern, float speedPercentage);
    void applyPatternSpeed(const String& pattern);
    void resetToDefaultSpeed();

   private:
    AccelStepper stepperX;
    AccelStepper stepperY;
    AccelStepper stepperRotation;
    StateManager* stateManager;

    float frontSpeed;
    float backSpeed;
    float leftSpeed;
    float rightSpeed;

    // Cache current positions to avoid const issues
    mutable long lastXPos;
    mutable long lastYPos;
    mutable long lastRotationPos;

    bool motorsRunning;
    void updateSprayControl(const Command& cmd);
    void configureMotors();
    void updatePositionCache() const;  // Updates cached positions
};

#endif