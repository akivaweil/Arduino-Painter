// SerialCommandHandler.h
#ifndef SERIAL_COMMAND_HANDLER_H
#define SERIAL_COMMAND_HANDLER_H

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

    void handleManualMovement(char cmd);
    void handleSystemCommand(char cmd);
    void sendResponse(bool success, const char* message);
};

#endif