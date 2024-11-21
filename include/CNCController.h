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
    StateManager stateManager;
    MovementController movementController;
    HomingController homingController;
    PatternExecutor patternExecutor;
    MaintenanceController maintenanceController;
    SerialCommandHandler serialHandler;
};

#endif