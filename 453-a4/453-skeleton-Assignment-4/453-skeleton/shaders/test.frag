#version 330 core

in vec2 UV;

uniform sampler2D sampler;

out vec4 color;

void main() {
	vec3 materialColor = texture(sampler, UV).rgb;
	color = vec4(materialColor, 1.0);
}
