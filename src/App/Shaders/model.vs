#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 normalMatrix;

uniform float morphFactor;
uniform float morphToSphere;
uniform float sphereRadius;
uniform vec3 morphCenter;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

vec3 morphToSpherePosition(vec3 position, float factor)
{
    vec3 worldPos = vec3(model * vec4(position, 1.0));
    
    if (morphToSphere > 0.5 && factor > 0.0) {
        vec3 toCenter = worldPos - morphCenter;
        float dist = length(toCenter);
        vec3 dir = toCenter / dist;
        
        float targetDist = mix(dist, sphereRadius, min(factor, 0.99));
        return morphCenter + dir * targetDist;
    }
    
    return worldPos;
}

void main() {
    vec3 morphedWorldPos = morphToSpherePosition(pos, morphFactor);
    vec3 morphedLocalPos = vec3(inverse(model) * vec4(morphedWorldPos, 1.0));
    
    fragPos = morphedWorldPos;
    fragNormal = transpose(inverse(mat3(model))) * normal;
    fragTexCoord = texCoord;

    gl_Position = mvp * vec4(morphedLocalPos, 1.0);
}