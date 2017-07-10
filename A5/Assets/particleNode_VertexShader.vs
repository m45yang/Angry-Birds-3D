#version 330

out vec3 tex_coords;

in vec3 position;

uniform mat4 Perspective;
uniform mat4 View;

void main()
{
    tex_coords = position.xy;
    vec4 pos = Perspective * View * vec4(position, 1.0);
    gl_Position = pos.xyzw;
}
