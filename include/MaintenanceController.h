// MaintenanceController.h
#ifndef MAINTENANCE_CONTROLLER_H
#define MAINTENANCE_CONTROLLER_H

#include "HomingController.h"
#include "MovementController.h"

// Forward declaration
class SerialCommandHandler;

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

    // New duration control methods
    void setPrimeDuration(unsigned long seconds);
    void setCleanDuration(unsigned long seconds);
    unsigned long getPrimeDuration() const { return primeDurationMs / 1000; }
    unsigned long getCleanDuration() const { return cleanDurationMs / 1000; }

    void setStateManager(StateManager* manager);
    void setHomingController(HomingController* homing)
    {
        homingController = homing;
    }

    void queueDelayedCommand(const String& command);
    void setSerialHandler(SerialCommandHandler* handler);

   private:
    MovementController& movementController;
    HomingController* homingController;
    StateManager* stateManager;
    int maintenanceStep;
    unsigned long stepTimer;
    bool pressurePotActive;     // New member to track pressure pot state
    bool waterDiversionActive;  // Track water diversion state

    unsigned long pressurePotActivationTime;  // Add this line

    // Duration settings (in milliseconds)
    unsigned long primeDurationMs;  // Duration for priming sequence
    unsigned long cleanDurationMs;  // Duration for cleaning sequence

    String queuedCommand;
    unsigned long commandQueueTime;
    SerialCommandHandler* serialHandler;

    void executePrimeSequence();
    void executeCleanSequence();
    void setWaterDiversion(bool active);  // New helper method
};

#endif
