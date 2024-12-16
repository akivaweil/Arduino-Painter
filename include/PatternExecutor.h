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
    void setFrontOffsets(float x, float y)
    {
        settings.initialOffsets.front.x = x;
        settings.initialOffsets.front.y = y;
    }

    void setBackOffsets(float x, float y)
    {
        settings.initialOffsets.back.x = x;
        settings.initialOffsets.back.y = y;
    }

    void setLeftOffsets(float x, float y)
    {
        settings.initialOffsets.left.x = x;
        settings.initialOffsets.left.y = y;
    }

    void setRightOffsets(float x, float y)
    {
        settings.initialOffsets.right.x = x;
        settings.initialOffsets.right.y = y;
    }

    void setGrid(int x, int y)
    {
        settings.rows.x = x;
        settings.rows.y = y;
    }

    void setHorizontalTravel(float x, float y);
    void setVerticalTravel(float x, float y);

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
    mutable int cachedPatternSide;  // Track which side's pattern is cached

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