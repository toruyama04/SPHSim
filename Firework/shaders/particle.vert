#version 450 core
layout (location = 0) in vec3 aPos;

out vec4 colour;

layout(std430, binding = 0) buffer Positions {
    vec4 positions[];
};

layout(std430, binding = 2) buffer Colours {
    vec4 colours[];
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 quadVertexPosition = aPos + positions[gl_InstanceID].xyz;
    gl_Position = projection * view * model * vec4(quadVertexPosition, 1.0f);
    colour = colours[gl_InstanceID];
}