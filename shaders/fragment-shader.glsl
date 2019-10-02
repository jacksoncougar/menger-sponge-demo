#version 330 core
uniform vec4 uniform_light_position = {1,1,1,1};
uniform vec4 uniform_light_color = {1,0,0,1};

in vec4 vcolor;
in vec4 vnormal;
in vec4 fragment_position;
out vec4 color;

void main() {
    vec3 light_direction = normalize(uniform_light_position.xyz - fragment_position.xyz);
    float power = max(dot(vnormal.xyz, light_direction), 0.2);

    color = power * uniform_light_color + power * color;
}