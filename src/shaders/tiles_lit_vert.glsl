#version 430 compatibility

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16

//vertex attributes
layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;

layout(location = 1) uniform mat4 _projectionMatrix;

uniform vec3 _chunkPos;
uniform uint _indices[CHUNK_WIDTH*CHUNK_HEIGHT];

out vec2 texCoord_v;

out vec3 vertexPos_v;
out vec3 normal_v;
out vec3 tangent_v;
out vec3 bitangent_v;

void main()
{
    //process texture coordinates
    vec2 finalTexCoord = texCoord * (1.0f / 16); // UV 0-1 => one tile
	vec2 tilesetOffset = vec2(_indices[gl_InstanceID] % 16, _indices[gl_InstanceID] / 16);
    finalTexCoord += tilesetOffset;
    texCoord_v = finalTexCoord;

    normal_v = normalize(normal);
	tangent_v = normalize(tangent.xyz);
	bitangent_v = normalize(cross(normal, tangent.xyz) * tangent.w);

    //process vertex position
    vec3 processedPos = vertexPos + _chunkPos;
    vertexPos_v = processedPos;
    gl_Position = _projectionMatrix * vec4(processedPos, 1.0);
}