#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float blockID;

out vec2 TexCoord;

uniform float offsetX;
uniform float offsetY;

void main(){
	vec4 shiftedPosition = vec4(aPos.x + offsetX, aPos.y + offsetY, 1.0, 1.0);
	gl_Position = shiftedPosition;
	TexCoord = aTexCoord;
}