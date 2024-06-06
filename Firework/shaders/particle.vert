#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 world = model * vec4(aPos, 1.0);
    world.xyz += aInstancePos;
    gl_Position = projection * view * world;
}