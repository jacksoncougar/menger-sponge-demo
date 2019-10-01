#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in mat4x4 instanceWorld;
uniform mat4x4 projection;
uniform mat4x4 view;
out vec4 vcolor;
void main() {
    vec4 t = instanceWorld * position + vec4(0.5,0.5,0.5,0);
    gl_Position.xyzw = projection * view * instanceWorld * position;
    vcolor = color;
}