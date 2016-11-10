Chenyang Wang (cw33374)
Shawn Wu (skw927)

Everything works.

There is a file under assets/obj/bone.obj that we load to
render the selection cylinder for the bones.

The vertex shader is used to do most of the computation for
the animated vertex positions. As a result, the animation is extremely 
responsive. To pass data to the GPU, we use a uniform buffer object to 
hold the undeformed and deformed transformation matrices per bone. This 
is currently limited at 200 since GLSL requires us to specify the 
size of these arrays, though this limit can be increased.
