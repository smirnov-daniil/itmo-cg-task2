#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 normalMatrix;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
    fragPos = vec3(model * vec4(pos, 1.0));
    fragNormal = mat3(normalMatrix) * normal;
    fragTexCoord = texCoord;

    gl_Position = mvp * vec4(pos, 1.0);
}
