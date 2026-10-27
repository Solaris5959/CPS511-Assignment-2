#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "drone.h"
#include "camera_manager.h"
#include "config.h"
#include "renderer.h"

using namespace Config;

Shader lighting;
Shader shadows;
Model cubeModel;
Model cylModel;
Model coneModel;
Model planeModel;

int main()
{
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CPS511 - Drone Simulator");
    SetTargetFPS(60);

    lighting = LoadShaderFromMemory(lightingVs, lightingFs);    // Initialize lighting shader

    cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));       // Init primitive cube mesh
    cylModel = LoadModelFromMesh(GenMeshCylinder(1.0f, 1.0f, 24));      // Init primitive cylinder mesh
    coneModel = LoadModelFromMesh(GenMeshCone(1.0f, 1.0f, 20));         // Init primitive cone mesh
    planeModel = LoadModelFromMesh(GenMeshPlane(32.0f, 32.0f, 16, 16)); // Init primitive plane mesh

    // Render meshes with lighting shader
    cubeModel.materials[0].shader = lighting; 
    cylModel.materials[0].shader = lighting;  
    coneModel.materials[0].shader = lighting; 
    planeModel.materials[0].shader = lighting;

    shadows = LoadShaderFromMemory(shadowVs, shadowFs); // Initialize shadow shader

    // ---- Lighting uniforms ----
    Vector3 lightPos = {10.0f, 40.0f, 15.0f};   // Point Light

    int lightPosLoc = GetShaderLocation(lighting, "lightPos");          // Pass point light position to lighting shader
    int viewPosLoc = GetShaderLocation(lighting, "viewPos");            // Pass viewing position to lighting shader
    int ambientLoc = GetShaderLocation(lighting, "ambientStrength");    // Pass baseline ambient lighting to lighting shader
    int specularLoc = GetShaderLocation(lighting, "specularStrength");  // Pass strength of specular highlights to lighting shader
    int shininessLoc = GetShaderLocation(lighting, "shininess");        // Pass baseline material shininess to lighting shader
    int lightColourLoc = GetShaderLocation(lighting, "lightColor");     // Pass colour of sunlight to lighting shader

    int spotPosLoc = GetShaderLocation(lighting, "spotLightPos");       // Pass spotlight position in global coords to lighting shader
    int spotDirLoc = GetShaderLocation(lighting, "spotLightDir");       // Pass spotlight direction in global coords to lighting shader
    int spotColorLoc = GetShaderLocation(lighting, "spotLightColor");   // Pass spotlight colour to lighting shader
    int spotInnerLoc = GetShaderLocation(lighting, "spotLightInnerCutOff"); // Pass inner spotlight cosine threshold to lighting shader
    int spotOuterLoc = GetShaderLocation(lighting, "spotLightOuterCutOff"); // Pass outer spotlight cosine threshold to lighting shader

    int lightPosShadowLoc = GetShaderLocation(shadows, "lightPos");     // Pass global sunlight vector to shadow shader

    bool isNight = false;       // Flag for nighttime scene
    bool spotlightOn = false;   // Flag for spotlight
    bool showControls = true;   // Flag for controls overlay

    // Set scene defaults
    float ambientIntensity = AMBIENT_STRENGTH;
    Color skyColour = SKYBLUE;
    Vector3 currentLightColour = LIGHT_COLOUR_DAY;

    // Initialize Cameras, set active to third person
    CameraManager thirdPersonCamera(THIRD_PERSON_OFFSET, THIRD_PERSON_TARGET, THIRD_PERSON_FOV);
    CameraManager firstPersonCamera(FIRST_PERSON_OFFSET, FIRST_PERSON_TARGET, FIRST_PERSON_FOV);
    CameraManager *camera = &thirdPersonCamera;

    // Initialize drone at world origin
    Drone drone({0.0f, 0.0f, 0.0f});

    // Main Game Loop
    while (!WindowShouldClose())
    {
        /* Update Variables */

        // First Person Camera, Position: slightly up from center, Target: directly down
        if (IsKeyPressed(KEY_C)) camera = &firstPersonCamera;
        // Third Person Camera, Position: up and back, Target: looking directly at the drone
        if (IsKeyPressed(KEY_V)) camera = &thirdPersonCamera;
        // Nighttime/Daytime scene toggle
        if (IsKeyPressed(KEY_Z)) isNight = !isNight;
        // Spotlight enable/disable toggle
        if (IsKeyPressed(KEY_X)) spotlightOn = !spotlightOn;
        // Controls visibility toggle
        if (IsKeyPressed(KEY_H)) showControls = !showControls;

        // Set scene variables depending on night vs. day scene
        ambientIntensity   = isNight ? (AMBIENT_STRENGTH * 0.2f) : AMBIENT_STRENGTH; 
        skyColour          = isNight ? DARKBLUE : SKYBLUE;
        activeShadowColour = isNight ? NIGHT_SHADOW : DAY_SHADOW;
        currentLightColour = isNight ? LIGHT_COLOUR_NIGHT : LIGHT_COLOUR_DAY;

        drone.update();                                            // Step drone's state
        camera->update(drone.getPosition(), drone.getYawAngle());  // Update camera position relative to drone
        Vector3 currentCamPosition = camera->getCamera().position; // Get camera position for this frame

        // Update information in lighting shader
        SetShaderValue(lighting, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);          // Set uniform point light position in lighting shader
        SetShaderValue(lighting, viewPosLoc, &currentCamPosition, SHADER_UNIFORM_VEC3); // Set uniform view position in lighting shader
        SetShaderValue(lighting, ambientLoc, &ambientIntensity, SHADER_UNIFORM_FLOAT);  // Set uniform baseline ambient light in lighting shader
        SetShaderValue(lighting, specularLoc, &SPECULAR_STRENGTH, SHADER_UNIFORM_FLOAT); // Set uniform specular highlight strength in lighting shader
        SetShaderValue(lighting, shininessLoc, &SHININESS, SHADER_UNIFORM_FLOAT);       // Set uniform material shininess in lighting shader
        SetShaderValue(lighting, lightColourLoc, &currentLightColour, SHADER_UNIFORM_VEC3); // Set uniform point light colour in lighting shader

        SetShaderValue(lighting, spotColorLoc, &SPOTLIGHT_COLOUR, SHADER_UNIFORM_VEC3); // Set uniform spotlight colour in lighting shader
        SetShaderValue(lighting, spotInnerLoc, &SPOTLIGHT_INNER_CUTOFF, SHADER_UNIFORM_FLOAT);  // Set spotlight inner angle cutoff in lighting shader
        SetShaderValue(lighting, spotOuterLoc, &SPOTLIGHT_OUTER_CUTOFF, SHADER_UNIFORM_FLOAT);  // Set spotlight outer angle cutoff in lighting shader

        // Variable for drone's current position (to pass to shader for spotlight)
        Vector3 dronePos = drone.getPosition();

        // Matrices for the drone's current orientation
        Matrix tiltMatrix = MatrixRotate(Vector3UnitX, 20.0f * DEG2RAD);
        Matrix yawMatrix = MatrixRotate(Vector3UnitY, drone.getYawAngle() * DEG2RAD);
        Matrix pitchMatrix = MatrixRotate(Vector3UnitX, drone.getPitchAngle() * DEG2RAD);
        Matrix rollMatrix = MatrixRotate(Vector3UnitZ, drone.getRollAngle() * DEG2RAD);

        // Combine individual orientation matrices into a sum transform
        Matrix transform = MatrixMultiply(tiltMatrix, rollMatrix);
        transform = MatrixMultiply(transform, pitchMatrix);
        transform = MatrixMultiply(transform, yawMatrix);

        // Transform base spotlight direction by drone orientation
        Vector3 droneForward = Vector3Transform(Vector3UnitZ, transform);
        
        // Set spotlight colour depending on enable/disable flag
        if (!spotlightOn) {
            Vector3 black = {0.0f, 0.0f, 0.0f};
            SetShaderValue(lighting, spotColorLoc, &black, SHADER_UNIFORM_VEC3);
        } else {
            SetShaderValue(lighting, spotColorLoc, &SPOTLIGHT_COLOUR, SHADER_UNIFORM_VEC3);
        }

        SetShaderValue(lighting, spotPosLoc, &dronePos, SHADER_UNIFORM_VEC3);       // Set spotlight position in lighting shader
        SetShaderValue(lighting, spotDirLoc, &droneForward, SHADER_UNIFORM_VEC3);   // Set spotlight direction/target in lighting shader

        /* Draw */
        BeginDrawing();
            ClearBackground(skyColour);          // Set world colour
            BeginMode3D(camera->getCamera());    // Set camera in 3D world
                
                /* Model pass */
                Matrix worldRoot = MatrixIdentity();    // Baseline world coordinate system (identity matrix)

                Matrix planeLocal = TRS({0, -0.01f, 0}, Vector3UnitY, 0, UNIT_VECTOR);  // Move plane slightly below y=0 (prevents z-fighting with grid)
                DrawModelWithWorldMatrix(planeModel, ParentChild(worldRoot, planeLocal), lighting, COLOUR_GROUND);  // Draw plane in world with lighting shader
                DrawGrid(32, 1.0f);                                                                                 // Draw grid at y=0

                drone.Draw();   // Draw drone (with lighting shader)


                /* Shadow pass */
                SetShaderValue(shadows, lightPosShadowLoc, &lightPos, SHADER_UNIFORM_VEC3);   // Set uniform sunlight direction in shadow shader

                // Render meshes with shadow shader
                cubeModel.materials[0].shader = shadows;
                cylModel.materials[0].shader = shadows;
                coneModel.materials[0].shader = shadows;
                planeModel.materials[0].shader = shadows;

                SHADOW_RENDER = true;   // Set shadow pass flag
                rlDisableDepthMask();   // Projecting to y=0 plane, prevents z-fighting with grid
                rlDisableBackfaceCulling();

                drone.Draw();           // Draw drone model as a shadow projected onto ground plane

                SHADOW_RENDER = false;  // Reset shadow pass flag
                rlEnableBackfaceCulling();
                rlEnableDepthMask();    // Re-enable depth writing/masking so model on next render pass samples depth
                
                // Reset back to lighting shader for the meshes
                cubeModel.materials[0].shader = lighting;
                cylModel.materials[0].shader = lighting;
                coneModel.materials[0].shader = lighting;
                planeModel.materials[0].shader = lighting;

            EndMode3D();

            /* Draw on-screen controls */
            float scale = (float)SCREEN_HEIGHT / 1080.0f;
            if (scale < 1.0f) scale = 1.0f;

            // Dynamically scaled UI elements
            int headingSize = (int)(24 * scale);
            int textSize = (int)(18 * scale);
            int lineSpacing = (int)(26 * scale);
            int sectionSpacing = (int)(36 * scale);
            int padding = (int)(20 * scale);
            int margin = (int)(15 * scale);

            // Controls window, ripped from readme. Size scales on window size. 
            if (showControls) {
                // Proportional dimensions with a scaled minimum width
                int minWidth = (int)(350 * scale);
                int panelWidth = (SCREEN_WIDTH * 0.25f > minWidth) ? (int)(SCREEN_WIDTH * 0.25f) : minWidth;
                int panelHeight = (int)(SCREEN_HEIGHT * 0.85f);
                
                // Higher Contrast Background: Dark gray/black with high opacity
                DrawRectangle(margin, margin, panelWidth, panelHeight, Fade(BLACK, 0.85f));
                DrawRectangleLines(margin, margin, panelWidth, panelHeight, RAYWHITE);

                int startX = margin + padding;
                int currentY = margin + padding;

                // Title
                DrawText("DRONE CONTROLS\n(Press 'H' to hide)", startX, currentY, headingSize, YELLOW);
                currentY += sectionSpacing + lineSpacing;

                // Movement
                DrawText("Movement", startX, currentY, headingSize, RAYWHITE); currentY += (headingSize + 5);
                DrawText("- W: Move Drone Forward", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- S: Move Drone Backward", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- A: Move Drone Left", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- D: Move Drone Right", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- R: Move Drone Up", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- F: Move Drone Down", startX + padding, currentY, textSize, LIGHTGRAY); currentY += sectionSpacing;

                // Rotation
                DrawText("Rotation", startX, currentY, headingSize, RAYWHITE); currentY += (headingSize + 5);
                DrawText("- Q: Yaw Drone Left", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- E: Yaw Drone Right", startX + padding, currentY, textSize, LIGHTGRAY); currentY += sectionSpacing;

                // Camera
                DrawText("Camera", startX, currentY, headingSize, RAYWHITE); currentY += (headingSize + 5);
                DrawText("- C: Switch to First Person", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- V: Switch to Third Person", startX + padding, currentY, textSize, LIGHTGRAY); currentY += sectionSpacing;

                // Environment & Toggles
                DrawText("Environment & Toggles", startX, currentY, headingSize, RAYWHITE); currentY += (headingSize + 5);
                DrawText("- Z: Switch Day/Night", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- X: Toggle Spotlight", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- Spacebar: Turn Drone On/Off", startX + padding, currentY, textSize, LIGHTGRAY); currentY += lineSpacing;
                DrawText("- Home: Reset Position", startX + padding, currentY, textSize, LIGHTGRAY);

            } else {
                // Show a scaled hint when hidden
                int hintWidth = (int)(260 * scale);
                int hintHeight = (int)(40 * scale);
                DrawRectangle(margin, margin, hintWidth, hintHeight, Fade(BLACK, 0.85f));
                DrawRectangleLines(margin, margin, hintWidth, hintHeight, RAYWHITE);
                DrawText("Press 'H' to show controls", margin + (int)(15 * scale), margin + (int)(12 * scale), textSize, YELLOW);
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
