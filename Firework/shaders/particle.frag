#version 460 core

in vec4 colour;
in vec2 vertexUV;

out vec4 FragColor;

void main() {
    float dist = length(vertexUV);
    if (dist > 1.0) discard;

    float lighting = sqrt(1.0 - dist * dist);

    FragColor = vec4(colour.rgb * lighting, colour.a);
}

