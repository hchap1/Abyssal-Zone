#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float blockID;

out vec2 TexCoord;
out float brightness;

uniform float xOffset;
uniform float yOffset;
uniform float screenX;
uniform float screenY;

void main(){
	vec4 shiftedPosition = vec4(aPos.x + xOffset, aPos.y + yOffset, 1.0, 1.0);
	gl_Position = shiftedPosition;
	TexCoord = aTexCoord;
	float dx = 1-abs((shiftedPosition.x * screenX) / 1000.0);
	float dy = 1-abs((shiftedPosition.y * screenY) / 1000.0);
	brightness = pow(sin(dx)*sin(dy), 3) * 1.7;
}