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

    unsigned long getPressurePotActiveTime() const;

   private:
    MovementController& movementController;
    int maintenanceStep;
    unsigned long stepTimer;
    bool pressurePotActive;  // New member to track pressure pot state

    unsigned long pressurePotActivationTime;  // Add this line

    void executePrimeSequence();
    void executeCleanSequence();
};

#endif
