#include "raylib.h"
#include "raymath.h"

#include "camera_manager.h"

CameraManager::CameraManager(Vector3 baseOffset, Vector3 baseTarget, float FOV, CameraProjection cameraType)
{
    // Initialize drone-relative positions
    positionOffset = baseOffset;
    targetOffset = baseTarget;
    baseUp = (cameraType == CAMERA_ORTHOGRAPHIC) ? Vector3UnitZ : Vector3UnitY; // Sky relative to camera (+y if perspective, +z if orthographic)

    // Initialize camera settings
    camera = {0};
    camera.fovy = FOV;
    camera.up = baseUp; 
    camera.projection = cameraType;

    camera.position = baseOffset; // Where camera is in scene
    camera.target = baseTarget;   // Where camera is pointing
}

void CameraManager::update(Vector3 dronePosition, float rotationAngleDegrees)
{
    // Convert drone yaw to radians
    float angleRadians = rotationAngleDegrees * DEG2RAD;

    // Create rotation matrix for offset
    Matrix rotation = MatrixRotateY(angleRadians);

    // Rotate offset, target, and up vectors by drone yaw
    Vector3 rotatedPosOffset = Vector3Transform(positionOffset, rotation);
    Vector3 rotatedTargetOffset = Vector3Transform(targetOffset, rotation);
    Vector3 rotatedUp = Vector3Transform(baseUp, rotation);

    // Position, target, and up of camera relative to drone
    camera.position = Vector3Add(dronePosition, rotatedPosOffset);
    camera.target = Vector3Add(dronePosition, rotatedTargetOffset);
    camera.up = rotatedUp;
}

Camera3D CameraManager::getCamera() const
{
    return camera;
}