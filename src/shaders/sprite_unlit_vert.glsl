#version 430 compatibility

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 texCoord;

layout(location = 1) uniform mat4 _projectionMatrix;
layout(location = 0) uniform mat4 _modelViewMatrix;

uniform vec4 _clipRect;
uniform vec2 _offset;
uniform vec2 _flip;

out vec2 texCoord_v;

void main()
{
    //process texture coordinates
    vec2 flippedTexCoord = vec2(abs(_flip.x - texCoord.x), abs(_flip.y - texCoord.y));
    vec2 finalTexCoord = flippedTexCoord * _clipRect.zw;
    finalTexCoord += _clipRect.xy;
    texCoord_v = finalTexCoord;

    //process vertex position
    vec4 processedPos = vec4((vertexPos - vec3(_offset, 0.0)), 1.0);
    gl_Position = _projectionMatrix * _modelViewMatrix * processedPos;
}