#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct Light {
  vec4 position; // ignore W
  vec4 color; // W is itensity
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
  vec4 position;
  vec4 color;
  float radius;
} push;

const float M_PI = 3.1415926535;

void main() {
  float dis = sqrt(dot(fragOffset, fragOffset));
  if (dis >= 1.0) {
    discard;
  }

  float cosDis = 0.5 * (cos(dis * M_PI) + 1.0); 
  outColor = vec4(push.color.xyz + 0.5 * cosDis, cosDis);
}