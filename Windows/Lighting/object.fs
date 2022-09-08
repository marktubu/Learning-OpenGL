#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light 
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform Material mat;
uniform Light light;

uniform sampler2D texture1;

void main()
{
	vec3 ambient = light.ambient * mat.ambient;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0);
	vec3 diffuse = diff * mat.diffuse * light.diffuse;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
	vec3 specular = spec * mat.specular * light.specular;
	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}