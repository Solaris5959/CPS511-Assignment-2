#pragma once
#include <raylib.h>

/**
 * \class CameraManager
 * \brief Manages the position and state of a Camera3D object.
 * 
 * The CameraManager class initializes a Raylib Camera3D object, and handles
 * information regarding it's position offset to a given drone, the target
 * it is pointing at (also relative to a target drone), and the state of
 * the camera object. 
 */
class CameraManager
{
private:
    Camera3D camera;
    Vector3 positionOffset;
    Vector3 targetOffset;

public:
    /**
     * \brief Initialize CameraManager and Camera given an offset and target relative to drone. 
     * 
     * Initializes CameraManager object, with a private camera object whose position and target
     * are driven by the passed in offsets and target - both of which are relative to the drone
     * it is following. Sets camera object information to a default value, with a passed in
     * FOV setting to allow for different types of cameras.
     * 
     * \param baseOffset Camera offset relative to local drone coordinates
     * \param baseTarget Camera target relative to local drone coordinates
     * \param FOV Field of view of Camera3D object
     * \param cameraType
     */
    CameraManager(Vector3 baseOffset, Vector3 baseTarget, float FOV, CameraProjection cameraType);

    /**
     * \brief Update camera position based on drone position.
     * 
     * Takes in position and yaw of drone the camera is following, and then updates the
     * global position of the camera, adding the local preset offset and target 
     * 
     * \param dronePosition World coordinate location of drone
     * \param rotationAngleDegree World coordinate yaw of drone
     */
    void update(Vector3 dronePosition, float rotationAngleDegree);

    /**
     * \brief Return immutable reference to internal Camera3D object.
     */
    Camera3D getCamera() const;
};
