R"zzz(
#version 330 core
uniform float time;

in vec4 color;
out vec4 fragment_color;
void main(){
	if(color.w == 0){
		discard;
	} else {
		fragment_color = color;
	}
}

)zzz"