#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D blockTexture;

void main(){
	vec4 pC = texture(blockTexture, TexCoord);
	FragColor = vec4(pC.x * 0.5f, pC.y * 0.5f, pC.z * 0.5f, pC.w);
}