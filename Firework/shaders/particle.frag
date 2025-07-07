#version 460 core

in float alpha;

out vec4 FragColor;

void main() {
    FragColor = vec4(0.1, 0.2, 0.9, alpha);
    if (alpha > 2.0)
    {
        FragColor = vec4(0.9, 0.0, 0.0, 1.0);
    }
}