#version 460 core

layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aTexCoord;

out float alpha;
// out vec2 texCoord;

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
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 pos = positions[gl_InstanceID].xyz + (right * (aPos.x * 2.0 - 1.0)) + (up * (aPos.y * 2.0 - 1.0));

    gl_Position = projection * view * model * vec4(pos, 1.0f);
    // texCoord = aTexCoord;
    alpha = positions[gl_InstanceID].w / velocity[gl_InstanceID].w;
}