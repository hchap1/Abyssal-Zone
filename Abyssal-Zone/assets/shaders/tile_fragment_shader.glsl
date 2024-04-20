#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D blockTexture;

void main(){
	FragColor = vec4(1.0, 0.0, 0.0, 1.0); //texture(blockTexture, TexCoord);
}