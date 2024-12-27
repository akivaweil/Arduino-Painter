// HomingController.h
#ifndef HOMING_CONTROLLER_H
#define HOMING_CONTROLLER_H

#include <Bounce2.h>

#include "MovementController.h"
#include "ServoController.h"
#include "StateManager.h"

class HomingController
{
   public:
    HomingController(MovementController& movement);
    void setup();
    void update();
    bool isHoming() const;
    bool isHomed() const;
    void startHoming();
    void setStateManager(StateManager* manager);
    void setServoController(ServoController* servo) { servoController = servo; }
    long getHomeRotationPosition() const;

   private:
    MovementController& movementController;
    StateManager* stateManager;
    ServoController* servoController;
    Bounce xHomeSensor;
    Bounce yHomeSensor;

    bool homing;
    bool homeComplete;
    int currentAxis;               // 0 = X, 1 = Y, 2 = Rotation
    long initialRotationPosition;  // Store initial position
    bool initialPositionSet;       // Track if initial position is set

    void processXHoming();
    void processYHoming();
    void processRotationHoming();
};

#endif