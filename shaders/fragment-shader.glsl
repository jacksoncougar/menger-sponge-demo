#version 330 core
uniform vec4 uniform_light_position = {1,1,1,1};
uniform vec4 uniform_light_color = {1,0,0,1};
uniform vec4 uniform_camera_position = {1,0,0,0};

in vec4 vcolor;
in vec4 vnormal;
in vec4 fragment_position;
out vec4 color;

void main() {
    vec3 light_direction = normalize(uniform_light_position.xyz - fragment_position.xyz);
    float power = max(dot(vnormal.xyz, light_direction), 0.2);
    
    float specularStrength = 0.5;
    vec3 camemra_direction = normalize(uniform_camera_position.xyz - fragment_position.xyz);
    vec3 reflection = reflect(-light_direction, vnormal.xyz);
    float specular = pow(max(dot(camemra_direction, reflection), 0.0), 256);
    vec3 specular_brightness = specularStrength * specular * uniform_light_color.xyz;  

    color.xyz = vcolor.xyz + (specular_brightness + uniform_light_color.xyz) + (power * uniform_light_color.xyz);
}