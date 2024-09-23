#version 450

layout(location = 0) in vec3 fragUvw;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform samplerCube skyboxSampler;

struct Light {
  vec4 position; //w is type; 0 - Point, 1 - Spot
  vec4 color; // W is itensity
  //vec2 angles; // x - internal angles, y - external angles
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 sunLight;
    vec4 ambientLightColor; // w is intensity
    Light lights[10];
    int numLights;
} ubo;

void main()
{
    outColor = texture(skyboxSampler, fragUvw);
    outColor.rgb = pow(outColor.rgb, vec3(0.5)); 
}
