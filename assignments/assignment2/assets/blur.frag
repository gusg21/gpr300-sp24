#version 450

out vec4 FragColor;

in vec2 UV;

uniform sampler2D _ColorBuffer;
uniform sampler2D _DepthBuffer;
uniform vec2 _BufferSize;

uniform float _DofIntensity; // 200
uniform float _DofOffset; // 0.55
uniform float _DofBlur; // 20
uniform float _FogPower; // 500

vec4 blur(float directions, float quality, float size) {
    // Blurring from https://www.shadertoy.com/view/Xltfzj
    float Tau = 6.28318530718; // Pi*2

    vec2 Radius = size/_BufferSize;
    
    // Pixel colour
    vec4 Color = texture(_ColorBuffer, UV);
    
    // Blur calculations
    for (float d=0.0; d<Tau; d+=Tau/directions)
    {
		for (float i=1.0/quality; i<=1.0; i+=1.0/quality)
        {
			Color += texture(_ColorBuffer, UV+vec2(cos(d),sin(d))*Radius*i);		
        }
    }

    Color /= quality * directions - 15.0;
    return Color;
}

void main(){
    
    float Directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality = 3.0; // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size = 8.0; // BLUR SIZE (Radius)
   
    
    
    // Output to screen
    float rawDepth = texture(_DepthBuffer, UV).r;
    float depth = clamp(pow(rawDepth, _DofIntensity) - _DofOffset, 0, 1);
    vec4 color = mix(blur(16.0f, 3.0f, depth * _DofBlur), vec4(0, 0, 0, 1), clamp(pow(rawDepth, _FogPower), 0, 1));
    FragColor = color;
}