// Patterns.cpp
#include "Patterns.h"

Command FRONT[] = {
    // Initial Movement
    MOVETO_X(3.5 + 0.75, false),  // →3.5→ - Initial offset
    MOVETO_Y(0 + 2.25, false),    //
    ROTATE(0),             // Rotate tray

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

};

Command BACK[] = {

    MOVETO_X(3.5 + 0.75, false),    // X offset
    MOVETO_Y(29.12 + 2.25, false),  // Y offset
    ROTATE(180),             // Rotate tray

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
};

Command LEFT[] = {
    // Initial Movement

    MOVETO_X(0 + 0.75, false),    //
    MOVETO_Y(3.5 + 2.25, false),  // ↑3.5↑ Initial offset
    ROTATE(-90),           // Rotate tray

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

};

Command RIGHT[] = {

    MOVETO_X(0 + 0.75, false),   
    MOVETO_Y(22.075 + 2.25, false),  // ↑3.5↑ Initial offset
    ROTATE(-180),            // Rotate tray

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

    ROTATE(90),            // Rotate tray back to starting position
};

const int FRONT_SIZE = sizeof(FRONT) / sizeof(Command);
const int BACK_SIZE = sizeof(BACK) / sizeof(Command);
const int LEFT_SIZE = sizeof(LEFT) / sizeof(Command);
const int RIGHT_SIZE = sizeof(RIGHT) / sizeof(Command);