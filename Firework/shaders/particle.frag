#version 460 core

in float alpha;

out vec4 FragColor;


void main() {
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}