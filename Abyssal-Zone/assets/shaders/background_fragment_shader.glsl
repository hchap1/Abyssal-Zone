#version 330 core

in vec2 TexCoord;
in float brightness;
out vec4 FragColor;

uniform sampler2D blockTexture;

void main(){
	vec4 pC = texture(blockTexture, TexCoord);
	FragColor = vec4(pC.x * brightness, pC.y * brightness, pC.z * brightness, pC.w);
	FragColor = vec4(pC.x * brightness, pC.y * brightness, pC.z * brightness, pC.w);
}