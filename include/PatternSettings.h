#ifndef PATTERN_SETTINGS_H
#define PATTERN_SETTINGS_H

struct PatternSettings
{
    struct
    {
        float x;
        float y;
    } offsets;

    struct
    {
        float x;
        float y;
    } travelDistance;

    struct
    {
        int x;
        int y;
    } rows;

    // Constructor with default values
    PatternSettings()
    {
        offsets.x = 3.5 + 0.75;    // Default from original pattern
        offsets.y = 0 + 2.25;      // Default from original pattern
        travelDistance.x = 26.49;  // Default from original pattern
        travelDistance.y = 4.16;   // Default from original pattern
        rows.x = 8;                // Default from original pattern
        rows.y = 6;                // Default from original pattern
    }
};

#endif