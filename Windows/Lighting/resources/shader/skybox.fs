#version 330 core
out vec4 FragColor;

in vec3 TexCoord;

// texture sampler
uniform samplerCube texture1;

void main()
{
	vec4 color = texture(texture1, TexCoord);
	FragColor = color;
}