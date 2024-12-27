// CNCController.cpp
#include "CNCController.h"

#include "config.h"

CNCController::CNCController()
    : movementController(),
      homingController(movementController),
      patternExecutor(movementController, homingController),
      maintenanceController(movementController),
      servoController(),
      serialHandler(stateManager, movementController, homingController,
                    patternExecutor, maintenanceController, servoController)
{
    // Inject StateManager into controllers
    movementController.setStateManager(&stateManager);
    homingController.setStateManager(&stateManager);
    maintenanceController.setStateManager(&stateManager);
    maintenanceController.setHomingController(&homingController);
    patternExecutor.setStateManager(&stateManager);
}

void CNCController::setup()
{
    Serial.begin(115200);

    // Initialize hardware components
    movementController.setup();
    homingController.setup();
    serialHandler.setup();
    servoController.setup();

    // Configure pins
    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN,
                 HIGH);  // Inverted logic - ensure spray is off initially
    pinMode(PRESSURE_POT_RELAY, OUTPUT);
    digitalWrite(
        PRESSURE_POT_RELAY,
        HIGH);  // Inverted logic - ensure pressure pot is off initially

    Serial.println(F("CNC Paint Sprayer Ready"));
    Serial.println(F("Commands: H-Home S-Start E-Stop R-Reset"));

    maintenanceController.setSerialHandler(&serialHandler);
}

void CNCController::loop()
{
    // Update all subsystems
    movementController.update();
    homingController.update();
    maintenanceController
        .update();  // Make sure this is called before processing commands

    // Process pattern execution if active
    if (patternExecutor.isExecuting())
    {
        patternExecutor.update();
    }

    // Process maintenance operations if active
    if (maintenanceController.isRunningMaintenance())
    {
        maintenanceController.update();
    }

    // Handle any incoming serial commands
    serialHandler.processCommands();

    servoController.update();
}