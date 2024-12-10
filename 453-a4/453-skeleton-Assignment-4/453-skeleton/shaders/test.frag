#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

uniform sampler2D sampler;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;
uniform bool applyShading; //bool to apply shading or not

out vec4 color;

void main() {
    vec3 materialColor = texture(sampler, UV).rgb; //color from the texture

    if (applyShading) {
        //ambient
        vec3 ambient = ambientColor * materialColor;

        //diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diffuseColor * diff * materialColor;

        //specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularColor * spec;

		//combine all 
        vec3 result = ambient + diffuse + specular;
        color = vec4(result, 1.0);
    } else {
        color = vec4(materialColor, 1.0);
    }
}
