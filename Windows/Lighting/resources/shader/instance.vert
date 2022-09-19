#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform vec2 transforms[100];

void main()
{
	vec2 pos = aPos + transforms[gl_InstanceID];
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	Color = aColor;
}