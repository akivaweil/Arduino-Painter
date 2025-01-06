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
            float angle;
        } front, right, back, left, lip;
    } initialOffsets;

    struct
    {
        struct
        {
            float x;
            float y;
        } horizontal, vertical, lip;
    } travelDistance;

    struct
    {
        int x;
        int y;
    } rows;

    struct
    {
        bool front;
        bool right;
        bool back;
        bool left;
        bool lip;
    } enabledSides;

    // Constructor with default values
    PatternSettings()
    {
        // Front offsets
        initialOffsets.front.x = 3.5 + 0.75;
        initialOffsets.front.y = 0 + 2.25;
        initialOffsets.front.angle = 90;

        // Right offsets
        initialOffsets.right.x = 1.5;
        initialOffsets.right.y = 26.49 + 2.25;
        initialOffsets.right.angle = 90;

        // Back offsets
        initialOffsets.back.x = 3.5 + 0.75;
        initialOffsets.back.y = 29.12 + 2.25;
        initialOffsets.back.angle = 90;

        // Left offsets
        initialOffsets.left.x = 1.5;
        initialOffsets.left.y = 3.5 + 2.75;
        initialOffsets.left.angle = 90;

        // Travel distances
        travelDistance.horizontal.x = 26.49;
        travelDistance.horizontal.y = 4.16;
        travelDistance.vertical.x = 33.28;
        travelDistance.vertical.y = 4.415;

        // Default grid dimensions
        rows.x = 8;
        rows.y = 6;

        // All sides enabled by default
        enabledSides.front = true;
        enabledSides.right = true;
        enabledSides.back = true;
        enabledSides.left = true;
        enabledSides.lip = true;
    }
};

#endif