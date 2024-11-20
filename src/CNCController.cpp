// CNCController.cpp
#include "CNCController.h"

#include "Patterns.h"
#include "config.h"

CNCController::CNCController()
    : systemState(IDLE),
      motorsRunning(false),
      currentSide(0),
      currentCommand(0),
      stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN),
      stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN),
      stepperRotation(AccelStepper::DRIVER, ROTATION_STEP_PIN, ROTATION_DIR_PIN)
{
}

void CNCController::setup()
{
    Serial.begin(115200);

    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN, HIGH);

    pinMode(X_HOME_SENSOR_PIN, INPUT_PULLUP);
    pinMode(Y_HOME_SENSOR_PIN, INPUT_PULLUP);

    xHomeSensor.attach(X_HOME_SENSOR_PIN);
    yHomeSensor.attach(Y_HOME_SENSOR_PIN);
    xHomeSensor.interval(20);
    yHomeSensor.interval(20);

    stepperX.setMaxSpeed(X_SPEED);
    stepperX.setAcceleration(X_ACCEL);

    stepperY.setMaxSpeed(Y_SPEED);
    stepperY.setAcceleration(Y_ACCEL);

    stepperRotation.setMaxSpeed(ROTATION_SPEED);
    stepperRotation.setAcceleration(ROTATION_ACCEL);

    Serial.println(F("CNC Paint Sprayer Ready"));
    Serial.println(F("Commands: H-Home S-Start E-Stop R-Reset"));
}

void CNCController::executeCommand(const Command& cmd)
{
    switch (cmd.type)
    {
        case 'X':
            if (cmd.sprayOn)
                digitalWrite(PAINT_RELAY_PIN, LOW);
            stepperX.move(cmd.value *
                          X_STEPS_PER_INCH);  // Use X-specific calibration
            break;

        case 'Y':
            if (cmd.sprayOn)
                digitalWrite(PAINT_RELAY_PIN, LOW);
            stepperY.move(cmd.value *
                          Y_STEPS_PER_INCH);  // Use Y-specific calibration
            break;

        case 'R':
            stepperRotation.move((cmd.value * STEPS_PER_ROTATION) / 360);
            break;

        case 'S':
            digitalWrite(PAINT_RELAY_PIN, cmd.sprayOn ? LOW : HIGH);
            break;
    }
}

void CNCController::processPattern()
{
    if (!motorsRunning)
    {
        Command* currentPattern;
        int patternSize;

        switch (currentSide)
        {
            case 0:
                currentPattern = SIDE1_PATTERN;
                patternSize = SIDE1_SIZE;
                break;
            case 1:
                currentPattern = SIDE2_PATTERN;
                patternSize = SIDE2_SIZE;
                break;
            case 2:
                currentPattern = SIDE3_PATTERN;
                patternSize = SIDE3_SIZE;
                break;
            case 3:
                currentPattern = SIDE4_PATTERN;
                patternSize = SIDE4_SIZE;
                break;
        }

        if (currentCommand < patternSize)
        {
            executeCommand(currentPattern[currentCommand]);
            currentCommand++;
        }
        else
        {
            currentCommand = 0;
            currentSide++;
            if (currentSide >= 4)
            {
                systemState = CYCLE_COMPLETE;
            }
        }
    }
}

void CNCController::loop()
{
    xHomeSensor.update();
    yHomeSensor.update();

    updateMotors();
    handleSerialCommand();
    processSystemState();
}