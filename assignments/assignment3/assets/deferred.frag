#version 450 core
out vec4 FragColor; 
in vec2 UV; //From fsTriangle.vert
//in vec4 LightSpacePos;

//All your material and lighting uniforms go here!
#define MAX_POINT_LIGHTS 64


//uniform sampler2D _MainTex;
uniform mat4 _LightViewProjection;
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0); //White light

struct Material{
	float Ka; //ambient coefficent
	float Kd; //diffuce coefficent
	float Ks; //specular coefficent
	float Shininess; //ooooh shiny
};

struct PointLight{
	vec3 position;
	float radius;
	vec3 color;
};

uniform Material _Material;
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

//layout(binding = i) can be used as an alternative to shader.setInt()
//Each sampler will always be bound to a specific texture unit
uniform layout(binding = 0) sampler2D _gPositions;
uniform layout(binding = 1) sampler2D _gNormals;
uniform layout(binding = 2) sampler2D _gAlbedo;
uniform layout(binding = 3) sampler2D _ShadowMap;

vec4 LightSpacePos;
vec3 normalLightDir;

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos, float bias)
{
	vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
	sampleCoord = sampleCoord * 0.5f + 0.5f;

	float myDepth = sampleCoord.z;
	float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;

	if(myDepth > 1.0f)
		return 0.0f;

	float totalShadow = 0.0f;

	vec2 texelOffset = 1.0 / textureSize(shadowMap, 0);

	for(int y = -1; y <= 1; y++)
	{
		for(int x = -1; x <= 1; x++)
		{
			float pcfDepth = texture(shadowMap, sampleCoord.xy + vec2(x, y) * texelOffset).r;
			totalShadow += step(pcfDepth, myDepth - bias);
		}
	}

	totalShadow /= 9.0f;

	return totalShadow;
}

float attenuateLinear(float distance, float radius){
	return clamp((radius-distance)/radius,0.0,1.0);
}

vec3 calcPointLighting(PointLight light, vec3 normal, vec3 pos)
{
	vec3 lightColor;

	vec3 diff = light.position - pos;

	vec3 toLight = normalize(diff);

	float difuseFactor = max(dot(normal, toLight),0.0f);
	vec3 diffuse = difuseFactor * _LightColor;

	vec3 toEye = normalize(_EyePos - pos);
	vec3 h = normalize(toLight + toEye);

	float specularFactor = pow(max(dot(normal,h),0.0f), _Material.Shininess);
	vec3 specular = specularFactor * _LightColor;

	lightColor = (diffuse + specular) * light.color;

	float d = length(diff);

	lightColor *= attenuateLinear(d, light.radius);

	return lightColor;
}

vec3 calculateLighting(vec3 normal, vec3 pos, vec3 albedo)
{
	vec3 toLight = -normalLightDir;

	vec3 ambient = _Material.Ka * _LightColor;

	float difuseFactor = max(dot(normal, toLight),0.0f);
	vec3 diffuse = difuseFactor * _LightColor;

	vec3 toEye = normalize(_EyePos - pos);
	vec3 h = normalize(toLight + toEye);

	float specularFactor = pow(max(dot(normal,h),0.0f), _Material.Shininess);
	vec3 specular = specularFactor * _LightColor;

	float bias = max(0.05 * (1.0 - dot(normal, normalLightDir)), 0.005);
	float shadow = calcShadow(_ShadowMap, LightSpacePos, bias);

	vec3 lighting = ambient + (diffuse + specular) * (1.0f - shadow);

	return lighting;
}


void main(){
	//Sample surface properties for this screen pixel
	normalLightDir = normalize(_LightDirection);

	vec3 normal = texture(_gNormals,UV).xyz;
	vec3 worldPos = texture(_gPositions,UV).xyz;
	vec3 albedo = texture(_gAlbedo,UV).xyz;
	LightSpacePos = _LightViewProjection * vec4(worldPos, 1.0f);
	//Worldspace lighting calculations, same as in forward shading

	vec3 totalLight = vec3(0);

	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		totalLight += calcPointLighting(_PointLights[i], normal, worldPos);
	}

	FragColor = vec4(albedo * totalLight,1.0);
}
