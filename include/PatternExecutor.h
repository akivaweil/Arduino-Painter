#ifndef PATTERN_EXECUTOR_H
#define PATTERN_EXECUTOR_H

#include "Command.h"
#include "HomingController.h"
#include "MovementController.h"
#include "PatternSettings.h"

struct Offset
{
    float x;
    float y;
    float angle;
};

struct InitialOffsets
{
    Offset front;
    Offset back;
    Offset left;
    Offset right;
};

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
    void setFrontOffsets(float x, float y, float angle)
    {
        settings.initialOffsets.front.x = x;
        settings.initialOffsets.front.y = y;
        settings.initialOffsets.front.angle = angle;
    }

    void setBackOffsets(float x, float y, float angle)
    {
        settings.initialOffsets.back.x = x;
        settings.initialOffsets.back.y = y;
        settings.initialOffsets.back.angle = angle;
    }

    void setLeftOffsets(float x, float y, float angle)
    {
        settings.initialOffsets.left.x = x;
        settings.initialOffsets.left.y = y;
        settings.initialOffsets.left.angle = angle;
    }

    void setRightOffsets(float x, float y, float angle)
    {
        settings.initialOffsets.right.x = x;
        settings.initialOffsets.right.y = y;
        settings.initialOffsets.right.angle = angle;
    }

    void setLipOffsets(float x, float y, float angle)
    {
        settings.initialOffsets.lip.x = x;
        settings.initialOffsets.lip.y = y;
        settings.initialOffsets.lip.angle = angle;
    }

    void setGrid(int x, int y)
    {
        settings.rows.x = x;
        settings.rows.y = y;
    }

    void setEnabledSides(bool front, bool right, bool back, bool left, bool lip)
    {
        settings.enabledSides.front = front;
        settings.enabledSides.right = right;
        settings.enabledSides.back = back;
        settings.enabledSides.left = left;
        settings.enabledSides.lip = lip;
    }

    bool isSideEnabled(int side) const
    {
        switch (side)
        {
            case 0:
                return settings.enabledSides.front;
            case 1:
                return settings.enabledSides.back;
            case 2:
                return settings.enabledSides.left;
            case 3:
                return settings.enabledSides.right;
            case 4:
                return settings.enabledSides.lip;
            default:
                return false;
        }
    }

    void setHorizontalTravel(float x, float y);
    void setVerticalTravel(float x, float y);
    void setLipTravel(float x, float y);

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