#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
	vec2 TexCoord;
} gs_in[];

out vec2 oTexCoord;

uniform float time;

void Explode(vec3 normal){
	float delta = sin(time) + 1.0;
	vec4 offset = vec4(normal * delta, 0.0);
	gl_Position = gl_in[0].gl_Position + offset;
	oTexCoord = gs_in[0].TexCoord;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position + offset;
	oTexCoord = gs_in[1].TexCoord;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position + offset;
	oTexCoord = gs_in[2].TexCoord;
	EmitVertex();

	EndPrimitive();
}

vec3 GetNormal(){
	vec3 a = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

void main()
{
	vec3 normal = GetNormal();
	Explode(normal);
}