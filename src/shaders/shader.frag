#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;

layout(location = 0) out vec4 outColor;

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

struct LightInfo {
    vec3 directionToLight;
    float distance;
    vec3 fragNormalWorld;
    vec3 viewDirection;
    Material material;
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

const uint DIFFUSE_TEX = 0;
const uint NORMAL_TEX = 1;

layout(set = 1, binding = 0) uniform sampler2D texSampler[6];
layout(set = 1, binding = 1) uniform sampler2DShadow shadowMap;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint materialIndex;
} push;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xyz);

    // Get current fragment depth in light space
    float currentDepth = projCoords.z;

    // Calculate shadow factor (1.0 means fully lit, 0.0 means fully shadowed)
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
    float shadow = currentDepth > closestDepth + bias ? 0.5 : 1.0; 
    
    return shadow;
}

void computeLighting(LightInfo lightInfo, vec3 intensity, inout vec3 diffuseLight, inout vec3 specularLight, float shadowFactor) 
{
  float cosAngIncidence = max(dot(lightInfo.fragNormalWorld, lightInfo.directionToLight), 0.0);
  vec3 diffuse = shadowFactor * intensity * cosAngIncidence * (1.0 - lightInfo.material.data.x);

  vec3 halfAngle = normalize(lightInfo.directionToLight + lightInfo.viewDirection);
  float blinnTerm = max(dot(lightInfo.fragNormalWorld, halfAngle), 0.0);
  blinnTerm = pow(blinnTerm, mix(50.0, 4.0, lightInfo.material.data.y));

  vec3 specular = shadowFactor * intensity * blinnTerm * mix(0.04, 1.0, lightInfo.material.data.x);

  diffuseLight += diffuse;
  specularLight += specular;
}

void calculateLight(Light currentLight, LightInfo lightInfo, inout vec3 diffuseLight, inout vec3 specularLight, vec4 fragPosLightSpace) {
  vec3 intensity = currentLight.color.xyz * currentLight.color.w;
  
  // Shadow factor (1.0 means fully lit, 0.0 means fully shadowed)
  float shadowFactor = calculateShadow(fragPosLightSpace, lightInfo.fragNormalWorld, lightInfo.directionToLight);
  
  if (currentLight.position.w == 0.0) { // Point light
    intensity /= (lightInfo.distance * lightInfo.distance);
    computeLighting(lightInfo, intensity, diffuseLight, specularLight, shadowFactor);
  } else if (currentLight.position.w == 1.0) { // Spot light
    currentLight.direction.xyz = normalize(currentLight.direction.xyz);
    float cosTheta = dot(lightInfo.directionToLight, currentLight.direction.xyz);

    if (cosTheta > currentLight.direction.w) {
      float smoothFactor = 0.01;
      float spotEffect = smoothstep(currentLight.direction.w, currentLight.direction.w + smoothFactor, cosTheta);
      intensity *= spotEffect / lightInfo.distance;
      computeLighting(lightInfo, intensity, diffuseLight, specularLight, shadowFactor);
    }
  }
}

void main() {
    Material material = ubo.materials[push.materialIndex];

    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    // --- Normal Mapping Setup ---
    vec3 normalMap = texture(texSampler[NORMAL_TEX], fragUv).xyz;

    if (length(normalMap) == 0.0) {
        normalMap = vec3(0.5, 0.5, 1.0);
    }

    normalMap *= 2.0 - 1.0;

    mat3 TBN = mat3(fragTangent, fragBitangent, fragNormalWorld);
    vec3 fragNormalTangent = normalize(mix(fragNormalWorld, TBN * normalMap, 1.0));
    // ---------------------------------

    if (ubo.sunLight.w > 0.01)
    {
        vec3 sunDirection = normalize(ubo.sunLight.xyz);
        
        vec4 fragPosLightSpace = ubo.sunLightSpaceMatrix * vec4(fragPosWorld, 1.0);
        
        float shadowFactor = calculateShadow(fragPosLightSpace, fragNormalWorld, sunDirection);

        float cosAngSunIncidence = max(dot(fragNormalWorld, sunDirection), 0.0);
        diffuseLight += shadowFactor * cosAngSunIncidence * ubo.sunLight.w;

        vec3 sunHalfAngle = normalize(sunDirection + viewDirection);
        float sunBlinnTerm = max(dot(fragNormalWorld, sunHalfAngle), 0.0);
        sunBlinnTerm = pow(sunBlinnTerm, mix(50.0, 4.0, material.data.y)); 

        specularLight += shadowFactor * 0.1 * ubo.sunLight.w * sunBlinnTerm * mix(0.04, 1.0, material.data.x);
    }

    // Light contributions
    for (int i = 0; i < ubo.numLights; i++) {
        Light light = ubo.lights[i];

        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float lightRadius = sqrt(light.color.w / 0.01); 
        float distance = length(directionToLight);

        if (distance <= lightRadius) {
          directionToLight = normalize(directionToLight);

          LightInfo lightInfo;
          lightInfo.directionToLight = directionToLight;
          lightInfo.distance = distance;
          lightInfo.fragNormalWorld = fragNormalWorld;
          lightInfo.viewDirection = viewDirection;
          lightInfo.material = material;

          vec4 fragPosLightSpace = light.lightSpaceMatrix[0] * vec4(fragPosWorld, 1.0);

          calculateLight(light, lightInfo, diffuseLight, specularLight, fragPosLightSpace);
        }
    }

    vec4 sampledColor = texture(texSampler[DIFFUSE_TEX], fragUv);
    vec3 finalColor = (diffuseLight + specularLight) * fragColor * material.color.rgb * sampledColor.rgb;

    // Gamma correction
    outColor = vec4(pow(finalColor, vec3(1.0 / 2.2)), 1.0);
}