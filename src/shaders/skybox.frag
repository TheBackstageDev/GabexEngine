#version 450

layout(location = 0) in vec2 fragUv;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube skyboxSampler;

struct PointLight {
  vec4 position; // ignore W
  vec4 color; // W is itensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 inverseView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

void main()
{
  outColor = vec4(texture(skyboxSampler, fragUv));
}