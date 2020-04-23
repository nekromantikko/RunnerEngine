#version 430 compatibility

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;

layout(location = 1) uniform mat4 _projectionMatrix;
layout(location = 0) uniform mat4 _modelViewMatrix;

uniform vec4 _clipRect;
uniform vec2 _offset;
uniform vec2 _flip;

out vec2 texCoord_v;

out vec3 vertexPos_v;
out vec3 normal_v;
out vec3 tangent_v;
out vec3 bitangent_v;

void main()
{
    //process texture coordinates
    vec2 flippedTexCoord = vec2(abs(_flip.x - texCoord.x), abs(_flip.y - texCoord.y));
    vec2 finalTexCoord = flippedTexCoord * _clipRect.zw;
    finalTexCoord += _clipRect.xy;
    texCoord_v = finalTexCoord;

    mat4 normalMatrix = transpose(inverse(_modelViewMatrix));

    vec3 normaln = normalize(normal);
    vec3 tangentn = normalize(tangent.xyz);
	vec3 bitangent = normalize(cross(normal, tangent.xyz) * tangent.w);

    bitangent_v = normalize((normalMatrix * vec4(bitangent, 0.0)).xyz);
	normal_v = normalize((normalMatrix * vec4(normaln, 0.0)).xyz);
	tangent_v = normalize((normalMatrix * vec4(tangentn.xyz, 0.0)).xyz);

    //process vertex position
    vec4 processedPos = vec4((vertexPos - vec3(_offset, 0.0)), 1.0);
    vertexPos_v = (_modelViewMatrix * processedPos).rgb;
    gl_Position = _projectionMatrix * _modelViewMatrix * processedPos;
}