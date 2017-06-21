mat1 = gr.material({0.0, 1.0, 0.5}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 10)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)
mat5 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.9, 0.8}, 250)

scene_root = gr.node('root')

s1 = gr.nh_sphere('s1', {250, 0, -20}, 40)
scene_root:add_child(s1)
s1:set_material(mat1)

s2 = gr.nh_sphere('s2', {-250, -200, -500}, 200)
scene_root:add_child(s2)
s2:set_material(mat2)

s3 = gr.nh_sphere('s3', {200, -50, -200}, 120)
scene_root:add_child(s3)
s3:set_material(mat3)

s4 = gr.nh_sphere('s4', {-100, 25, -300}, 50)
scene_root:add_child(s4)
s4:set_material(mat4)

s5 = gr.nh_sphere('s5', {280, 100, -250}, 200)
scene_root:add_child(s5)
s5:set_material(mat5)

-- A small stellated dodecahedron.

steldodec = gr.mesh( 'dodec', 'Assets/smstdodeca.obj' )
steldodec:set_material(mat3)
scene_root:add_child(steldodec)

white_light = gr.light({0.0, 0.0, 799.0}, {0.9, 0.9, 0.9}, {1, 0, 0})

gr.render(scene_root, 'sample.png', 500, 500,
    {0, 0, 800}, {0, 0, -1}, {0, 1, 0}, 50,
    {0.3, 0.3, 0.3}, {white_light})
