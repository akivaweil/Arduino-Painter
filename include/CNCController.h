// CNCController.h
#ifndef CNC_CONTROLLER_H
#define CNC_CONTROLLER_H

#include "HomingController.h"
#include "MaintenanceController.h"
#include "MovementController.h"
#include "PatternExecutor.h"
#include "SerialCommandHandler.h"
#include "StateManager.h"

class CNCController
{
   public:
    CNCController();
    void setup();
    void loop();

   private:
    MovementController movementController;        // First
    StateManager stateManager;                    // Second
    HomingController homingController;            // Uses movement controller
    PatternExecutor patternExecutor;              // Uses movement controller
    MaintenanceController maintenanceController;  // Uses movement controller
    SerialCommandHandler serialHandler;           // Uses everything else
};

#endif