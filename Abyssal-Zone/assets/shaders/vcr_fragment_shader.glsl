#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
    vec4 color = texture(screenTexture, TexCoord);
    FragColor = vec4(color.x, color.y, color.z, color.w * 0.1);
}
