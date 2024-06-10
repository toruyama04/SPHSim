#version 450 core

out vec4 colour;

struct Particle {
    vec4 position;
    vec4 velocity;
    vec4 alpha;
    vec4 regionPoints[4];
    float lifetime;
    int region;
    float fadeRate;
    int padding;
};

layout(std430, binding = 0) buffer ParticleBuffer {
	Particle particles[];
};


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	Particle particle = particles[gl_VertexID];
    gl_PointSize = 1.0f;
    gl_Position = projection * view * model * vec4(particle.position.xyz, 1.0f);
    colour = particle.alpha;
}