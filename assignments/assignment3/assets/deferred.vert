#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Vertex position in model space
layout(location = 1) in vec3 vNormal; //Vertex position in model space
layout(location = 2) in vec2 vTextCoord; //Vertex texture coordinate (UV)

uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix
uniform mat4 _LightViewProj;

out vec4 LightSpacePos;
out vec2 UV;

out Surface
{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

void main()
{
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));
	// Transform vertex normal to world space using normal matrix
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTextCoord;
	//Transform vertex position to homogeneous clip space
	LightSpacePos = _LightViewProj * _Model * vec4(vPos, 1);

	float u = (((uint(gl_VertexID)+2u) / 3u) % 2u);
	float v = (((uint(gl_VertexID)+1u) / 3u) % 2u);
	UV = vec2(u,v);
	//gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
	gl_Position = vec4(-1.0+u*2.0,-1.0+v*2.0,0.0,1.0);

}