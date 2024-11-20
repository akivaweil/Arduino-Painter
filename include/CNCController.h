// CNCController.h
#ifndef CNC_CONTROLLER_H
#define CNC_CONTROLLER_H

#include <AccelStepper.h>
#include <Bounce2.h>

#include "Command.h"
#include "SystemState.h"

class CNCController
{
   public:
    CNCController();
    void setup();
    void loop();
    void executeCommand(const Command& cmd);
    void processPattern();

   private:
    SystemState systemState;
    bool motorsRunning;
    int currentSide;
    int currentCommand;

    Command lastExecutedCommand;
    bool awaitingCompletion = false;

    AccelStepper stepperX;
    AccelStepper stepperY;
    AccelStepper stepperRotation;
    Bounce xHomeSensor;
    Bounce yHomeSensor;

    void handleSerialCommand();
    void updateMotors();
    void processSystemState();
};

#endif