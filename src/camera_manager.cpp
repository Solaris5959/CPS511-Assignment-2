#include "raylib.h"
#include "raymath.h"

#include "camera_manager.h"
#include "config.h"

CameraManager::CameraManager(Vector3 baseOffset, Vector3 baseTarget, float FOV)
{
    // Initialize drone-relative positions
    positionOffset = baseOffset;
    targetOffset = baseTarget;

    // Initialize camera settings
    camera = {0};
    camera.fovy = FOV;
    camera.up = (Vector3){0.0f, 1.0f, 0.0f}; // Sky relative to camera
    camera.projection = CAMERA_PERSPECTIVE;

    camera.position = baseOffset; // Where camera is in scene
    camera.target = baseTarget;   // Where camera is pointing
}

void CameraManager::update(Vector3 dronePosition, float rotationAngleDegrees)
{
    // Convert drone yaw to radians
    float angleRadians = rotationAngleDegrees * DEG2RAD;

    // Create rotation matrix for offset
    Matrix rotation = MatrixRotateY(angleRadians);

    // Rotate offset and target vectors by drone yaw
    Vector3 rotatedPosOffset = Vector3Transform(positionOffset, rotation);
    Vector3 rotatedTargetOffset = Vector3Transform(targetOffset, rotation);

    // Position and target camera relative to drone
    camera.position = Vector3Add(dronePosition, rotatedPosOffset);
    camera.target = Vector3Add(dronePosition, rotatedTargetOffset);
}

Camera3D CameraManager::getCamera() const
{
    return camera;
}