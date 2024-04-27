#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 LightPos;
layout (location = 3) in float LightType;
layout (location = 4) in float BlockID;

out vec2 TexCoord;

out float redBrightness;
out float greenBrightness;
out float blueBrightness;

uniform float xOffset;
uniform float yOffset;
uniform float screenX;
uniform float screenY;
uniform float lightConstant;
uniform float frame;
uniform float texOffset;
uniform float torchLight;

void main(){
	vec4 shiftedPosition = vec4(aPos.x + xOffset, aPos.y + yOffset, 1.0, 1.0);
	gl_Position = shiftedPosition;
	vec2 text = aTexCoord;
	if (BlockID == 7.0) {
		text.y += frame * texOffset;
	}
	TexCoord = text;
	float lx = LightPos.x + xOffset;
	float ly = LightPos.y + yOffset;
	float dlx = 1-clamp(abs((lx-shiftedPosition.x) * screenX) / 1000.0, 0.0, 1.0);
	float dly = 1-clamp(abs((ly-shiftedPosition.y) * screenY) / 1000.0, 0.0, 1.0);
	float dx = 1-clamp(abs((shiftedPosition.x * screenX) / 1000.0), 0.0, 1.0);
	float dy = 1-clamp(abs((shiftedPosition.y * screenY) / 1000.0), 0.0, 1.0);
	float centerBrightness = (pow(sin(dx)*sin(dy), 2) * 1.6);
	float lightBrightness = (pow(sin(dlx)*sin(dly), 2) * 2.9);
	if (LightType == 3.0) { lightBrightness *= lightConstant; }
	
	if (LightType == 3.0){
		redBrightness = lightBrightness;
		greenBrightness = lightBrightness;
		blueBrightness = lightBrightness;
	}
	if (LightType == 7.0){
		redBrightness = lightBrightness * 1.3 * torchLight;
		greenBrightness = lightBrightness * 0.6 * torchLight;
		blueBrightness = lightBrightness * 0.2 * torchLight;
	}
	redBrightness += centerBrightness;
	greenBrightness += centerBrightness;
	blueBrightness += centerBrightness;

}