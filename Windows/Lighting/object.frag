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

struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    vec3 ambient;
	vec3 diffuse;
	vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Light 
{
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform vec3 viewPos;

uniform Material mat;

uniform DirLight dirLight;

#define NUM_POINT_LIGHT 4
uniform PointLight pointLights[NUM_POINT_LIGHT];

uniform Light spotLight;

vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir)
{
    vec3 ambient = light.ambient * texture(mat.diffuse, TexCoord).rgb;
    // diffuse 
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(mat.diffuse, TexCoord).rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    vec3 specular = light.specular * spec * texture(mat.specular, TexCoord).rgb; 
    
    return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight light, vec3 norm, vec3 viewDir)
{
    vec3 ambient = light.ambient * texture(mat.diffuse, TexCoord).rgb;
    
    // diffuse 
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(mat.diffuse, TexCoord).rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    vec3 specular = light.specular * spec * texture(mat.specular, TexCoord).rgb;  
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    ambient  *= attenuation; 
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse + specular;
    return result;
}

vec3 calcSpotLight(Light light, vec3 norm, vec3 viewDir)
{
    vec3 ambient = light.ambient * texture(mat.diffuse, TexCoord).rgb;
    
    // diffuse 
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(mat.diffuse, TexCoord).rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    vec3 specular = light.specular * spec * texture(mat.specular, TexCoord).rgb;  
    
    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerOff);
    float intensity = clamp((theta - light.outerOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    ambient  *= attenuation; 
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse + specular;
    return result;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = calcDirLight(dirLight, norm, viewDir);
    for(int i=0;i<NUM_POINT_LIGHT;i++)
    {
        result += calcPointLight(pointLights[i], norm, viewDir);
    }
    result += calcSpotLight(spotLight, norm, viewDir);
    FragColor = vec4(result, 1.0);
}

