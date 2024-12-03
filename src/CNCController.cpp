// CNCController.cpp
#include "CNCController.h"

#include "config.h"

CNCController::CNCController()
    : movementController(),
      homingController(movementController),
      patternExecutor(movementController, homingController),
      maintenanceController(movementController),
      serialHandler(stateManager, movementController, homingController,
                    patternExecutor, maintenanceController)
{
    // Inject StateManager into controllers
    homingController.setStateManager(&stateManager);
}

void CNCController::setup()
{
    Serial.begin(115200);

    // Initialize hardware components
    movementController.setup();
    homingController.setup();
    serialHandler.setup();

    // Configure pins
    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN, HIGH);  // Ensure spray is off initially
    pinMode(PRESSURE_POT_RELAY, OUTPUT);
    digitalWrite(PRESSURE_POT_RELAY, HIGH);

    Serial.println(F("CNC Paint Sprayer Ready"));
    Serial.println(F("Commands: H-Home S-Start E-Stop R-Reset"));
}

void CNCController::loop()
{
    // Update all subsystems
    movementController.update();
    homingController.update();

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
}