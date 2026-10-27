#pragma once
#include <raylib.h>
#include "shaders/lighting.h"
#include "shaders/shadow_depth.h"

extern Shader lighting;
extern Shader shadows;

extern Model cubeModel;
extern Model cylModel;
extern Model coneModel;
extern Model planeModel;

inline const Color DAY_SHADOW = (Color){0, 50, 20, 255};    // Very dark green
inline const Color NIGHT_SHADOW = (Color){2, 5, 10, 255};   // Pitch black with a slight blue tint
inline Color activeShadowColour = DAY_SHADOW;

inline bool SHADOW_RENDER = false;  // Flag for shadow pass

// Functions (From Example Code)
// Perform composite transformation
inline Matrix TRS(const Vector3 &t, const Vector3 &axis, float angleRad, const Vector3 &s)
{
    Matrix T = MatrixTranslate(t.x, t.y, t.z);
    Matrix R = MatrixRotate(axis, angleRad);
    Matrix S = MatrixScale(s.x, s.y, s.z);
    return MatrixMultiply(MatrixMultiply(S, R), T);
}

// childWorld = parentWorld * childLocal,
inline static Matrix ParentChild(const Matrix &parentWorld, const Matrix &childLocal)
{
    return MatrixMultiply(childLocal, parentWorld);
}

// Draw model within context of a coordinate system
static void DrawModelWithWorldMatrix(Model &model, const Matrix &world, Shader shader, Color tint = WHITE)
{
    Matrix prev = model.transform;
    model.transform = world;

    // If this is the shadow pass, hard set the shader and colour
    Shader activeShader = SHADOW_RENDER ? shadows : shader;
    tint = (SHADOW_RENDER) ? activeShadowColour : tint;

    float color[4] =
        {
            tint.r / 255.0f,
            tint.g / 255.0f,
            tint.b / 255.0f,
            tint.a / 255.0f};

    int baseColorLoc = GetShaderLocation(activeShader, "baseColor");
    SetShaderValue(activeShader, baseColorLoc, color, SHADER_UNIFORM_VEC4);

    DrawModel(model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, tint);

    model.transform = prev;
}