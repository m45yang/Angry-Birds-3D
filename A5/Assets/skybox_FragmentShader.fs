#version 330

out vec4 fragColor;
in vec3 tex_coords;
uniform samplerCube skybox;

void main()
{
    fragColor = texture(skybox, tex_coords);
}
