#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;
layout(location = 4) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

struct Light {
  vec4 position; //w is type; 0 - Point, 1 - Spot
  vec4 color; // W is itensity
  //vec2 angles; // x - internal angles, y - external angles
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
  vec4 ambientLightColor; // w is intensity
  Light lights[10];
  Material materials[100];
  int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint materialIndex;
} push;

void main() {
  Material material = ubo.materials[push.materialIndex];

  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 cameraPosWorld = ubo.invView[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  float cosAngSunIncidence = max(dot(surfaceNormal, normalize(ubo.sunLight.xyz)), 0);
  diffuseLight += cosAngSunIncidence * ubo.sunLight.w;

    // Light contributions
    for (int i = 0; i < ubo.numLights; i++) {
        Light light = ubo.lights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        // Diffuse contribution
        vec3 diffuse = intensity * cosAngIncidence * (1.0 - material.data.x); 

        // Specular contribution using Blinn-Phong
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = max(dot(surfaceNormal, halfAngle), 0.0);
        blinnTerm = pow(blinnTerm, mix(50.0, 4.0, material.data.y)); // roughness controls sharpness

        vec3 specular = intensity * blinnTerm * mix(0.04, 1.0, material.data.x); 

        diffuseLight += diffuse;
        specularLight += specular;
    }

  vec4 sampledColor = texture(texSampler, fragUv);
  outColor = vec4((diffuseLight + specularLight) * fragColor * material.color.rgb * sampledColor.rgb, 1.0); 
}