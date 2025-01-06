// Patterns.cpp
#include "Patterns.h"

Command FRONT[] = {
    // Initial Movement
    MOVETO_X(3.0 + .75, false),  // →3.0→ - Initial offset
    MOVETO_Y(0 + 2.75, false),   //

    // Row 1
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(27.49, true),  // →27.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 3
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(27.49, true),  // →27.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 5
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(27.49, true),  // →27.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 7
    SPRAY_ON(),           // ● - Start spray
    MOVE_X(27.49, true),  // →27.49→ - Move right with spray
    SPRAY_OFF(),          // ○ - Stop spray
    MOVE_Y(4.16, false),  // ↑4.16↑ - Move up

    // Row 8
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
};

Command BACK[] = {
    MOVETO_X(3.0 + .75, false),     // X offset
    MOVETO_Y(29.12 + 2.75, false),  // Y offset

    // Row 1
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(27.49, true),   // →27.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 3
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(27.49, true),   // →27.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 5
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(27.49, true),   // →27.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 7
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(27.49, true),   // →27.49→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 8
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-27.49, true),  // ←27.49← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
};

Command LEFT[] = {
    // Initial Movement
    MOVETO_X(1.0, false),         //
    MOVETO_Y(3.5 + 3.25, false),  // ↑3.5↑ Initial offset

    // Row 1
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(34.28, true),   // →34.28→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-34.28, true),  // ←34.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 3
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(34.28, true),   // →34.28→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-34.28, true),  // ←34.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 5
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(34.28, true),   // →34.28→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.415, false),  // ↑4.415↑ - Move up

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-34.28, true),  // ←34.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
};

Command RIGHT[] = {
    MOVETO_X(1.0, false),
    MOVETO_Y(26.49 + 3.25, false),  // ↑3.5↑ Initial offset

    // Row 1
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(34.28, true),    // →34.28→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 2
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-34.28, true),   // ←34.28← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 3
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(34.28, true),    // →34.28→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 4
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-34.28, true),   // ←34.28← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 5
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(34.28, true),    // →34.28→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-34.28, true),  // ←34.28← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
};

Command LIP[] = {
    // Initial Movement
    MOVETO_X(1.0, false),       // Move to starting position
    MOVETO_Y(0 + 2.75, false),  // Initial Y offset

    // Column 1 (bottom to top)
    SPRAY_ON(),            // Start spray
    MOVE_Y(27.49, true),   // Move up with spray
    SPRAY_OFF(),           // Stop spray
    MOVE_X(4.415, false),  // Move right to next column

    // Column 2 (top to bottom)
    SPRAY_ON(),            // Start spray
    MOVE_Y(-27.49, true),  // Move down with spray
    SPRAY_OFF(),           // Stop spray
    MOVE_X(4.415, false),  // Move right to next column

    // Column 3 (bottom to top)
    SPRAY_ON(),            // Start spray
    MOVE_Y(27.49, true),   // Move up with spray
    SPRAY_OFF(),           // Stop spray
    MOVE_X(4.415, false),  // Move right to next column

    // Column 4 (top to bottom)
    SPRAY_ON(),            // Start spray
    MOVE_Y(-27.49, true),  // Move down with spray
    SPRAY_OFF(),           // Stop spray
    MOVE_X(4.415, false),  // Move right to next column

    // Column 5 (bottom to top)
    SPRAY_ON(),            // Start spray
    MOVE_Y(27.49, true),   // Move up with spray
    SPRAY_OFF(),           // Stop spray
    MOVE_X(4.415, false),  // Move right to next column

    // Column 6 (top to bottom)
    SPRAY_ON(),            // Start spray
    MOVE_Y(-27.49, true),  // Move down with spray
    SPRAY_OFF(),           // Stop spray
};

const int FRONT_SIZE = sizeof(FRONT) / sizeof(Command);
const int BACK_SIZE = sizeof(BACK) / sizeof(Command);
const int LEFT_SIZE = sizeof(LEFT) / sizeof(Command);
const int RIGHT_SIZE = sizeof(RIGHT) / sizeof(Command);
const int LIP_SIZE = sizeof(LIP) / sizeof(Command);