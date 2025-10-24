#version 330 core

in vec3 FragColor;
out vec4 color;

uniform vec3 objectColor;

void main()
{
    color = vec4(objectColor, 1.0);
}