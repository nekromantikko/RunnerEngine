#version 430 compatibility
#define MAX_LIGHTS 16
#define TILE_SIZE 32
#define SCREEN_HEIGHT 576

uniform sampler2D _palette;
uniform sampler2D _indexedColor;
uniform sampler2D _normalMap;

uniform vec3 _position;
uniform sampler2D _tileLayout;

in vec2 texCoord_v;

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

vec3 lambert(int index, vec3 pixelCoord, vec3 finalNormal)
{
    vec3 lightDirection = _lights[index].position.xyz - pixelCoord;

    float dist = length(lightDirection);
    vec3 l = normalize(lightDirection);
    vec3 n = finalNormal;

    float cosTheta = max(dot(n,l), 0.0);
    vec3 lightColor = _lights[index].color.rgb * _lights[index].color.a;
    float attenuation = 10000.0 / (dist*dist);
    vec3 diffuse = lightColor * cosTheta * attenuation;
    return diffuse;
}

void main()
{
    ivec2 layerSize = textureSize(_tileLayout, 0);

    vec2 coord = gl_FragCoord.xy;
    coord.y = SCREEN_HEIGHT - coord.y;
    coord -= vec2(_position.x,_position.y);

    vec2 coordInTiles = coord / TILE_SIZE;
    ivec2 roundedCoord = ivec2(floor(coordInTiles.x), floor(coordInTiles.y));
    ivec2 repeatingCoord = ivec2(mod(roundedCoord.x, layerSize.x), mod(roundedCoord.y, layerSize.y));

    vec4 texel = texelFetch(_tileLayout, repeatingCoord, 0);
    vec2 tileTopLeft = texel.xy;
    vec2 relTileSize = vec2(TILE_SIZE) / vec2(textureSize(_indexedColor, 0));

    vec2 tileCoord = coordInTiles - vec2(roundedCoord);
    vec2 finalTileCoord = tileCoord * relTileSize;
    finalTileCoord += tileTopLeft;

    vec4 colorIndex = texture(_indexedColor, finalTileCoord);
    vec4 baseColor = texture(_palette, colorIndex.xy);
    float alpha = texel.z;
    baseColor.a *= alpha;

    vec3 normalColor = texture(_normalMap, finalTileCoord).rgb;
    normalColor = normalize(normalColor * 2.0 - 1.0);

    vec3 pixelCoord = gl_FragCoord.xyz;
    pixelCoord.y = SCREEN_HEIGHT - pixelCoord.y;
    pixelCoord.z = _position.z;

    vec3 diffuseSum = vec3(0.0);

    for (int i = 0; i < _lightCount; i++)
    {
        diffuseSum += lambert(i, pixelCoord, normalColor);
    }

    vec3 intensity = _ambientColor + diffuseSum;
    vec4 finalColor = vec4(baseColor.rgb * intensity, baseColor.a);

    fragColor = finalColor;
}