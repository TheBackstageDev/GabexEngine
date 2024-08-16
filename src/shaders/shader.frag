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
  vec4 direction; //SpotLight direction, W is cutoffAngle
};

struct Material
{
  vec4 color;
  vec3 data; //x is metallic, y is roughness, z is id;
};

struct LightInfo {
    vec3 directionToLight;
    float distance;
    vec3 surfaceNormal;
    vec3 viewDirection;
    Material material;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 sunLight;
  vec4 ambientLightColor; // w is intensity
  Light lights[20];
  Material materials[100];
  int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint materialIndex;
} push;

void calculatePointLight(Light currentLight, LightInfo lightInfo, inout vec3 diffuseLight, inout vec3 specularLight) {
    float attenuation = 1.0 / (lightInfo.distance * lightInfo.distance);

    float cosAngIncidence = max(dot(lightInfo.surfaceNormal, lightInfo.directionToLight), 0);
    vec3 intensity = currentLight.color.xyz * currentLight.color.w * attenuation;

    vec3 diffuse = intensity * cosAngIncidence * (1.0 - lightInfo.material.data.x);

    vec3 halfAngle = normalize(lightInfo.directionToLight + lightInfo.viewDirection);
    float blinnTerm = max(dot(lightInfo.surfaceNormal, halfAngle), 0.0);
    blinnTerm = pow(blinnTerm, mix(50.0, 4.0, lightInfo.material.data.y));

    vec3 specular = intensity * blinnTerm * mix(0.04, 1.0, lightInfo.material.data.x) * attenuation;

    diffuseLight += diffuse;
    specularLight += specular;
}

void calculateSpotLight(Light currentLight, LightInfo lightInfo, inout vec3 diffuseLight, inout vec3 specularLight)
{
    currentLight.direction.xyz = normalize(currentLight.direction.xyz);
    float cosTheta = dot(lightInfo.directionToLight, currentLight.direction.xyz);

    if (cosTheta > currentLight.direction.w) {
        float attenuation = 1.0 / (lightInfo.distance * lightInfo.distance);

        float smoothFactor = 0.01;
        float spotEffect = smoothstep(currentLight.direction.w, currentLight.direction.w + smoothFactor, cosTheta);
        vec3 intensity = currentLight.color.xyz * currentLight.color.w * attenuation * spotEffect;

        float cosAngIncidence = max(dot(lightInfo.surfaceNormal, lightInfo.directionToLight), 0.0);
        vec3 diffuse = intensity * cosAngIncidence * (1.0 - lightInfo.material.data.x);

        vec3 halfAngle = normalize(lightInfo.directionToLight + lightInfo.viewDirection); 
        float blinnTerm = max(dot(lightInfo.surfaceNormal, halfAngle), 0.0);
        blinnTerm = pow(blinnTerm, mix(20.0, 2.0, lightInfo.material.data.y)); 

        vec3 specular = intensity * blinnTerm * mix(0.04, 1.0, lightInfo.material.data.x);

        diffuseLight += diffuse;
        specularLight += specular;
    }
}

void main() {
    Material material = ubo.materials[push.materialIndex];

    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 sunDirection = normalize(ubo.sunLight.xyz);

    float cosAngSunIncidence = max(dot(surfaceNormal, sunDirection), 0);
    diffuseLight += cosAngSunIncidence * ubo.sunLight.w;

      // Sunlight Specular Contribution
    vec3 sunHalfAngle = normalize(sunDirection + viewDirection);
    float sunBlinnTerm = max(dot(surfaceNormal, sunHalfAngle), 0.0);
    sunBlinnTerm = pow(sunBlinnTerm, mix(50.0, 4.0, material.data.y)); 

    specularLight += 0.1 * ubo.sunLight.w * sunBlinnTerm * mix(0.04, 1.0, material.data.x); 
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
            lightInfo.surfaceNormal = surfaceNormal;
            lightInfo.viewDirection = viewDirection;
            lightInfo.material = material;

            if (light.position.w == 0.0)
            {
               calculatePointLight(light, lightInfo, diffuseLight, specularLight);
            } else if (light.position.w == 1.0) {
              calculateSpotLight(light, lightInfo, diffuseLight, specularLight);
            }
        }
    }

  vec4 sampledColor = texture(texSampler, fragUv);
  outColor = vec4((diffuseLight + specularLight) * fragColor * material.color.rgb * sampledColor.rgb, 1.0); 
}