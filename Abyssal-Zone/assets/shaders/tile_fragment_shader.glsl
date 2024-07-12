#version 330 core

in vec2 TexCoord;

in float redBrightness;
in float greenBrightness;
in float blueBrightness;
in float yPos;
in float r;
out vec4 FragColor;
in float vignette;

uniform sampler2D blockTexture;

void main() {
	vec2 tx = TexCoord;
	float distance = abs(yPos - r);
	if (distance < 0.007) {
		tx.x += 0.1;
	}
	vec4 pC = texture(blockTexture, tx);
	FragColor = vec4(pC.x * redBrightness, pC.y * greenBrightness, pC.z * blueBrightness, pC.w) * abs(1 - vignette);
}