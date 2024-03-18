#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

uniform mat4 _Model; 
uniform mat4 _ViewProjection;
uniform mat4 _LightViewProj; //view + projection of light source camera

out Surface{
	vec3 WorldPos; //Vertex position in world space
	vec2 TexCoord;
	vec3 WorldNormal; //Vertex normal in world space
}vs_out;

out vec4 LightSpacePos; //Sent to fragment shader

void main(){
    LightSpacePos = _LightViewProj * _Model * vec4(vPos,1);

	//Transform vertex position to World Space.
	vs_out.WorldPos = vec3(_Model * vec4(vPos,1.0));
	//Transform vertex normal to world space using Normal Matrix
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;

	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
