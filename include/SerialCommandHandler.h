#ifndef SERIAL_COMMAND_HANDLER_H
#define SERIAL_COMMAND_HANDLER_H

#include <Arduino.h>  // For String class

#include "HomingController.h"
#include "MovementController.h"
#include "PatternExecutor.h"
#include "ServoController.h"
#include "StateManager.h"

// Forward declaration
class MaintenanceController;

class SerialCommandHandler
{
   public:
    SerialCommandHandler(StateManager& state, MovementController& movement,
                         HomingController& homing, PatternExecutor& pattern,
                         MaintenanceController& maintenance,
                         ServoController& servo);
    void setup();
    void processCommands();

    // Make this public so MaintenanceController can use it
    void handleSystemCommand(const String& command);

   private:
    StateManager& stateManager;
    MovementController& movementController;
    HomingController& homingController;
    PatternExecutor& patternExecutor;
    MaintenanceController& maintenanceController;
    ServoController& servoController;

    void handleManualMovement(const String& command);
    void handleSpeedCommand(const String& command);
    void handleRotationCommand(const String& command);
    void sendResponse(bool success, const char* message);
    const char* getStateString(SystemState state);
    void handleContinuousMovement(const String& command);
    void handleManualStop();
    void handleContinuousDiagonalMovement(const String& command);
    void handleSprayToggle(const String& command);
    void handleServoCommand(const String& command);
    void handlePauseCommand(const String& command);
    void handlePressurePotDelay(const String& command);
};

#endif