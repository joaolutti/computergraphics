#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 vertexUV;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;

void main() {
    gl_Position = P * V * M * vec4(pos, 1.0);
    FragPos = vec3(M * vec4(pos, 1.0)); //
    Normal = normalize(mat3(transpose(inverse(M))) * normal); //normal transformation
    UV = vertexUV;
}
