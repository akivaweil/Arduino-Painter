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
    void primeGun();
    void cleanGun();
    void quickCalibrate();
    void paintSide(int side);  // 0=front, 1=right, 2=back, 3=left

   private:
    SystemState systemState;
    bool motorsRunning;
    int currentSide;
    int currentCommand;
    int targetSide;  // For single side painting

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

    void executePrimeSequence();
    void executeCleanSequence();
    void executeCalibrationSequence();
    void executeSingleSidePaint(int side);
};

#endif