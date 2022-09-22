#version 330 core
out vec4 FragColor;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

in vec2 oTexCoord;

uniform Material material;

void main()
{
	vec4 color = texture(material.texture_diffuse1, oTexCoord);
	FragColor = color;
}