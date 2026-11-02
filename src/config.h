#pragma once
#include "raylib.h"

namespace Config
{
    // Drone Movement (all per unit time)
    inline constexpr float YAW_SPEED = 50.0f;           // Speed of drone yaw
    inline constexpr float SPEED_INCREMENT = 5.0f;      // Acceleration of movement
    inline constexpr float PRESERVED_VELOCITY = 0.10f;  // Percentage of current velocity to maintain in next movement
    inline constexpr float MAX_SPEED = 5.0f;            // Maximum velocity vector len

    inline constexpr float MAX_PITCH = 30.0f;   // In local Z (degrees)
    inline constexpr float MAX_ROLL = 30.0f;    // In local X (degrees)

    inline constexpr float G = 9.81f; // Gravity constant for when drone is off

    // Screen/UI
    inline constexpr int SCREEN_WIDTH = 1280;
    inline constexpr int SCREEN_HEIGHT = 720;

    // Rendering
    inline const float AMBIENT_STRENGTH = 0.25f;     // Baseline ambient light
    inline const float SPECULAR_STRENGTH = 0.60f;    // Strength of specular highlights
    inline const float SHININESS = 200.0f;          // Baseline shininess of all materials
    inline const float SPOTLIGHT_INNER_CUTOFF = cos(12.5f * DEG2RAD);
    inline const float SPOTLIGHT_OUTER_CUTOFF = cos(17.5f * DEG2RAD);

    // Light Colours
    inline constexpr Vector3 LIGHT_COLOUR_DAY = {1.0f, 1.0f, 1.0f};      // Bright white sun
    inline constexpr Vector3 LIGHT_COLOUR_NIGHT = {0.1f, 0.15f, 0.3f};   // Dim, moody blue moonlight
    inline constexpr Vector3 SPOTLIGHT_COLOUR = {1.0f, 0.95f, 0.8f};

    // Drone Rendering
    inline const Color COLOUR_GROUND = (Color) {0, 117, 44, 255};       // Dark Green
    inline const Color COLOUR_CHASSIS = (Color){210, 200, 180, 255};    // Pale Sand / Bone
    inline const Color COLOUR_FRAME = (Color){40, 45, 35, 255};         // Deep Olive
    inline const Color COLOUR_MECHANICAL = (Color){120, 110, 100, 255}; // Muddy Steel
    inline const Color COLOUR_ROTORS = (Color){20, 18, 15, 255};        // Very Dark Brown/Black
    inline const Color COLOUR_ACCENT = (Color){160, 70, 60, 255};       // Faded Brick Red

    inline constexpr float DRONE_FLOOR = 0.9f;  // Derived from leg height + half body height

    // All in degrees per unit time
    inline constexpr float ROTOR_FAST_SPEED = 3240.0f;      // When ascending
    inline constexpr float ROTOR_NORMAL_SPEED = 2160.0f;    // When no vertical change
    inline constexpr float ROTOR_SLOW_SPEED = 1440.0f;      // When descending
    inline constexpr float ROTOR_OFF_SPEED = 0.0f;          // When drone off

    inline constexpr float BODY_WIDTH = 0.75f;      // Main drone chassis width  (Local X)
    inline constexpr float BODY_LENGTH = 1.15f;     // Main drone chassis length (Local Z)
    inline constexpr float BODY_HEIGHT = 0.6f;      // Main drone chassis height (Local Y)
    inline constexpr float EXO_MARGIN = 0.0f;       // Clearance between chassis and roll cage
    inline constexpr float EXO_PIPE_THICK = 0.08f;  // Thickness of the cage pipes
    inline constexpr float EXO_CORNER_SIZE = 0.1f;  // Size of corner junction boxes
    inline constexpr float ARM_WIDTH = 0.1f;        // Drone arm width  (Local X)
    inline constexpr float ARM_HEIGHT = 0.1f;       // Drone arm height (Local Y)
    inline constexpr float ARM_LENGTH = 1.2f;       // Drone arm length (Local Z)

    inline const float armAngularOffset = atan(BODY_LENGTH / BODY_WIDTH) * RAD2DEG; // Puts arms at corners of main body
    inline const float ARM_ANGLES[4] = {
        90.0f - armAngularOffset,
        90.0f + armAngularOffset,
        270.0f - armAngularOffset,
        270.0f + armAngularOffset};
    inline constexpr float ARM_OFFSET = 0.375f + (ARM_LENGTH / 2.0f);   // Offset of rotor block from arms (Local Z)

    inline constexpr Vector3 UNIT_VECTOR = {1.0f, 1.0f, 1.0f};  // Used for filling scaling transformations

    // Camera Control
    inline constexpr Vector3 THIRD_PERSON_OFFSET = {0.0f, 2.5f, -5.0f}; // Camera position relative to drone position (Behind, above)
    inline constexpr Vector3 THIRD_PERSON_TARGET = {0.0f, 0.0f, 0.0f};  // Point camera at drone (local origin)
    inline constexpr float THIRD_PERSON_FOV = 45.0f;                    // Higher FOV for third person cam
    inline constexpr Vector3 FIRST_PERSON_OFFSET = {0.0f, -0.5f, 0.0f};  // Camera position relative to drone position (Slightly below, technically inside chassis)
    inline constexpr Vector3 FIRST_PERSON_TARGET = {0.0f, -1.0, 0.0f}; // Point camera directly down, slightly forward (so not parallel to camera's `up` dir)
    inline constexpr float FIRST_PERSON_WIDTH = 1.0f;                  // Orthographic camera width for ground cam
}
