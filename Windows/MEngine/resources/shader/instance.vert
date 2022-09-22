#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;

out vec3 Color;

void main()
{
	vec2 pos = aPos + aOffset;
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	Color = aColor;
}