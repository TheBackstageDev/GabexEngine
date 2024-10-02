#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;
layout(location = 4) in mat3 fragTBN; 

layout(location = 0) out vec4 outColor;

struct Light {
  vec4 position;
  vec4 color;
  vec4 direction;
  mat4 lightSpaceMatrix[6]; 
};

struct Material {
  vec4 color; 
  vec3 data; // x is metallic, y is roughness, z is id
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
  vec4 ambientLightColor;
  Light lights[20];
  Material materials[100];
  int numLights;
  float exposure;
  bool renderShadows;
} ubo;

const uint DIFFUSE_TEX = 0;
const uint NORMAL_TEX = 1;
const uint SHADOW_MAP_TEX = 0;

layout(set = 1, binding = 0) uniform sampler2DShadow texSamplerShadow[];
layout(set = 1, binding = 0) uniform sampler2D texSampler[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint materialIndex;
    uint textureIndex[6];
} push;

float shadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal, bool renderShadows) {
    if (!renderShadows)
        return 1.0;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    if (projCoords.z >= 1.0) {
        return 1.0;
    }

    projCoords = projCoords * 0.5 + 0.5;

    float diffuseFactor = dot(normal, normalize(lightDir));
    float bias = mix(0.005, 0.0005, diffuseFactor);

    // PCF kernel size (the larger, the softer)
    float shadow = 0.0;
    int samples = 2;
    float radius = 1.0 / textureSize(texSamplerShadow[SHADOW_MAP_TEX], 0).x; 

    for (int x = -samples; x <= samples; ++x) {
        for (int y = -samples; y <= samples; ++y) {
            vec2 offset = vec2(float(x), float(y)) * radius;
            shadow += texture(texSamplerShadow[SHADOW_MAP_TEX], vec3(projCoords.xy + offset, projCoords.z - bias));
        }
    }

    shadow /= float((samples * 2 + 1) * (samples * 2 + 1));

    return shadow;
}

void computeLighting(LightInfo lightInfo, vec3 intensity, inout vec3 diffuseLight, inout vec3 specularLight, float shadowFactor) {
    float cosAngIncidence = max(dot(lightInfo.fragNormalWorld, lightInfo.directionToLight), 0.0);
    vec3 diffuse = intensity * cosAngIncidence * (1.0 - lightInfo.material.data.x);

    vec3 halfAngle = normalize(lightInfo.directionToLight + lightInfo.viewDirection);
    float blinnTerm = max(dot(lightInfo.fragNormalWorld, halfAngle), 0.0);
    blinnTerm = pow(blinnTerm, mix(50.0, 4.0, lightInfo.material.data.y));

    vec3 specular = intensity * blinnTerm * mix(0.04, 1.0, lightInfo.material.data.x);

    diffuseLight += diffuse * shadowFactor;
    specularLight += specular * shadowFactor; 
}

void calculateLight(Light currentLight, LightInfo lightInfo, inout vec3 diffuseLight, inout vec3 specularLight, vec4 fragPosLightSpace, float shadowfactor) {
    vec3 intensity = currentLight.color.xyz * currentLight.color.w;

    if (currentLight.position.w == 0.0) { // Point light
        intensity /= (lightInfo.distance * lightInfo.distance);
        computeLighting(lightInfo, intensity, diffuseLight, specularLight, shadowfactor);
    } else if (currentLight.position.w == 1.0) { // Spot light
        currentLight.direction.xyz = normalize(currentLight.direction.xyz);
        float cosTheta = dot(lightInfo.directionToLight, currentLight.direction.xyz);

        if (cosTheta > currentLight.direction.w) {
            float smoothFactor = 0.01;
            float spotEffect = smoothstep(currentLight.direction.w, currentLight.direction.w + smoothFactor, cosTheta);
            intensity *= spotEffect / lightInfo.distance;
            computeLighting(lightInfo, intensity, diffuseLight, specularLight, shadowfactor);
        }
    }
}

void main() {
    Material material = ubo.materials[push.materialIndex];

    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 normalMap = texture(texSampler[push.textureIndex[NORMAL_TEX]], fragUv).xyz * 2.0 - 1.0;
    normalMap = normalize(fragTBN * normalMap);

    if (normalMap.z == 0.0)
    {
        normalMap = vec3(0.5, 0.5, 1.0);
    }

    if (ubo.sunLight.w > 0.01) {
        vec3 sunDirection = normalize(ubo.sunLight.xyz);
        vec4 fragPosLightSpace = ubo.sunLightSpaceMatrix * vec4(fragPosWorld, 1.0);
        
        float shadowFactor = shadowCalculation(fragPosLightSpace, sunDirection, normalMap, ubo.renderShadows); 

        float cosAngSunIncidence = max(dot(normalMap, sunDirection), 0.0);
        diffuseLight += cosAngSunIncidence * ubo.sunLight.w * shadowFactor; 

        vec3 sunHalfAngle = normalize(sunDirection + viewDirection);
        float sunBlinnTerm = max(dot(normalMap, sunHalfAngle), 0.0);
        sunBlinnTerm = pow(sunBlinnTerm, mix(50.0, 4.0, material.data.y)); 

        specularLight += ubo.sunLight.w * sunBlinnTerm * mix(0.04, 1.0, material.data.x) * shadowFactor; 
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
          lightInfo.fragNormalWorld = normalMap;
          lightInfo.viewDirection = viewDirection;
          lightInfo.material = material;

          vec4 fragPosLightSpace = light.lightSpaceMatrix[0] * vec4(fragPosWorld, 1.0);
          //float shadowFactor = shadowCalculation(fragPosLightSpace); 
          calculateLight(light, lightInfo, diffuseLight, specularLight, fragPosLightSpace, 1.0);
        }
    }

    vec4 sampledColor = texture(texSampler[push.textureIndex[DIFFUSE_TEX]], fragUv);

    //default if diffuse texture doesn't exist
    if (length(sampledColor) == 0.0) {
      sampledColor.rgb = vec3(0.7, 0.7, 0.7);
    }

    if (sampledColor.a < 0.1)
        discard;

    vec3 finalColor = (diffuseLight + specularLight) * fragColor * material.color.rgb * sampledColor.rgb;

    vec3 mapped = vec3(1.0) - exp(-finalColor * ubo.exposure);

    const float gamma = 2.2;

    outColor = vec4(pow(mapped, vec3(1.0 / gamma)), sampledColor.a);
}
