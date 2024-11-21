#ifndef SERIAL_COMMAND_HANDLER_H
#define SERIAL_COMMAND_HANDLER_H

#include <Arduino.h>  // For String class

#include "HomingController.h"
#include "MaintenanceController.h"
#include "MovementController.h"
#include "PatternExecutor.h"
#include "StateManager.h"

class SerialCommandHandler
{
   public:
    SerialCommandHandler(StateManager& state, MovementController& movement,
                         HomingController& homing, PatternExecutor& pattern,
                         MaintenanceController& maintenance);
    void setup();
    void processCommands();

   private:
    StateManager& stateManager;
    MovementController& movementController;
    HomingController& homingController;
    PatternExecutor& patternExecutor;
    MaintenanceController& maintenanceController;

    // Updated to accept String parameters instead of char
    void handleManualMovement(const String& command);
    void handleSystemCommand(const String& command);
    void sendResponse(bool success, const char* message);
};

#endif