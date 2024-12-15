#ifndef PATTERN_EXECUTOR_H
#define PATTERN_EXECUTOR_H

#include "Command.h"
#include "HomingController.h"
#include "MovementController.h"
#include "PatternSettings.h"

class PatternExecutor
{
   public:
    // Modified constructor to accept HomingController
    PatternExecutor(MovementController& movement, HomingController& homing);
    void update();

    void startPattern();
    void startSingleSide(int side);
    bool isExecuting() const;
    void stop();

    String getCurrentPatternName() const;
    int getCurrentSide() const { return currentSide; }

    // Add method to set state manager if not already present
    void setStateManager(StateManager* manager) { stateManager = manager; }

    // Pattern configuration methods
    void setOffsetX(float x) { settings.offsets.x = x; }
    void setOffsetY(float y) { settings.offsets.y = y; }
    void setTravelX(float x) { settings.travelDistance.x = x; }
    void setTravelY(float y) { settings.travelDistance.y = y; }
    void setGrid(int x, int y)
    {
        settings.rows.x = x;
        settings.rows.y = y;
    }

    ~PatternExecutor();

   private:
    MovementController& movementController;
    HomingController& homingController;  // Changed to reference
    StateManager* stateManager;
    int currentSide;
    int currentCommand;
    int targetSide;
    bool executingSingleSide;
    bool stopped;
    int currentRow;
    int currentRotation;

    PatternSettings settings;
    Command* generatePattern(int side) const;
    int calculatePatternSize(int side) const;

    void reportStatus(const char* event, const String& details);
    float calculateMovementDuration(const Command& cmd) const;

    Command* getCurrentPattern() const;
    int getCurrentPatternSize() const;
    void processNextCommand();

    int calculateOptimalRotation(int targetRotation);

    mutable Command* currentPattern;
};

#endif