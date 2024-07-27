#version 460 core

in float alpha;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D particleTexture;

void main() {
    vec4 texColor = texture(particleTexture, TexCoord);
    vec4 finalColor = texColor;
    finalColor.a = alpha;
    FragColor = finalColor;
}