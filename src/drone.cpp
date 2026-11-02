#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <algorithm>

#include "drone.h"
#include "config.h"
#include "renderer.h"

using namespace Config;

inline float min(float a, float b) { // Helper inline for minimum of two floats
    return (a < b) ? a : b;
}

Drone::Drone(Vector3 startPosition) // Constructor, handles drone and primitives
{
    position = startPosition;
    position.y += DRONE_FLOOR;
    rotorSpeed = ROTOR_OFF_SPEED;
    velocity = Vector3Zero();

    onState = false;
}

float Drone::getYawAngle() { 
    return yawAngle;
}

float Drone::getPitchAngle() { 
    return pitchAngle;
}

float Drone::getRollAngle() { 
    return rollAngle;
}

Vector3 Drone::getPosition()
{
    return position;
}

void Drone::addVelocity(Vector3 acceleration) // Add velocity derived from user input to drone velocity
{
    // Get current yaw rotation in radians
    float rotateAngleRadians = yawAngle * DEG2RAD;

    // Create rotation matrix for the incoming acceleration from the yaw
    Matrix rotation = MatrixRotateY(rotateAngleRadians);

    // 
    Vector3 worldAcceleration = Vector3Transform(acceleration, rotation);

    velocity = Vector3Add(velocity, worldAcceleration);
}

void Drone::yaw(float deltaAngle) // Rotate the drone about the Y axis, clamped to [0,359]
{
    yawAngle += deltaAngle;

    if (yawAngle >= 360) yawAngle -= 360;
    if (yawAngle < 0) yawAngle += 360;
}

void Drone::resetToOrigin() // Return drone to origin with 0 velocity
{
    position = Vector3Zero();
    velocity = Vector3Zero();
    yawAngle = 0;
    rotorSpeed = ROTOR_OFF_SPEED;
    onState = false;
}

void Drone::update() // Handle user input, drives movement and reset functionality
{
    if (IsKeyPressed(KEY_SPACE)) onState = !onState; // Toggle rotor spin
    if (IsKeyPressed(KEY_HOME)) resetToOrigin();

    // Set target RPM to 0, will be overwritten if drone is on
    float targetRotorSpeed = ROTOR_OFF_SPEED;

    // If drone is on, read user input, if not, fall with gravity
    if (onState) {
        // Initial input acceleration is 0 in all directions
        Vector3 inputDir = {0.0f, 0.0f, 0.0f};

        // Set target RPM to default, can be changed by ascending/descending
        targetRotorSpeed = ROTOR_NORMAL_SPEED;

        // Add components of acceleration depending on user input
        if (IsKeyDown(KEY_W)) inputDir.z += 1.0f;
        if (IsKeyDown(KEY_S)) inputDir.z -= 1.0f;
        if (IsKeyDown(KEY_A)) inputDir.x += 1.0f;
        if (IsKeyDown(KEY_D)) inputDir.x -= 1.0f;

        // If ascending or descending, update target rotor RPM
        if (IsKeyDown(KEY_R)) {inputDir.y += 1.0f; targetRotorSpeed = ROTOR_FAST_SPEED;}
        if (IsKeyDown(KEY_F)) {inputDir.y -= 1.0f; targetRotorSpeed = ROTOR_SLOW_SPEED;}

        // If input acceleration is non-zero, scale the vector by speed factor and frame time
        // and add to current velocity
        if (Vector3Length(inputDir) > 0.0f) {
            inputDir = Vector3Scale(inputDir, SPEED_INCREMENT * GetFrameTime());

            addVelocity(inputDir);
        }

        // Yaw left or right
        if (IsKeyDown(KEY_Q)) yaw(YAW_SPEED * GetFrameTime());
        if (IsKeyDown(KEY_E)) yaw(-YAW_SPEED * GetFrameTime());
    } else {
        // Add framerate independent gravity acceleration to current velocity
        velocity.y -= G * GetFrameTime();
    }

    // Ramp up/down rotor speed to reach target, independent of framerate
    rotorSpeed += (targetRotorSpeed - rotorSpeed) * 3.0f * GetFrameTime();

    // Update drone position based on updated instantaneous velocity 
    updatePosition();
}

void Drone::updatePosition() // Updates drone position and pitch/roll based on velocity
{ 
    float dt = GetFrameTime();

    // Apply rotation to rotor, clamped to [0, 360)
    rotorAngle += rotorSpeed * dt;
    if (rotorAngle >= 360) rotorAngle -= 360;
    if (rotorAngle < 0) rotorAngle += 360;

    float currentSpeed = Vector3Length(velocity);
    // If current velocity exceeds maximum, normalize it and scale up to the max speed
    // Exception for if drone is off, allows gravity to accumulate
    if (onState && currentSpeed > MAX_SPEED) velocity = Vector3Scale(velocity, MAX_SPEED / currentSpeed);
    // If current speed is sufficiently low, stop movement
    if (currentSpeed < 0.05f) velocity = Vector3Zero();

    // Mirrors yaw rotation to mimic centrifugal tilt,
    // otherwise drone tilts in the wrong (opposite) direction.
    Matrix invYaw = MatrixRotateY(-yawAngle * DEG2RAD);
    Vector3 localVelocity = Vector3Transform(velocity, invYaw);

    // Update pitch and roll depending on proportion of x and z velocity to max speed
    float targetPitch = (localVelocity.z / MAX_SPEED) * MAX_PITCH;
    float targetRoll = -(localVelocity.x / MAX_SPEED) * MAX_ROLL;

    // Clamp angles within their globally configured max
    pitchAngle = std::clamp(targetPitch, -MAX_PITCH, MAX_PITCH);
    rollAngle = std::clamp(targetRoll, -MAX_ROLL, MAX_ROLL);

    // Step movement by velocity * dt
    Vector3 frameMovement = Vector3Scale(velocity, dt);
    position = Vector3Add(position, frameMovement);

    if (position.y < DRONE_FLOOR) position.y = DRONE_FLOOR; // Clamp drone y to [0.0, inf)

    // Decay velocity, independent of framerate
    float decayMultiplier = powf(PRESERVED_VELOCITY, dt);
    velocity = Vector3Scale(velocity, decayMultiplier);

    // If rotors are off, drone should fall with gravity
    if (!onState) velocity.y = (velocity.y / decayMultiplier); 
}

void Drone::Draw()
{
    // Rotation matrices for the 3 axis of movement driven rotation
    Matrix rotPitch = MatrixRotateX(pitchAngle * DEG2RAD);
    Matrix rotRoll  = MatrixRotateZ(rollAngle * DEG2RAD);
    Matrix rotYaw   = MatrixRotateY(yawAngle * DEG2RAD);

    // Combine all 3 rotations into a single matrix
    Matrix combinedRotation = MatrixMultiply(MatrixMultiply(rotRoll, rotPitch), rotYaw);

    // Create translation matrix of drone in world coordinates
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);

    // Create `drone world` coordinates by applying the combined rotation, then the translation.
    // `drone world` is the parent transformation for all components of the drone.
    Matrix droneWorld = MatrixMultiply(combinedRotation, translation);

    // Call the segmented draw routines
    DrawChassis(droneWorld);
    DrawExoFrame(droneWorld);
    DrawGrabbers(droneWorld, grabberExtension);
    DrawArmsAndRotors(droneWorld);
}

void Drone::DrawChassis(const Matrix& droneWorld)
{
    Matrix droneBodyLocal = TRS(
        Vector3Zero(), 
        Vector3UnitY, 0, 
        {BODY_WIDTH, BODY_HEIGHT, BODY_LENGTH}
    );
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, droneBodyLocal), lighting, COLOUR_CHASSIS);

    Matrix droneHeadLocal = TRS(
        {0, 0, BODY_LENGTH / 2 + 0.15f}, 
        Vector3UnitY, 0, 
        {0.65f, 0.5f, 0.3f}
    );

    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, droneHeadLocal), lighting, COLOUR_ACCENT);
}

void Drone::DrawExoFrame(const Matrix& droneWorld)
{
    float cx = (BODY_WIDTH / 2.0f) + EXO_MARGIN;
    float cy = (BODY_HEIGHT / 2.0f) + EXO_MARGIN;
    float cz = (BODY_LENGTH / 2.0f) + EXO_MARGIN;
    
    float pipeRad = EXO_PIPE_THICK / 2.0f; 

    // Corner Boxes/Junctions 
    for (int x : {-1, 1}) {
        for (int y : {-1, 1}) {
            for (int z : {-1, 1}) {
                Matrix cornerLocal = TRS({cx * x, cy * y, cz * z}, Vector3UnitY, 0, {EXO_CORNER_SIZE, EXO_CORNER_SIZE, EXO_CORNER_SIZE});
                DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, cornerLocal), lighting, COLOUR_MECHANICAL);
            }
        }
    }

    // Longitudinal Pipes (Z-axis)
    for (int x : {-1, 1}) {
        for (int y : {-1, 1}) {
            Matrix pipeLocal = TRS(
                {cx * x, cy * y, -cz}, 
                {1.0f, 0.0f, 0.0f}, 90.0f * DEG2RAD, 
                {pipeRad, cz * 2.0f, pipeRad} 
            );
            DrawModelWithWorldMatrix(cylModel, ParentChild(droneWorld, pipeLocal), lighting, COLOUR_FRAME);
        }
    }

    // Lateral Pipes (X-axis)
    for (int y : {-1, 1}) {
        for (int z : {-1, 1}) {
            Matrix pipeLocal = TRS(
                {cx, cy * y, cz * z}, 
                {0.0f, 0.0f, 1.0f}, 90.0f * DEG2RAD, 
                {pipeRad, cx * 2.0f, pipeRad} 
            );
            DrawModelWithWorldMatrix(cylModel, ParentChild(droneWorld, pipeLocal), lighting, COLOUR_FRAME);
        }
    }

    // Vertical Pipes (Y-axis)
    for (int x : {-1, 1}) {
        for (int z : {-1, 1}) {
            Matrix pipeLocal = TRS(
                {cx * x, -cy, cz * z}, 
                Vector3UnitY, 0.0f, 
                {pipeRad, cy * 2.0f, pipeRad}
            );
            DrawModelWithWorldMatrix(cylModel, ParentChild(droneWorld, pipeLocal), lighting, COLOUR_FRAME);
        }
    }
}

void Drone::DrawGrabbers(const Matrix& droneWorld, float grabberExt)
{
    // Calculate offsets for legs and grabber based on grabber extension
    float legXBase = (BODY_WIDTH / 2.0f) + grabberExt;
    float strutWidth = grabberExt; 
    float strutCenterX = (BODY_WIDTH / 2.0f) + (grabberExt / 2.0f) - 0.05f;

    // Horizontal sliding struts (Mechanical)
    Matrix leftStrutLocal = TRS({strutCenterX, -(BODY_HEIGHT / 2.0f), 0.0f}, Vector3UnitY, 0, {strutWidth, 0.08f, BODY_LENGTH * 0.4f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, leftStrutLocal), lighting, COLOUR_MECHANICAL);
    Matrix rightStrutLocal = TRS({-strutCenterX, -(BODY_HEIGHT / 2.0f), 0.0f}, Vector3UnitY, 0, {strutWidth, 0.08f, BODY_LENGTH * 0.4f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, rightStrutLocal), lighting, COLOUR_MECHANICAL);

    // Top Rails, Legs, and Skids (Frame)
    Matrix leftTopRailLocal = TRS({legXBase - 0.05f, -(BODY_HEIGHT / 2.0f), 0.0f}, Vector3UnitY, 0, {0.1f, 0.1f, BODY_LENGTH});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, leftTopRailLocal), lighting, COLOUR_FRAME);
    Matrix rightTopRailLocal = TRS({-(legXBase - 0.05f), -(BODY_HEIGHT / 2.0f), 0.0f}, Vector3UnitY, 0, {0.1f, 0.1f, BODY_LENGTH});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, rightTopRailLocal), lighting, COLOUR_FRAME);

    Matrix leftFrontLegLocal = TRS({legXBase - 0.05f, -(BODY_HEIGHT / 2.0f) - 0.3f, (BODY_LENGTH / 2.0) - 0.05f}, Vector3UnitY, 0, {0.1f, 0.6f, 0.1f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, leftFrontLegLocal), lighting, COLOUR_FRAME);
    Matrix rightFrontLegLocal = TRS({-(legXBase - 0.05f), -(BODY_HEIGHT / 2.0f) - 0.3f, (BODY_LENGTH / 2.0f) - 0.05f}, Vector3UnitY, 0, {0.1f, 0.6f, 0.1f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, rightFrontLegLocal), lighting, COLOUR_FRAME);
    
    Matrix leftBackLegLocal = TRS({legXBase - 0.05f, -(BODY_HEIGHT / 2.0f) - 0.3f, -(BODY_LENGTH / 2.0f) + 0.05f}, Vector3UnitY, 0, {0.1f, 0.6f, 0.1f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, leftBackLegLocal), lighting, COLOUR_FRAME);
    Matrix rightBackLegLocal = TRS({-(legXBase - 0.05f), -(BODY_HEIGHT / 2.0f) - 0.3f, -(BODY_LENGTH / 2.0f) + 0.05f}, Vector3UnitY, 0, {0.1f, 0.6f, 0.1f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, rightBackLegLocal), lighting, COLOUR_FRAME);

    Matrix leftSkidLocal = TRS({legXBase - 0.05f, -(BODY_HEIGHT / 2.0f) - 0.55f, 0.0f}, Vector3UnitY, 0, {0.1f, 0.1f, BODY_LENGTH});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, leftSkidLocal), lighting, COLOUR_FRAME);
    Matrix rightSkidLocal = TRS({-(legXBase - 0.05f), -(BODY_HEIGHT / 2.0f) - 0.55f, 0.0f}, Vector3UnitY, 0, {0.1f, 0.1f, BODY_LENGTH});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, rightSkidLocal), lighting, COLOUR_FRAME);

    // Inward-facing grabber teeth (Accent)
    Matrix leftToothLocal = TRS({legXBase - 0.2f, -(BODY_HEIGHT / 2.0f) - 0.55f, 0.0f}, Vector3UnitY, 0, {0.2f, 0.05f, BODY_LENGTH * 0.4f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, leftToothLocal), lighting, COLOUR_ACCENT);
    Matrix rightToothLocal = TRS({-(legXBase - 0.2f), -(BODY_HEIGHT / 2.0f) - 0.55f, 0.0f}, Vector3UnitY, 0, {0.2f, 0.05f, BODY_LENGTH * 0.4f});
    DrawModelWithWorldMatrix(cubeModel, ParentChild(droneWorld, rightToothLocal), lighting, COLOUR_ACCENT);
}

void Drone::DrawArmsAndRotors(const Matrix& droneWorld)
{
    // Draw 4 arms about chassis
    for (int i = 0; i < 4; i++)
    {
        // Stretch and offset arms before rotating them by globally
        // configured rotation angles
        Matrix armScale = MatrixScale(ARM_WIDTH, ARM_HEIGHT, ARM_LENGTH);
        Matrix armOutwardOffset = MatrixTranslate(0.0f, 0.0f, ARM_OFFSET);
        Matrix armYawRotation = MatrixRotateY(ARM_ANGLES[i] * DEG2RAD);
        
        Matrix armLocal = MatrixMultiply(MatrixMultiply(armScale, armOutwardOffset), armYawRotation);
        Matrix armWorld = ParentChild(droneWorld, armLocal);
        DrawModelWithWorldMatrix(cylModel, armWorld, lighting, COLOUR_FRAME); // Matching arms to frame ties the design together

        // Reverse scaling of arms
        Matrix unscaledRotorMountLocal = TRS({0, 0, 0.95f}, Vector3UnitY, 0, {1.0f / ARM_WIDTH, 1.0f / ARM_HEIGHT, 1.0f / ARM_LENGTH});
        Matrix rotorAnchorWorld = ParentChild(armWorld, unscaledRotorMountLocal);

        // Motor Housing
        Matrix motorHousingLocal = TRS({0, -0.05f, 0}, Vector3UnitY, 0.0f, {0.1f, 0.2f, 0.1f});
        DrawModelWithWorldMatrix(cylModel, ParentChild(rotorAnchorWorld, motorHousingLocal), lighting, COLOUR_FRAME);

        // Rotor 
        float spinDirection = (i % 2 == 0) ? 1.0f : -1.0f;
        float currentShaftAngle = rotorAngle * spinDirection;

        // Set shaft scale, then apply offsets and rotation for bottom and top segments
        Matrix motorShaftScale = MatrixScale(0.05, 0.25f, 0.05);
        Matrix motorShaftTopLocal = TRS({0, 0.05, 0}, Vector3UnitY, currentShaftAngle * DEG2RAD, UNIT_VECTOR);
        Matrix motorShaftBottomLocal = TRS({0, -0.2f, 0}, Vector3UnitY, -currentShaftAngle * DEG2RAD, UNIT_VECTOR); // Spin opposite

        // Create shaft world coordinate system for rotors
        Matrix motorShaftTopWorld = ParentChild(rotorAnchorWorld, motorShaftTopLocal);
        Matrix motorShaftBottomWorld = ParentChild(rotorAnchorWorld, motorShaftBottomLocal);

        DrawModelWithWorldMatrix(cylModel, MatrixMultiply(motorShaftScale, motorShaftTopWorld), lighting, COLOUR_MECHANICAL);
        DrawModelWithWorldMatrix(cylModel, MatrixMultiply(motorShaftScale, motorShaftBottomWorld), lighting, COLOUR_MECHANICAL);

        // Upper Rotors
        DrawRotorSet(motorShaftTopWorld, 0.2f);

        // Lower Rotors
        DrawRotorSet(motorShaftBottomWorld, 0.05f);
    }
}

void Drone::DrawRotorSet(Matrix parentShaftWorld, float yOffset) {
    for (int j = 0; j < 3; j++) {
        // Scale, then twist (rotate about Z), then offset, then rotate about shaft (Y)
        Matrix bladeScale  = MatrixScale(0.1f, 0.02f, 0.7f);
        Matrix bladeTwist  = MatrixRotateZ(30.0f * DEG2RAD);
        Matrix bladeOffset = MatrixTranslate(0.0f, yOffset, 0.35f);
        Matrix bladeSpin   = MatrixRotateY(120.0f * j * DEG2RAD);
        Matrix bladeLocal  = MatrixMultiply(MatrixMultiply(MatrixMultiply(bladeScale, bladeTwist), bladeOffset), bladeSpin);

        DrawModelWithWorldMatrix(cubeModel, ParentChild(parentShaftWorld, bladeLocal), lighting, COLOUR_ROTORS);
    }
}

