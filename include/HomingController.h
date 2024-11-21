// HomingController.h
#ifndef HOMING_CONTROLLER_H
#define HOMING_CONTROLLER_H

#include <Bounce2.h>

#include "MovementController.h"

class HomingController
{
   public:
    HomingController(MovementController& movement);
    void setup();
    void update();

    bool isHoming() const;
    bool isHomed() const;
    void startHoming();

   private:
    MovementController& movementController;
    Bounce xHomeSensor;
    Bounce yHomeSensor;

    bool homing;
    bool homeComplete;
    int currentAxis;  // 0 = X, 1 = Y

    void processXHoming();
    void processYHoming();
};

#endif