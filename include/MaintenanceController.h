// MaintenanceController.h
#ifndef MAINTENANCE_CONTROLLER_H
#define MAINTENANCE_CONTROLLER_H

#include "MovementController.h"

class MaintenanceController
{
   public:
    MaintenanceController(MovementController& movement);
    void update();

    void startPriming();
    void startCleaning();
    void startCalibration();

    bool isRunningMaintenance() const;

   private:
    MovementController& movementController;
    int maintenanceStep;
    unsigned long stepTimer;

    void executePrimeSequence();
    void executeCleanSequence();
    void executeCalibrationSequence();
};

#endif
