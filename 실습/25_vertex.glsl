#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

out vec3 outPos;
out vec3 outColor;
out vec3 outNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    outPos = vec3(model * vec4(aPos, 1.0));
    outColor = aColor;
    outNormal = vec3(model * vec4(aNormal, 1.0));
}