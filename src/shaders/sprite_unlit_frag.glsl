#version 430 compatibility

uniform sampler2D _palette;
uniform sampler2D _indexedColor;
uniform vec4 _color;

in vec2 texCoord_v;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 colorIndex = texture(_indexedColor, texCoord_v);
    vec4 baseColor = texture(_palette, colorIndex.xy);
    vec4 tintedColor = baseColor * _color;
    float i = colorIndex.r * 255;
    tintedColor.a = sign(mod(i,16));

    fragColor = tintedColor;
}

