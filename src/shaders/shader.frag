#version 450

#extension GL_EXT_nonuniform_qualifier : enable

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
  vec3 data;
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
} ubo;

const uint DIFFUSE_TEX = 0;
const uint NORMAL_TEX = 1;
const uint SHADOW_MAP_TEX = 0;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint materialIndex;
    uint textureIndex[6];
} push;

float shadowCalculation(vec4 fragPosLightSpace, vec3 direction) {
    // Normalize the coordinates to [0, 1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords *= 0.5 + 0.5; 

    float shadow = 0.0;
/* 
    if(projCoords.z > 1.0)
        return shadow; //returns 0 */
    
    float closestDepth = texture(texSampler[SHADOW_MAP_TEX], projCoords.xy).r; 
    float currentDepth = projCoords.z;

    float bias = max(0.05 * (1.0 - dot(fragNormalWorld, normalize(direction))), 0.005);  

    vec2 texelSize = 1.0 / textureSize(texSampler[SHADOW_MAP_TEX], 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(texSampler[SHADOW_MAP_TEX], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
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

    if (ubo.sunLight.w > 0.01) {
        vec3 sunDirection = normalize(ubo.sunLight.xyz);
        vec4 fragPosLightSpace = ubo.sunLightSpaceMatrix * vec4(fragPosWorld, 1.0);
        
        float shadowFactor = shadowCalculation(fragPosLightSpace, sunDirection); 

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

    vec3 finalColor = (diffuseLight + specularLight) * fragColor * material.color.rgb * sampledColor.rgb;

    // Gamma correction
    outColor = vec4(pow(finalColor, vec3(1.0 / 2.2)), sampledColor.a);
}
