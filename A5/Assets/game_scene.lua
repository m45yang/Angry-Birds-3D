-- puppet.lua

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
yellow_green = gr.material({0.8, 1.0, 0.55}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
orange = gr.material({1.0, 0.5, 0.0}, {0.1, 0.1, 0.1}, 10)

-- Pig model
pig = gr.mesh('sphere', 'pig', 0)
pig:set_material(green)

pig_left_ear = gr.mesh('sphere', 'pig_left_ear', 0)
pig:add_child(pig_left_ear)
pig_left_ear:set_material(green)
pig_left_ear:scale(1.25, 1.25, 1.25)
pig_left_ear:scale(0.15, 0.25, 0.05)
pig_left_ear:translate(-0.4, 0.8, 0.4)

pig_right_ear = gr.mesh('sphere', 'pig_right_ear', 0)
pig:add_child(pig_right_ear)
pig_right_ear:set_material(green)
pig_right_ear:scale(1.25, 1.25, 1.25)
pig_right_ear:scale(0.25, 0.15, 0.05)
pig_right_ear:translate(0.5, 0.8, 0.4)

pig_left_eye = gr.mesh('sphere', 'pig_left_eye', 0)
pig:add_child(pig_left_eye)
pig_left_eye:set_material(white)
pig_left_eye:scale(1.25, 1.25, 1.25)
pig_left_eye:scale(0.2, 0.2, 0.2)
pig_left_eye:translate(-0.625, 0.0, 0.7)

pig_left_pupil = gr.mesh('sphere', 'pig_left_pupil', 0)
pig_left_eye:add_child(pig_left_pupil)
pig_left_pupil:set_material(black)
pig_left_pupil:scale(1/0.2, 1/0.2, 1/0.2)
pig_left_pupil:scale(0.05, 0.05, 0.05)
pig_left_pupil:translate(-0.5, 0.0, 0.75)

pig_right_eye = gr.mesh('sphere', 'pig_right_eye', 0)
pig:add_child(pig_right_eye)
pig_right_eye:set_material(white)
pig_right_eye:scale(1.25, 1.25, 1.25)
pig_right_eye:scale(0.2, 0.2, 0.2)
pig_right_eye:translate(0.625, 0.0, 0.7)

pig_right_pupil = gr.mesh('sphere', 'pig_right_pupil', 0)
pig_right_eye:add_child(pig_right_pupil)
pig_right_pupil:set_material(black)
pig_right_pupil:scale(1/0.2, 1/0.2, 1/0.2)
pig_right_pupil:scale(0.05, 0.05, 0.05)
pig_right_pupil:translate(0.5, 0.0, 0.75)

pig_nose = gr.mesh('sphere', 'pig_nose', 0)
pig:add_child(pig_nose)
pig_nose:set_material(yellow_green)
pig_nose:scale(1.25, 1.25, 1.25)
pig_nose:scale(0.3, 0.25, 0.1)
pig_nose:translate(0.0, 0.0, 1.0)

pig_left_nostril = gr.mesh('sphere', 'pig_left_nostril', 0)
pig_nose:add_child(pig_left_nostril)
pig_left_nostril:set_material(black)
pig_left_nostril:scale(1/0.3, 1/0.25, 1/0.1)
pig_left_nostril:scale(0.08, 0.1, 0.07)
pig_left_nostril:translate(-0.3, 0.0, 0.5)

pig_right_nostril = gr.mesh('sphere', 'pig_right_nostril', 0)
pig_nose:add_child(pig_right_nostril)
pig_right_nostril:set_material(black)
pig_right_nostril:scale(1/0.3, 1/0.25, 1/0.1)
pig_right_nostril:scale(0.08, 0.1, 0.07)
pig_right_nostril:translate(0.3, 0.0, 0.5)


-- Red angry bird model
bird = gr.mesh('sphere', 'bird', 0)
bird:set_material(red)

bird_hair_top = gr.mesh('sphere', 'bird_hair_top', 0)
bird:add_child(bird_hair_top)
bird_hair_top:set_material(red)
bird_hair_top:scale(1.25, 1.25, 1.25)
bird_hair_top:scale(0.2, 0.4, 0.2)
bird_hair_top:rotate('x', -20)
bird_hair_top:translate(0.0, 0.9, 0.5)

bird_hair_bottom = gr.mesh('sphere', 'bird_hair_bottom', 0)
bird:add_child(bird_hair_bottom)
bird_hair_bottom:set_material(red)
bird_hair_bottom:scale(1.25, 1.25, 1.25)
bird_hair_bottom:scale(0.2, 0.4, 0.2)
bird_hair_bottom:rotate('x', -40)
bird_hair_bottom:translate(0.0, 0.8, 0.4)

bird_left_eye = gr.mesh('sphere', 'bird_left_eye', 0)
bird:add_child(bird_left_eye)
bird_left_eye:set_material(white)
bird_left_eye:scale(1.25, 1.25, 1.25)
bird_left_eye:scale(0.2, 0.2, 0.2)
bird_left_eye:translate(-0.2, 0.0, 0.9)

bird_left_eyebrow = gr.mesh('cube', 'bird_left_eyebrow', 0)
bird_left_eye:add_child(bird_left_eyebrow)
bird_left_eyebrow:set_material(black)
bird_left_eyebrow:scale(1.25, 1.25, 1.25)
bird_left_eyebrow:scale(0.6, 1.2, 0.6)
bird_left_eyebrow:rotate('z', 70)
bird_left_eyebrow:translate(0.15, 0.8, 0.6)

bird_left_pupil = gr.mesh('sphere', 'bird_left_pupil', 0)
bird_left_eye:add_child(bird_left_pupil)
bird_left_pupil:set_material(black)
bird_left_pupil:scale(1/0.2, 1/0.2, 1/0.2)
bird_left_pupil:scale(0.09, 0.09, 0.05)
bird_left_pupil:translate(0.0, 0.0, 0.8)

bird_right_eye = gr.mesh('sphere', 'bird_right_eye', 0)
bird:add_child(bird_right_eye)
bird_right_eye:set_material(white)
bird_right_eye:scale(1.25, 1.25, 1.25)
bird_right_eye:scale(0.2, 0.2, 0.2)
bird_right_eye:translate(0.2, 0.0, 0.9)

bird_right_eyebrow = gr.mesh('cube', 'bird_right_eyebrow', 0)
bird_right_eye:add_child(bird_right_eyebrow)
bird_right_eyebrow:set_material(black)
bird_right_eyebrow:scale(1.25, 1.25, 1.25)
bird_right_eyebrow:scale(0.6, 1.2, 0.6)
bird_right_eyebrow:rotate('z', -70)
bird_right_eyebrow:translate(-0.15, 0.8, 0.6)

bird_right_pupil = gr.mesh('sphere', 'bird_right_pupil', 0)
bird_right_eye:add_child(bird_right_pupil)
bird_right_pupil:set_material(black)
bird_right_pupil:scale(1/0.2, 1/0.2, 1/0.2)
bird_right_pupil:scale(0.09, 0.09, 0.05)
bird_right_pupil:translate(0.0, 0.0, 0.8)

bird_beak = gr.mesh('cube', 'bird_beak', 0)
bird:add_child(bird_beak)
bird_beak:set_material(orange)
bird_beak:scale(1.25, 1.25, 1.25)
bird_beak:scale(0.2, 0.2, 0.2)
bird_beak:rotate('y', 45)
bird_beak:rotate('x', 65)
bird_beak:translate(0.0, -0.4, 0.85)

bird_belly = gr.mesh('sphere', 'bird_belly', 0)
bird:add_child(bird_belly)
bird_belly:set_material(white)
bird_belly:scale(0.9, 0.9, 0.9)
bird_belly:translate(0.0, -0.14, 0.06)


-- Scene arrangement
rootnode = gr.node('root')

ground_p = gr.physics(
  'ground_p',
  'cube',
  0.0, 0.0, -75.0,
  500.0, 1.0, 500.0,
  2
)
rootnode:add_child(ground_p)
ground_p:set_gravity(0)

ground = gr.mesh('cube', 'ground', 0)
ground_p:add_child(ground)
ground:set_material(yellow_green)


pig1_a = gr.animation(
  'pig1_a',
  'sphere',
  0.0, 0.0, -5.0,
  3.0, 3.0, 3.0,
  1
)
rootnode:add_child(pig1_a)
pig1_a:set_animation_material(green)
pig1_a:keyframe_add(5)
pig1_a:keyframe_translate(-6.0, 3.5, -1.0, 0)
pig1_a:keyframe_translate(6.0, 3.5, -1.0, 1)
pig1_a:keyframe_translate(6.0, 3.5, 5.0, 2)
pig1_a:keyframe_translate(-6.0, 3.5, 5.0, 3)
pig1_a:keyframe_translate(-6.0, 3.5, -1.0, 4)

pig1 = gr.node('pig1')
pig1_a:add_child(pig1)
pig1:add_child(pig)


bird1_p = gr.physics(
  'bird1_p',
  'sphere',
  0.0, 1.5, 0.0,
  1.0, 1.0, 1.0,
  0
)
rootnode:add_child(bird1_p)

bird1 = gr.node('bird1')
bird1_p:add_child(bird1)
bird1:rotate('y', 180)
bird1:add_child(bird)


wall1_p = gr.physics(
  'wall1_p',
  'cube',
  0.0, 20.5, -10.0,
  10.0, 10.0, 10.0,
  0
)
rootnode:add_child(wall1_p)

wall1 = gr.mesh('cube', 'wall1', -1)
wall1_p:add_child(wall1)

return rootnode
