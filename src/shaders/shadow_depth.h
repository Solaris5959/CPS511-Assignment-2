static const char *shadowVs = R"(
#version 330

// Receive vertex information
in vec3 vertexPosition;
in vec3 vertexNormal;

// Vector for sun lighting, must be passed in manually
uniform vec3 lightPos;

// Receive draw-to-screen matrices
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

// Send vAlignment and vShadowBoundsXZ to frag shader
out float vAlignment;
out vec2 vShadowBoundsXZ;

void main()
{
    // Calculate world position of vertex based on full model transformation
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);

    vec3 lightingUnitVec = normalize(lightPos - worldPos.xyz);

    // Calculate scaling factor for sunlight direction vector to intersect with 
    // y = 0 plane starting at the vertex world position.
    float t = -worldPos.y / lightingUnitVec.y;

    // Create a new vector, which is the result of scaling the light direction
    // from the vertex to the ground plane.
    vec3 shadowPos = (t * lightingUnitVec) + worldPos.xyz;
    shadowPos.y += 0.001; // Move projection slightly above ground plane (stops z-fighting)
    
    // Calculate normal of vertex in world coordinates. 
    vec3 normalWorld = mat3(transpose(inverse(matModel))) * vertexNormal;

    // Calculate cos of angle between world coordinate vertex normal and sunlight, send to frag shader
    vAlignment = dot(normalize(normalWorld), normalize(lightingUnitVec)); 
    // Get the x and y coordinates of the projected shadow vertex, send to frag shader
    vShadowBoundsXZ = shadowPos.xz;
    
    // Draw the vertex at its screen position (view/camera matrix first, then project to screen)
    gl_Position =  matProjection * matView * vec4(shadowPos, 1.0);
}
)";

static const char *shadowFs = R"(
#version 330

// Receive vertex alignment and bounds information from vertex shader
in float vAlignment;
in vec2 vShadowBoundsXZ;

// Shadow colour, must be passed in manually
uniform vec4 baseColor;

// Sends out the final colour to the renderer
out vec4 finalColor;

void main() {
    float bounds = 32.0;

    // Cull faces not visible to the sun, and shadows outside the ground plane
    if (vAlignment > 0.0 || 
        vShadowBoundsXZ.x > bounds || vShadowBoundsXZ.x < -bounds || 
        vShadowBoundsXZ.y > bounds || vShadowBoundsXZ.y < -bounds) 
    {
        discard;
    }

    // Set colour of remaining fragments
    finalColor = baseColor;
}
)";