#ifndef PATTERN_SETTINGS_H
#define PATTERN_SETTINGS_H

struct PatternSettings
{
    struct
    {
        struct
        {
            float x;
            float y;
        } front, right, back, left;
    } initialOffsets;

    struct
    {
        struct
        {
            float x;
            float y;
        } horizontal, vertical;
    } travelDistance;

    struct
    {
        int x;
        int y;
    } rows;

    // Constructor with default values
    PatternSettings()
    {
        // Front offsets
        initialOffsets.front.x = 3.5 + 0.75;
        initialOffsets.front.y = 0 + 2.25;

        // Right offsets
        initialOffsets.right.x = 1.5;
        initialOffsets.right.y = 26.49 + 2.25;

        // Back offsets
        initialOffsets.back.x = 3.5 + 0.75;
        initialOffsets.back.y = 29.12 + 2.25;

        // Left offsets
        initialOffsets.left.x = 1.5;
        initialOffsets.left.y = 3.5 + 2.75;

        // Travel distances
        travelDistance.horizontal.x = 26.49;
        travelDistance.horizontal.y = 4.16;
        travelDistance.vertical.x = 33.28;
        travelDistance.vertical.y = 4.415;

        // Default grid dimensions
        rows.x = 8;
        rows.y = 6;
    }
};

#endif