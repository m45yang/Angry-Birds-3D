#version 330

out vec4 fragColor;

in vec3 tex_coords;
in vec4 particle_color;

uniform sampler2D sprite;

void main()
{
    fragColor = (texture(sprite, tex_coords) * particle_color);
}
