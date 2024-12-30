#ifndef TCP_COMMAND_HANDLER_H
#define TCP_COMMAND_HANDLER_H

#include <ArduinoJson.h>
#include <WiFiS3.h>

#include "StateManager.h"

// Forward declarations
class MovementController;
class HomingController;
class PatternExecutor;
class MaintenanceController;
class ServoController;

class TCPCommandHandler
{
   public:
    TCPCommandHandler(StateManager& state, MovementController& movement,
                      HomingController& homing, PatternExecutor& pattern,
                      MaintenanceController& maintenance,
                      ServoController& servo);

    void setup(const char* ssid, const char* password, int port);
    void processConnections();
    void handleSystemCommand(const String& command);
    void sendResponse(bool success, const char* message);

   private:
    WiFiServer server;
    WiFiClient client;
    StateManager& stateManager;
    MovementController& movementController;
    HomingController& homingController;
    PatternExecutor& patternExecutor;
    MaintenanceController& maintenanceController;
    ServoController& servoController;

    void handleManualMovement(const String& command);
    void handleSpeedCommand(const String& command);
    void handleRotationCommand(const String& command);
    const char* getStateString(SystemState state);
    void handleContinuousMovement(const String& command);
    void handleManualStop();
    void handleContinuousDiagonalMovement(const String& command);
    void handleSprayToggle(const String& command);
    void handleServoCommand(const String& command);
    void handlePrimeTimeCommand(const String& command);
    void handleCleanTimeCommand(const String& command);
    void handleBackWashTimeCommand(const String& command);
    void handleSetGridCommand(const String& command);
    void handleSetHorizontalTravelCommand(const String& command);
    void handleSetVerticalTravelCommand(const String& command);
    void handleSetOffsetCommand(const String& command);

    void sendJsonResponse(bool success, const char* message);
};

#endif