#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 vertexUV;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec2 UV;

void main() {
	gl_Position = P * V * M * vec4(pos, 1.0);
	UV = vertexUV;
}
