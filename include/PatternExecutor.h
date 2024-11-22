// PatternExecutor.h
#ifndef PATTERN_EXECUTOR_H
#define PATTERN_EXECUTOR_H

#include "Command.h"
#include "MovementController.h"

class PatternExecutor
{
   public:
    PatternExecutor(MovementController& movement);
    void update();

    void startPattern();
    void startSingleSide(int side);
    bool isExecuting() const;
    void stop();

    String getCurrentPatternName() const;
    int getCurrentSide() const { return currentSide; }

   private:
    MovementController& movementController;
    StateManager* stateManager;
    int currentSide;
    int currentCommand;
    int targetSide;
    bool executingSingleSide;
    bool stopped;

    Command* getCurrentPattern() const;
    int getCurrentPatternSize() const;
    void processNextCommand();
};

#endif
