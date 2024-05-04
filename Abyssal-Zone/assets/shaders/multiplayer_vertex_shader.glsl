#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float crouching;

out vec2 TexCoord;

uniform float xOffset;
uniform float yOffset;

void main(){
	vec4 shiftedPosition = vec4(aPos.x + xOffset, aPos.y + yOffset, 1.0, 1.0);
	gl_Position = shiftedPosition;
	vec2 texCoord = aTexCoord;
	if (crouching == 1.0) {
		texCoord = vec2(aTexCoord.x, aTexCoord.y + 0.5);
	}
	TexCoord = texCoord;
}