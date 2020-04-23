#version 430 compatibility
#define MAX_LIGHTS 16

uniform sampler2D _palette;
uniform sampler2D _indexedColor;
uniform vec4 _color;

in vec2 texCoord_v;

in vec3 vertexPos_v;
in vec3 normal_v;

struct Light
{
    vec4 position;
    vec4 color;
};

layout(std140) uniform LightData
{
    vec3 _ambientColor;
    uint _lightCount;
    Light _lights[MAX_LIGHTS];
};

layout(location = 0) out vec4 fragColor;

vec3 lambert(int index)
{
    vec3 lightDirection = _lights[index].position.xyz - vertexPos_v;

    float dist = length(lightDirection);
    vec3 l = normalize(lightDirection);
    vec3 n = normal_v;

    float cosTheta = max(dot(n,l), 0.0);
    vec3 lightColor = _lights[index].color.rgb * _lights[index].color.a;
    float attenuation = 10000.0 / (dist*dist);
    vec3 diffuse = lightColor * cosTheta;
    return diffuse;
}

void main()
{
    vec4 colorIndex = texture(_indexedColor, texCoord_v);
    vec4 baseColor = texture(_palette, colorIndex.xy);
    vec4 tintedColor = baseColor * _color;

    vec3 diffuseSum = vec3(0.0);

    for (int i = 0; i < _lightCount; i++)
    {
        diffuseSum += lambert(i);
    }

    vec3 intensity = _ambientColor + diffuseSum;
    vec4 finalColor = vec4(tintedColor.rgb * intensity, tintedColor.a);

    fragColor = finalColor;
}