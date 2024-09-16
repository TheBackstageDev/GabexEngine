#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUv;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;

struct Light {
  vec4 position; //w is type; 0 - Point, 1 - Spot
  vec4 color; // W is itensity
  vec2 angles; // x - internal angles, y - external angles
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

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    vec3 worldNormal = normalize(normalize(mat3(push.modelMatrix) * normal));

    vec3 worldTangent = normalize(mat3(push.modelMatrix) * tangent);
    vec3 worldBitangent = normalize(cross(worldNormal, worldTangent) * tangent.z);

    fragNormalWorld = worldNormal;
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    fragUv = uv;
    fragTangent = worldTangent;
    fragBitangent = worldBitangent;
}