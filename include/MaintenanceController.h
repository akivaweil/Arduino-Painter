// MaintenanceController.h
#ifndef MAINTENANCE_CONTROLLER_H
#define MAINTENANCE_CONTROLLER_H

#include "MovementController.h"

class MaintenanceController
{
   public:
    MaintenanceController(MovementController& movement);
    void update();
    void setup();

    void startPriming();
    void startCleaning();

    // New pressure pot control methods
    void togglePressurePot();
    bool isPressurePotActive() const;

    bool isRunningMaintenance() const;

   private:
    MovementController& movementController;
    int maintenanceStep;
    unsigned long stepTimer;
    bool pressurePotActive;  // New member to track pressure pot state

    void executePrimeSequence();
    void executeCleanSequence();
};

#endif
