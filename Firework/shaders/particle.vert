#version 450 core
layout (location = 0) in vec3 aPos;

out vec4 colour;

struct Particle {
    vec4 position;
    vec4 velocity;
    vec4 alpha;
    vec4 regionPoint;
    float lifetime;
    float fadeRate;
    float originX;
    float originY;
};

layout(std430, binding = 0) buffer ParticleBuffer {
	Particle particles[];
};


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	Particle particle = particles[gl_InstanceID];
    vec3 quadVertexPosition = aPos + particle.position.xyz;
    gl_Position = projection * view * model * vec4(quadVertexPosition, 1.0f);
    colour = particle.alpha;
}