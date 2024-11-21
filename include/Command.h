#ifndef COMMAND_H
#define COMMAND_H

class Command
{
   public:
    char type;     // Command types:
                   // 'X' - X move
                   // 'Y' - Y move
                   // 'R' - rotate
                   // 'S' - spray
                   // 'P' - prime gun
                   // 'C' - clean gun
                   // 'Q' - quick calibrate
                   // 'F' - paint front
                   // 'I' - paint right
                   // 'B' - paint back
                   // 'L' - paint left
    float value;   // Distance in inches or degrees for rotation
    bool sprayOn;  // Whether spray should be on during movement

    // Default constructor
    Command() : type('N'), value(0), sprayOn(false) {}

    // Regular constructor
    Command(char t, float v, bool s) : type(t), value(v), sprayOn(s) {}
};

// Command Creation Macros
#define MOVE_X(dist, spray) Command('X', dist, spray)
#define MOVE_Y(dist, spray) Command('Y', dist, spray)
#define MOVETO_X(pos, spray) Command('M', pos, spray)
#define MOVETO_Y(pos, spray) Command('N', pos, spray)
#define ROTATE(deg) Command('R', deg, false)
#define SPRAY_ON() Command('S', 0, true)
#define SPRAY_OFF() Command('S', 0, false)

// Maintenance command macros
#define PRIME_GUN() Command('P', 0, true)
#define CLEAN_GUN() Command('C', 0, true)
#define QUICK_CAL() Command('Q', 0, false)

// Side painting command macros
#define PAINT_FRONT() Command('F', 0, false)
#define PAINT_RIGHT() \
    Command('I', 0, false)  // Using 'I' to avoid confusion with 'R' (rotate)
#define PAINT_BACK() Command('B', 0, false)
#define PAINT_LEFT() Command('L', 0, false)

#endif