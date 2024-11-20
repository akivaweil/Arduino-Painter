// Command.h
#ifndef COMMAND_H
#define COMMAND_H

class Command
{
   public:
    char type;  // 'X' for X move, 'Y' for Y move, 'R' for rotate, 'S' for spray
    float value;   // Distance in inches or degrees for rotation
    bool sprayOn;  // Whether spray should be on during movement

    Command(char t, float v, bool s) : type(t), value(v), sprayOn(s) {}
};

// Command Creation Macros
#define MOVE_X(dist, spray) Command('X', dist, spray)
#define MOVE_Y(dist, spray) Command('Y', dist, spray)
#define ROTATE(deg) Command('R', deg, false)
#define SPRAY_ON() Command('S', 0, true)
#define SPRAY_OFF() Command('S', 0, false)

#endif