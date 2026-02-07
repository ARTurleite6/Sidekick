#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 mvp;

out vec4 vertexColor;

void main()
{
    gl_Position = mvp * vec4(inPosition, 1.0);

    vertexColor = inColor;
}
