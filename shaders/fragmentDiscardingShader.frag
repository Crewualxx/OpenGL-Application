#version 410 core

in vec2 fTexCoords;
in vec3 fPosition;

out vec4 fColor;

uniform mat4 model;
uniform mat4 view;
uniform int enableFog;

// textures
uniform sampler2D diffuseTexture;

float computeFog()
{
 float fogDensity = 0.025f;
 float fragmentDistance = length(view * model * vec4(fPosition, 1.0f));
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    // for tree leaves
    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.1)
        discard;
    
	if(enableFog == 0){
		fColor = colorFromTexture;
	}else{
		float fogFactor = computeFog();
		vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		fColor = mix(fogColor, colorFromTexture, fogFactor);
	} 
	
}
