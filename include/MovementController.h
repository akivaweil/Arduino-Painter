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

    float getCurrentXSpeed();

    bool startContinuousMovement(bool isXAxis, bool isPositive, float speed,
                                 float acceleration);
    bool startContinuousDiagonalMovement(bool xPositive, bool yPositive,
                                         float speed, float acceleration);
    void toggleSpray(bool on);

    void setXHomed(bool homed) { xHomed = homed; }
    void setYHomed(bool homed) { yHomed = homed; }
    bool isXHomed() const { return xHomed; }
    bool isYHomed() const { return yHomed; }

    void logPosition();

    bool isPositionValid(long xSteps, long ySteps) const;

   private:
    AccelStepper stepperX;
    AccelStepper stepperY;
    AccelStepper stepperRotation;
    StateManager* stateManager;

    float frontSpeed;
    float backSpeed;
    float leftSpeed;
    float rightSpeed;

    bool xHomed;
    bool yHomed;

    // Cache current positions to avoid const issues
    mutable long lastXPos;
    mutable long lastYPos;
    mutable long lastRotationPos;

    bool motorsRunning;
    void updateSprayControl(const Command& cmd);
    void configureMotors();
    void updatePositionCache() const;  // Updates cached positions

    // Add member variables to track continuous movement
    bool continuousMovementActive;
    bool continuousMovementIsX;
    bool continuousMovementPositive;

    bool continuousDiagonalActive;
    bool continuousDiagonalXPositive;
    bool continuousDiagonalYPositive;

    void enforceXLimit(long& targetSteps);
    void enforceYLimit(long& targetSteps);

    unsigned long lastPositionLog;
    const unsigned long POSITION_LOG_INTERVAL = 300;  // 300ms between logs
    bool isManualMovement() const;
};

#endif