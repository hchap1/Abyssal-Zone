#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 LightPos;

out vec2 TexCoord;
out float brightness;

uniform float xOffset;
uniform float yOffset;
uniform float screenX;
uniform float screenY;
uniform float lightConstant;

void main(){
	vec4 shiftedPosition = vec4(aPos.x + xOffset, aPos.y + yOffset, 1.0, 1.0);
	gl_Position = shiftedPosition;
	TexCoord = aTexCoord;
	float lx = LightPos.x + xOffset;
	float ly = LightPos.y + yOffset;
	float dlx = 1-clamp(abs((lx-shiftedPosition.x) * screenX) / 1000.0, 0.0, 1.0);
	float dly = 1-clamp(abs((ly-shiftedPosition.y) * screenY) / 1000.0, 0.0, 1.0);
	float dx = 1-clamp(abs((shiftedPosition.x * screenX) / 1000.0), 0.0, 1.0);
	float dy = 1-clamp(abs((shiftedPosition.y * screenY) / 1000.0), 0.0, 1.0);
	float centerBrightness = (pow(sin(dx)*sin(dy), 2) * 1.6);
	float lightBrightness = (pow(sin(dlx)*sin(dly), 2) * 2.9 * lightConstant);
	if (centerBrightness > lightBrightness){ brightness = centerBrightness; }
	else { brightness = lightBrightness; }
	//brightness = 1.0f;
}