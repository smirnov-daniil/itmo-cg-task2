#version 330 core

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D diffuseTexture;
uniform samplerCube skybox;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(fragNormal);

    vec3 ambientSkybox = texture(skybox, norm).rgb;
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * mix(lightColor, ambientSkybox, 0.5);

    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec4 texColor = texture(diffuseTexture, fragTexCoord);

    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}
