#version 460 core

layout (location = 0) in vec3 aPos;

out vec4 colour;

layout(std430, binding = 0) buffer Positions {
    vec4 positions[];
};

layout(std430, binding = 1) buffer Velocities {
    vec4 velocity[];
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 quadVertexPosition = aPos + positions[gl_InstanceID].xyz;
    gl_Position = projection * view * model * vec4(quadVertexPosition, 1.0f);

    colour = vec4(1.0, 1.0, 1.0, positions[gl_InstanceID].w / velocity[gl_InstanceID].w);
}