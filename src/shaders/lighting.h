static const char *lightingVs = R"(
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragPos;
out vec3 fragNormal;
out vec4 fragColor;

void main()
{
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPos = worldPos.xyz;

    // Correct normal transform for non-uniform scaling
    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;

    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

static const char *lightingFs = R"(
#version 330

in vec3 fragPos;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform vec3 lightColor;

uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor;
uniform float spotLightInnerCutOff; // Cosine of inner angle
uniform float spotLightOuterCutOff; // Cosine of outer angle

void main()
{
    /* Point Light calculation */
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos - fragPos);
    vec3 R = reflect(-L, N);

    // Ambient
    vec3 ambient = ambientStrength * baseColor.rgb;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * baseColor.rgb;

    // Specular
    float spec = 0.0;
    if (diff > 0.0)
    {
        spec = pow(max(dot(V, R), 0.0), shininess);
    }
    vec3 specular = specularStrength * spec * lightColor;

    float d = length(fragPos - lightPos) * (1.0 / 32.0); // Distance of fragment from light
    float attenuation = 1 / (1 + 0.05 * d + 0.05 * d * d); // Attenuation Factor 

    vec3 result = ambient + (attenuation * (diffuse + specular));


    /* On-Drone Spotlight Calculation */
    vec3 spotLightToFrag = normalize(spotLightPos - fragPos);

    float theta = dot(spotLightToFrag, normalize(-spotLightDir)); // Get angle (cos) between fragment and center of spotlight beam

    // Smooth edge calculation
    float epsilon = spotLightInnerCutOff - spotLightOuterCutOff;
    float intensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);

    // Only calculate light if fragment is inside the outer cone
    if(theta > spotLightOuterCutOff) 
    {
        // Spotlight Diffuse
        float spotDiff = max(dot(N, spotLightToFrag), 0.0);
        vec3 spotDiffuse = spotDiff * spotLightColor * baseColor.rgb;

        // Spotlight Specular
        vec3 spotR = reflect(-spotLightToFrag, N);
        float spotSpec = pow(max(dot(V, spotR), 0.0), shininess);
        vec3 spotSpecular = specularStrength * spotSpec * spotLightColor;

        // Spotlight Attenuation (Distance fade)
        float spotD = length(spotLightPos - fragPos) * (1.0 / 32.0);
        float spotAttenuation = 1.0 / (1.0 + 0.05 * spotD + 0.05 * spotD * spotD);

        // Apply intensity (cone fade) and attenuation (distance fade)
        vec3 spotResult = spotAttenuation * intensity * (spotDiffuse + spotSpecular);
        
        // Add spotlight to the sun's result
        result += spotResult; 
    }

    finalColor = vec4(result, baseColor.a) * fragColor;
}
)";
