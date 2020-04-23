#version 430 compatibility

//vertex attributes
layout(location = 0) in vec3 vertexPos;

out vec2 texCoord_v;

void main()
{
	gl_Position = vec4(vertexPos, 1.0);

	texCoord_v = (vertexPos.xy + 1) / 2;
}