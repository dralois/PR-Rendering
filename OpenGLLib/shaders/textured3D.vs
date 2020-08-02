#version 330 core

layout ( location = 0 ) in vec3 position;
layout ( location = 1 ) in vec3 normal;
layout ( location = 2 ) in vec2 texCoords;

uniform mat4 matV;
uniform mat4 matP;

out vec2 v2f_texCoords;

void main()
{
    vec4 cs_pos = matV * vec4(position, 1.0f);
    v2f_texCoords = texCoords;
    gl_Position = matP * cs_pos;
}
