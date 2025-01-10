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
    void executeQueuedCommand();
    void setSerialHandler(SerialCommandHandler* handler);

    void startBackWash();
    void setBackWashDuration(unsigned long seconds);
    unsigned long getBackWashDuration() const
    {
        return backWashDurationMs / 1000;
    }

    void setPressurePotDelay(unsigned long milliseconds);
    unsigned long getPressurePotDelay() const { return pressurePotDelay; }

    // New methods for configuring maintenance positions
    void setPrimePosition(float x, float y, float angle);
    void setCleanPosition(float x, float y, float angle);
    void getPrimePosition(float& x, float& y, float& angle) const;
    void getCleanPosition(float& x, float& y, float& angle) const;

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
    unsigned long primeDurationMs;     // Duration for priming sequence
    unsigned long cleanDurationMs;     // Duration for cleaning sequence
    unsigned long backWashDurationMs;  // Duration for back wash sequence

    String queuedCommand;
    unsigned long commandQueueTime;
    SerialCommandHandler* serialHandler;

    unsigned long pressurePotDelay = 5000;  // Default 5 second delay

    void executePrimeSequence();
    void executeCleanSequence();
    void setWaterDiversion(bool active);  // New helper method
    void executeBackWashSequence();

    // New members for maintenance position configuration
    struct Position
    {
        float x = 0.0;       // Default X position
        float y = 0.0;       // Default Y position
        float angle = 90.0;  // Default angle
    };

    Position primePosition;  // Position for priming operations
    Position cleanPosition;  // Position for cleaning operations
};

#endif
