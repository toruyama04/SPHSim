#version 460 core

in float alpha;
// in vec2 texCoord;

// uniform sampler2D particleTexture;

out vec4 FragColor;

void main() {
    // vec4 textureColor = texture(particleTexture, texCoord);
    // textureColor.a = alpha;
    //FragColor = textureColor;

    // Premultiplied alpha blending
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}