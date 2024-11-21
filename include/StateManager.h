// StateManager.h
#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "SystemState.h"

class StateManager
{
   public:
    StateManager();

    SystemState getCurrentState() const;
    void setState(SystemState newState);
    bool isValidTransition(SystemState newState) const;
    void reportStateChange();

   private:
    SystemState currentState;
    SystemState previousState;
};

#endif