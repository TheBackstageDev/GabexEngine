#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 fragUvw;

struct Light {
  vec4 position; //w is type; 0 - Point, 1 - Spot
  vec4 color; // W is itensity
  vec2 angles; // x - internal angles, y - external angles
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 sunLight;
  mat4 sunLightSpaceMatrix;
  vec4 ambientLightColor; // w is intensity
  Light lights[20];
  int numLights;
} ubo;


layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main()
{
    mat4 viewNoTranslation = mat4(mat3(ubo.view));
    
    fragUvw = vec3(position.x, position.y * -1, position.z);
    gl_Position = ubo.projection * viewNoTranslation * push.modelMatrix * vec4(position.xyz, 1.0);
}