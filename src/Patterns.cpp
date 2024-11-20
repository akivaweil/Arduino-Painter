// Patterns.cpp
#include "Patterns.h"

Command SIDE1_PATTERN[] = {
    // Initial Movement
    MOVE_X(3.5, false),  // →3.5→ - Initial offset

    // Row 1
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(26.49, true),  // →26.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 3
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(26.49, true),  // →26.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 5
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(26.49, true),  // →26.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 7
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(26.49, true),  // →26.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 8
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray

    // Rotation
    ROTATE(180)  // Rotate tray 180 degrees
};

Command SIDE2_PATTERN[] = {
    // Row 1
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26.49, true),   // →26.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 3
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26.49, true),   // →26.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 5
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26.49, true),   // →26.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 7
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26.49, true),   // →26.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 8
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26.49, true),  // ←26.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray

    // Final movement and rotation
    //   MOVE_X(-3.5, false),   // ←3.5← Return to start
    ROTATE(90)  // Rotate tray 90 degrees
};

Command SIDE3_PATTERN[] = {
    // Initial Movement
    MOVE_Y(3.5, false),  // ↑3.5↑ Initial offset

    // Row 1
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(33.28, true),   // →33.28→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-33.28, true),  // ←33.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 3
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(33.28, true),   // →33.28→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-33.28, true),  // ←33.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 5
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(33.28, true),   // →33.28→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-33.28, true),  // ←33.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray

    // Rotation
    ROTATE(180)  // Rotate tray 180 degrees
};

Command SIDE4_PATTERN[] = {
    // Row 1
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(33.28, true),    // →33.28→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 2
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-33.28, true),   // ←33.28← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 3
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(33.28, true),    // →33.28→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 4
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-33.28, true),   // ←33.28← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 5
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(33.28, true),    // →33.28→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-33.28, true),  // ←33.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray

    // Final movement
    MOVE_Y(-3.5, false)  // ↓3.5↓ Return to start
};

const int SIDE1_SIZE = sizeof(SIDE1_PATTERN) / sizeof(Command);
const int SIDE2_SIZE = sizeof(SIDE2_PATTERN) / sizeof(Command);
const int SIDE3_SIZE = sizeof(SIDE3_PATTERN) / sizeof(Command);
const int SIDE4_SIZE = sizeof(SIDE4_PATTERN) / sizeof(Command);