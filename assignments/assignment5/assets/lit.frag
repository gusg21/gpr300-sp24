#version 450
out vec4 FragColor; //The color of this fragment
in Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
	mat3 TBN;
}fs_in;

in vec4 LightSpacePos;

uniform sampler2D _MainTex; 
//uniform sampler2D _NormalMapTex; 
uniform sampler2D _ShadowMap; 
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46) * 2.0f;

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;
uniform float _Time;

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos) {
	//Homogeneous Clip space to NDC [-w,w] to [-1,1]
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
    //Convert from [-1,1] to [0,1]
    sampleCoord = sampleCoord * 0.5 + 0.5;
	
	float minBias = 0.005; //Example values! 
	float maxBias = 0.015;
	float bias = max(maxBias * (1.0 - dot(fs_in.WorldNormal,fs_in.TBN * -_LightDirection)),minBias);
	float myDepth = sampleCoord.z - bias;

	float totalShadow = 0;
	vec2 texelOffset = 1.0 / textureSize(_ShadowMap, 0);
	for (int y = -2; y <= 2; y++){
		for (int x = -2; x <= 2; x++) {
			vec2 uv = sampleCoord.xy + vec2(x * texelOffset.x, y * texelOffset.y);
			totalShadow+=step(texture(_ShadowMap,uv).r,myDepth);
		}
	}
	totalShadow/=9.0;


	float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
	//step(a,b) returns 1.0 if a >= b, 0.0 otherwise
	return totalShadow;
}

void main(){
	//Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = fs_in.WorldNormal;
	//Light pointing straight down
	vec3 toLight = fs_in.TBN * -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight),0.0);
	//Calculate specularly reflected light
	vec3 toEye = fs_in.TBN * normalize(_EyePos - fs_in.WorldPos);
	//Blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	// Calc shadow
	float shadow = calcShadow(_ShadowMap, LightSpacePos);
	//Combination of specular and diffuse reflection
	vec3 lightColor = _AmbientColor * _Material.Ka * (1.0 - shadow) + ((_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * (1.0 - shadow));

	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;
	FragColor = vec4(objectColor * lightColor,1.0);
}
