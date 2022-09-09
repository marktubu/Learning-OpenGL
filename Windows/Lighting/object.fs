#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light 
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 viewPos;

uniform Material mat;
uniform Light light;

void main()
{
	vec3 diffuse = vec3(texture(mat.diffuse, TexCoord));
	vec3 specular = vec3(1.0) - vec3(texture(mat.specular, TexCoord));
	vec3 ambient = light.ambient * diffuse;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0);
	diffuse = diff * diffuse * light.diffuse;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	specular = spec * specular * light.specular;
	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}