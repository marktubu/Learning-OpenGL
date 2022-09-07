#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform sampler2D texture1;

void main()
{
	vec3 color = texture(texture1, TexCoord).xyz;
	FragColor = vec4(color * objectColor * lightColor, 1.0);
}