#version 460 core

layout (location = 0) in vec2 vertex;

out vec4 colour;
out vec2 vertexUV;

layout(std430, binding = 0) buffer PositionsSSBO {
    vec4 positions[];
};

layout(std430, binding = 14) buffer ColoursSSBO {
    vec4 colours[];
};

uniform mat4 view;
uniform mat4 projection;
uniform float particleSphereSize;

void main() {

    vec3 particlePos = positions[gl_InstanceID].xyz;

    vec2 offset = vertex * particleSphereSize;
    vec4 billboardPos = vec4(particlePos, 1.0);
    vec4 viewPos = view * billboardPos;
    viewPos.xy += offset;
    gl_Position = projection * viewPos;

    vertexUV = vertex;

    colour = colours[gl_InstanceID];
}



