#ifndef PATTERN_EXECUTOR_H
#define PATTERN_EXECUTOR_H

#include <Arduino.h>

#include "Command.h"
#include "HomingController.h"
#include "MovementController.h"
#include "PatternSettings.h"

// Forward declarations
class TCPCommandHandler;

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

    void setGrid(int x, int y)
    {
        settings.rows.x = x;
        settings.rows.y = y;
    }

    void setHorizontalTravel(float x, float y);
    void setVerticalTravel(float x, float y);

    ~PatternExecutor();

    void setTCPHandler(TCPCommandHandler* handler) { tcpHandler = handler; }

    void logEvent(const char* event, const String& details);

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
    TCPCommandHandler* tcpHandler;
};

#endif