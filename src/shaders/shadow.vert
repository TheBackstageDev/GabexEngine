#version 450

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;      // Model matrix to transform the object
    //mat4 lightViewProj;   
} pushConstants;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;

struct Light {
  vec4 position; //w is type; 0 - Point, 1 - Spot
  vec4 color; // W is itensity
  vec4 direction; //SpotLight direction, W is cutoffAngle
  mat4 lightSpaceMatrix[6]; 
};

struct Material
{
  vec4 color;
  vec3 data; //x is metallic, y is roughness, z is id;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 sunLight;
  mat4 sunLightSpaceMatrix;
  vec4 ambientLightColor; // w is intensity
  Light lights[20];
  Material materials[100];
  int numLights;
} ubo;

void main() {
    vec4 worldPosition = pushConstants.modelMatrix * vec4(position, 1.0);
    
    gl_Position = ubo.sunLightSpaceMatrix * worldPosition;
}