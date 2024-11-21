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

   private:
    MovementController& movementController;
    int currentSide;
    int currentCommand;
    int targetSide;
    bool executingSingleSide;

    Command* getCurrentPattern() const;
    int getCurrentPatternSize() const;
    void processNextCommand();
};

#endif
