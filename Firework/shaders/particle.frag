#version 460 core

in float alpha;
out vec4 FragColor;

void main() {

    if (alpha < 0.8)
        FragColor = vec4(0.0, 0.0, 1.0, 0.05);
    else
        FragColor = vec4(0.0, 0.0, 1.0, 0.8);

}