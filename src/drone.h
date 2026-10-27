#pragma once
#include <raylib.h>

/**
 * \class Drone
 * \brief Class to draw, hold the state of, and perform user input operations on a drone.
 *
 * The Drone class contains information on the drone's world position, relative yaw/pitch/roll,
 * and velocity. Reads user input controls to influence the velocity, which in turn drives changes
 * in world position.
 *
 * Additionally holds drawing functions for the drone, which apply the local transformations within
 * the context of their parent coordinate systems. Some of these transformations are driven by the
 * state of the drone (the rotor spin for example).
 */
class Drone
{
private:
    Vector3 position;
    Vector3 velocity;
    float yawAngle = 0.0f;
    float pitchAngle = 0.0f;
    float rollAngle = 0.0f;
    float rotorAngle = 0.0f;
    float grabberExtension = 0.2f;

    float rotorSpeed;

    bool onState;

    /**
     * \brief Helper to apply velocity to current position and step tilt towards target.
     * 
     * Handles the framerate independent stepping of the drone, including rotor position, 
     * drone position, velocity decay, map bounding, and pitch/roll. 
     * 
     * Clamps velocity within max speed, and halts movement (zeroes velocity) if the
     * current velocity is low enough. Additionally pitches and yaws the drone proportional
     * to it's current velocity and negative yaw, which means it leans into lateral movements.
     * 
     * Applies current velocity vector to drone's world position, ensures drone respects the
     * ground plane as a boundary on y, and then decays the drone's velocity by globally
     * configured value. If the drone is in an off state, instead does not decay at all,
     * to allow the drone to crash to the ground.
     */
    void updatePosition();

    /**
     * \brief Helper to draw drone chassis in context of drone coordinates.
     * 
     * \param droneWorld Parent Drone rotation and translation matrix
     */
    void DrawChassis(const Matrix &droneWorld);

    /**
     * \brief Helper to draw drone roll-cage/exo-frame in context of drone coordinates.
     * 
     * \param droneWorld Parent Drone rotation and translation matrix
     */
    void DrawExoFrame(const Matrix &droneWorld);

    /**
     * \brief Helper to draw drone skids and grabbers in context of drone coordinates.
     * 
     * Draws the skids/legs of the drone, as well as the dynamic grabber arms 
     * attached to them. Opening amount is driven by parameter, which is clamped
     * by globally configured values so as to not break the model. 
     * 
     * \param droneWorld Parent Drone rotation and translation matrix
     * \param grabberExtension Current offset of grabbers
     */
    void DrawGrabbers(const Matrix &droneWorld, float grabberExtension);

    /**
     * \brief Helper to draw drone arms and rotors in context of drone coordinates.
     * 
     * Draws 4 arms and 2 sets of rotors per arm. Rotors rotate with their parent 
     * shaft, in opposite directions to their neighbours (both laterally and 
     * vertically).
     * 
     * \param droneWorld Parent Drone rotation and translation matrix
     */
    void DrawArmsAndRotors(const Matrix &droneWorld);

    /**
     * \brief Helper to draw a set of 3 rotor blades at a given offset in Y.
     * 
     * Draws the rotor blades given a parent world matrix and y offset. Applies
     * the scaling to create rotor shape, twists/rotates them about Z so they are
     * 'lifting' air in realistic ways, then offsets about the shaft and rotates
     * them 0, 120, 240 degrees. 
     * \see DrawArmsAndRotors
     * 
     * \param parentShaftWorld World coordinate system of parent rotor shaft
     * \param yOffset Offset to draw rotors relative to base of shaft cylModel
     */
    void DrawRotorSet(Matrix parentShaftWorld, float yOffset);

public:
    /**
     * \brief Drone object constructor
     *
     * Initializes drone world position, rotor spinning speed, velocity, and on-state.
     *
     * \param startPosition 3D Vector of starting position
     */
    Drone(Vector3 startPosition);

    /**
     * \brief Returns current drone yaw angle
     *
     * Returns the drone's current yaw angle in degrees.
     * Used in the camera position/target calculations.
     *
     * \returns Private yawAngle member
     */
    float getYawAngle();

    /**
     * \brief Returns current drone pitch angle
     *
     * Returns the drone's current pitch angle in degrees.
     * Used in the camera position/target calculations.
     *
     * \returns Private pitchAngle member
     */
    float getPitchAngle();

    /**
     * \brief Returns current drone roll angle
     *
     * Returns the drone's current roll angle in degrees.
     * Used in the camera position/target calculations.
     *
     * \returns Private rollAngle member
     */
    float getRollAngle();

    /**
     * \brief Returns current drone position in world
     *
     * Returns the drone's current position in world coordinates.
     *
     * \returns Drone world position as a 3D vector.
     */
    Vector3 getPosition();

    /**
     * \brief Applies acceleration to the drone's current velocity
     *
     * Given a 3D vector `acceleration`, applies that vector to the current world velocity,
     * relative to the drone's current heading.
     *
     * \param acceleration 3D acceleration vector to be applied to the velocity
     */
    void addVelocity(Vector3 offset);

    /**
     * \brief Increments drone's yaw angle by deltaAngle
     *
     * Given a float `deltaAngle`, increments the drone's yaw angle private member.
     *
     * \param deltaAngle The change in yaw angle, positive or negative
     */
    void yaw(float deltaAngle);

    /**
     * \brief Resets drone to world origin, with no velocity in an off state.
     *
     * When the user presses the Home key, returns the drone to a default state.
     * Resets the drone back to the world origin, sets velocity to 0, immediately
     * turns off the rotors, and "switches off" (puts in off state).
     */
    void resetToOrigin();

    /**
     * \brief Update function called every frame to handle user input and update drone state.
     *
     * An update function that is called every drawn frame, which reads drone-relevant user input,
     * builds an acceleration vector from that information, and then calls helper to update drone
     * position - \see updatePosition().
     *
     * If the drone is in an off-state, the velocity will directly be accelerated in -y by gravity.
     *
     * Independent of state, read for reset and on/off toggles, and ramp up/down rotor speed to
     * target derived by motion.
     */
    void update();

    /**
     * \brief Creates coordinate system for drone, calls helper draw functions to draw the entire drone.
     *
     * Given the drone's current yaw, pitch, roll, and position, create a combined transformation matrix 
     * to be applied to every component of the drone. With that matrix, call the drawing helper functions and
     * pass it in as the new world coordinates. This makes subsequent component in the heirarchy inherit 
     * the transformations.
     * 
     * \see DrawChassis(const Matrix &droneWorld)
     * \see DrawExoFrame(const Matrix &droneWorld)
     * \see void DrawGrabbers(const Matrix &droneWorld, float grabberExtension)
     * \see DrawArmsAndRotors(const Matrix &droneWorld)
     */
    void Draw();
};
