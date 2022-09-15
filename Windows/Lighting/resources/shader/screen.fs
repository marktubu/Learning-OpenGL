#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	vec4 color = texture(texture1, TexCoord);
	color = vec4(color.r, color.g * 0.3, color.b * 0.3, color.a);
	FragColor = color;
}