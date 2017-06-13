-- puppet.lua

rootnode = gr.node('root')
-- rootnode:rotate('y', -90.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('sphere', 'torso')
rootnode:add_child(torso)
torso:set_material(white)
torso:scale(0.25, 0.8, 0.25)

shoulders = gr.mesh('sphere', 'shoulders')
torso:add_child(shoulders)
shoulders:scale(1.0/0.25, 1.0/0.8, 1.0/0.25)
shoulders:scale(0.5, 0.1, 0.1)
shoulders:translate(0.0, 0.9, 0.0)
shoulders:set_material(white)

neck = gr.mesh('sphere', 'neck')
shoulders:add_child(neck)
neck:scale(1.0/0.5, 1.0/0.1, 1.0/0.1)
neck:scale(0.09, 0.12, 0.1)
neck:translate(0.0, 1.5, 0.0)
neck:set_material(white)

neck_head = gr.joint('neck_head', {0.0, 0.0, 0.0}, {-30.0, 0.0, 30.0})
neck:add_child(neck_head)

head = gr.mesh('sphere', 'head')
neck_head:add_child(head)
head:scale(1.0/0.09, 1.0/0.12, 1.0/0.1)
head:scale(0.21, 0.21, 0.21)
head:translate(0.0, 2.2, 0.0)
head:set_material(white)

eye = gr.mesh('sphere', 'eye')
head:add_child(eye)
eye:scale(1.0/0.21, 1.0/0.21, 1.0/0.21)
eye:scale(0.05, 0.05, 0.05)
eye:translate(0.0, 0.0, 0.9)
eye:set_material(red)

shoulders_left_up_arm = gr.joint('shoulders_left_up_arm', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
shoulders:add_child(shoulders_left_up_arm)

left_up_arm = gr.mesh('sphere', 'left_up_arm')
shoulders_left_up_arm:add_child(left_up_arm)
left_up_arm:scale(1.0/0.5, 1.0/0.1, 1.0/0.1)
left_up_arm:scale(0.08, 0.4, 0.08)
left_up_arm:translate(-0.8, -3.5, 0.0)
left_up_arm:set_material(white)

left_up_arm_left_low_arm = gr.joint('left_up_arm_left_low_arm', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
left_up_arm:add_child(left_up_arm_left_low_arm)

left_low_arm = gr.mesh('sphere', 'left_low_arm')
left_up_arm_left_low_arm:add_child(left_low_arm)
left_low_arm:scale(1.0/0.08, 1.0/0.4, 1.0/0.08)
left_low_arm:scale(0.07, 0.3, 0.07)
left_low_arm:translate(0.0, -1.5, 0.0)
left_low_arm:set_material(white)

left_low_arm_left_hand = gr.joint('left_low_arm_left_hand', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
left_low_arm:add_child(left_low_arm_left_hand)

left_hand = gr.mesh('sphere', 'left_hand')
left_low_arm_left_hand:add_child(left_hand)
left_hand:scale(1.0/0.07, 1.0/0.3, 1.0/0.07)
left_hand:scale(0.08, 0.08, 0.08)
left_hand:translate(0.0, -1.0, 0.0)
left_hand:set_material(white)

shoulders_right_up_arm = gr.joint('shoulders_right_up_arm', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
shoulders:add_child(shoulders_right_up_arm)

right_up_arm = gr.mesh('sphere', 'right_up_arm')
shoulders_right_up_arm:add_child(right_up_arm)
right_up_arm:scale(1.0/0.5, 1.0/0.1, 1.0/0.1)
right_up_arm:scale(0.08, 0.40, 0.08)
right_up_arm:translate(0.8, -3.5, 0.0)
right_up_arm:set_material(white)

right_up_arm_right_low_arm = gr.joint('right_up_arm_right_low_arm', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
right_up_arm:add_child(right_up_arm_right_low_arm)

right_low_arm = gr.mesh('sphere', 'right_low_arm')
right_up_arm_right_low_arm:add_child(right_low_arm)
right_low_arm:scale(1.0/0.08, 1.0/0.4, 1.0/0.08)
right_low_arm:scale(0.07, 0.3, 0.07)
right_low_arm:translate(0.0, -1.5, 0.0)
right_low_arm:set_material(white)

right_low_arm_right_hand = gr.joint('right_low_arm_right_hand', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
right_low_arm:add_child(right_low_arm_right_hand)

right_hand = gr.mesh('sphere', 'right_hand')
right_low_arm_right_hand:add_child(right_hand)
right_hand:scale(1.0/0.07, 1.0/0.3, 1.0/0.07)
right_hand:scale(0.08, 0.08, 0.08)
right_hand:translate(0.0, -1.0, 0.0)
right_hand:set_material(white)

hips = gr.mesh('sphere', 'hips')
torso:add_child(hips)
hips:scale(1.0/0.25, 1.0/0.8, 1.0/0.25)
hips:scale(0.35, 0.1, 0.1)
hips:translate(0.0, -0.95, 0.0)
hips:set_material(white)

hips_left_up_leg = gr.joint('hips_left_up_leg', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
hips:add_child(hips_left_up_leg)

left_up_leg = gr.mesh('sphere', 'left_up_leg')
hips_left_up_leg:add_child(left_up_leg)
left_up_leg:scale(1.0/0.5, 1.0/0.1, 1.0/0.1)
left_up_leg:scale(0.15, 0.4, 0.08)
left_up_leg:translate(-0.8, -3.5, 0.0)
left_up_leg:set_material(white)

left_up_leg_left_low_leg = gr.joint('left_up_leg_left_low_leg', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
left_up_leg:add_child(left_up_leg_left_low_leg)

left_low_leg = gr.mesh('sphere', 'left_low_leg')
left_up_leg_left_low_leg:add_child(left_low_leg)
left_low_leg:scale(1.0/0.15, 1.0/0.4, 1.0/0.08)
left_low_leg:scale(0.1, 0.4, 0.08)
left_low_leg:translate(0.0, -1.6, 0.0)
left_low_leg:set_material(white)

left_low_leg_left_foot = gr.joint('left_low_leg_left_foot', {-180.0, 0.0, 75.0}, {0.0, 50.0, 110.0})
left_low_leg:add_child(left_low_leg_left_foot)

left_foot = gr.mesh('sphere', 'left_foot')
left_low_leg_left_foot:add_child(left_foot)
left_foot:scale(1.0/0.1, 1.0/0.4, 1.0/0.08)
left_foot:scale(0.3, 0.1, 0.08)
left_foot:translate(-1.5, -0.8, 0.0)
left_foot:set_material(white)

hips_right_up_leg = gr.joint('hips_right_up_leg', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
hips:add_child(hips_right_up_leg)

right_up_leg = gr.mesh('sphere', 'left_up_leg')
hips_right_up_leg:add_child(right_up_leg)
right_up_leg:scale(1.0/0.5, 1.0/0.1, 1.0/0.1)
right_up_leg:scale(0.15, 0.4, 0.08)
right_up_leg:translate(0.8, -3.5, 0.0)
right_up_leg:set_material(white)

right_up_leg_right_low_leg = gr.joint('right_up_leg_right_low_leg', {-180.0, 0.0, 75.0}, {-10.0, 0.0, 10.0})
right_up_leg:add_child(right_up_leg_right_low_leg)

right_low_leg = gr.mesh('sphere', 'right_low_leg')
right_up_leg_right_low_leg:add_child(right_low_leg)
right_low_leg:scale(1.0/0.15, 1.0/0.4, 1.0/0.08)
right_low_leg:scale(0.1, 0.4, 0.08)
right_low_leg:translate(0.0, -1.6, 0.0)
right_low_leg:set_material(white)

right_low_leg_right_foot = gr.joint('right_low_leg_right_foot', {-180.0, 0.0, 75.0}, {80.0, 140.0, 190.0})
right_low_leg:add_child(right_low_leg_right_foot)

right_foot = gr.mesh('sphere', 'right_foot')
right_low_leg_right_foot:add_child(right_foot)
right_foot:scale(1.0/0.1, 1.0/0.4, 1.0/0.08)
right_foot:scale(0.3, 0.1, 0.08)
right_foot:translate(-1.5, -0.8, 0.0)
right_foot:set_material(white)


return rootnode
