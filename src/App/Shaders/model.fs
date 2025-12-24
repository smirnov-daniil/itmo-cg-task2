#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D diffuseTexture;
uniform samplerCube skybox;

uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;
uniform float dirLightIntensity;
uniform bool dirLightEnabled;

uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform float spotLightIntensity;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;
uniform bool spotLightEnabled;

uniform vec3 viewPos;

out vec4 FragColor;

vec3 calculateDirectionalLight(vec3 normal, vec3 viewDir, vec3 color)
{
    float ambientStrength = 0.2 * dirLightIntensity;
    vec3 ambient = ambientStrength * dirLightColor;
    
    vec3 lightDir = normalize(-dirLightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = dirLightIntensity * diff * dirLightColor;
    
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * dirLightIntensity * spec * dirLightColor;
    
    return (ambient + diffuse + specular) * color;
}

vec3 calculateSpotLight(vec3 normal, vec3 viewDir, vec3 color)
{
    vec3 lightDir = normalize(spotLightPosition - fragPos);
    
    float theta = dot(lightDir, normalize(-spotLightDirection));
    
    if (theta < spotLightOuterCutOff)
    {
        float ambientStrength = 0.05 * spotLightIntensity;
        return ambientStrength * spotLightColor * color;
    }
    
    float epsilon = spotLightCutOff - spotLightOuterCutOff;
    float intensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);
    
    // Ambient
    float ambientStrength = 0.2 * spotLightIntensity;
    vec3 ambient = ambientStrength * spotLightColor;
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = spotLightIntensity * diff * spotLightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spotLightIntensity * spec * spotLightColor;
    
    return intensity * (ambient + diffuse + specular) * color;
}

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    
    vec3 ambientSkybox = texture(skybox, norm).rgb;
    vec3 ambient = 0.3 * ambientSkybox;
    
    vec3 result = ambient;
    vec4 texColor = texture(diffuseTexture, fragTexCoord);
    
    if (dirLightEnabled) {
        result += calculateDirectionalLight(norm, viewDir, vec3(1.0, 1.0, 1.0));
    }
    
    if (spotLightEnabled) {
        result += calculateSpotLight(norm, viewDir, vec3(1.0, 1.0, 1.0));
    }
    
    FragColor = vec4(result * texColor.rgb, texColor.a);
}