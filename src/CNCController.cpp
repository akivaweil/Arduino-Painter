// CNCController.cpp
#include "CNCController.h"

#include "config.h"

CNCController::CNCController()
    : movementController(),
      homingController(movementController),
      patternExecutor(movementController, homingController),
      maintenanceController(movementController),
      servoController(),
      tcpHandler(stateManager, movementController, homingController,
                 patternExecutor, maintenanceController, servoController)
{
    // Inject StateManager into controllers
    movementController.setStateManager(&stateManager);
    homingController.setStateManager(&stateManager);
    maintenanceController.setStateManager(&stateManager);
    maintenanceController.setHomingController(&homingController);
    patternExecutor.setStateManager(&stateManager);

    // Add servo controller to movement controller and homing controller
    movementController.setServoController(&servoController);
    homingController.setServoController(&servoController);

    // Add TCP handler to controllers
    movementController.setTCPHandler(&tcpHandler);
    patternExecutor.setTCPHandler(&tcpHandler);
}

void CNCController::setup()
{
    Serial.begin(115200);

    // Initialize WiFi and TCP server
    tcpHandler.setup("Everwood", "Everwood-Staff", 8080);

    // Initialize hardware components
    movementController.setup();
    homingController.setup();
    servoController.setup();

    // Configure pins
    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN, HIGH);
    pinMode(PRESSURE_POT_RELAY, OUTPUT);
    digitalWrite(PRESSURE_POT_RELAY, HIGH);

    Serial.println(F("CNC Paint Sprayer Ready"));
}

void CNCController::loop()
{
    static unsigned long lastReport = 0;
    unsigned long startTime = micros();
    unsigned long motionTime, slowUpdateTime, networkTime = 0;

    // Critical motion updates
    movementController.update();  // Most important for smooth motion
    motionTime = micros() - startTime;

    // Less frequent updates
    static uint8_t slowUpdateCounter = 0;
    if (slowUpdateCounter++ >= 10)
    {  // Only run every 10th loop
        unsigned long slowStart = micros();
        homingController.update();
        maintenanceController.update();

        if (patternExecutor.isExecuting())
        {
            patternExecutor.update();
        }
        slowUpdateCounter = 0;
        slowUpdateTime = micros() - slowStart;
    }

    // Network handling - check less frequently
    static uint8_t networkCounter = 0;
    if (networkCounter++ >= 20)  // Reduced frequency: check every 20th loop
    {
        unsigned long networkStart = micros();
        tcpHandler.processConnections();
        networkTime = micros() - networkStart;
        networkCounter = 0;
    }

    // Timing report - only if not moving
    if (!movementController.isMoving() &&
        (micros() - lastReport >= LOOP_REPORT_INTERVAL))
    {
        unsigned long totalTime = micros() - startTime;
        char buffer[128];
        char slowUpdateStr[32] = "";

        if (slowUpdateCounter == 0)
        {
            snprintf(slowUpdateStr, sizeof(slowUpdateStr), ", SlowUpdate: %lu",
                     slowUpdateTime);
        }

        snprintf(
            buffer, sizeof(buffer),
            "Loop breakdown (us) - Total: %lu, Motion: %lu, Network: %lu%s",
            totalTime, motionTime, networkTime, slowUpdateStr);

        tcpHandler.sendResponse(true, buffer);
        lastReport = micros();
    }
}