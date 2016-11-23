R"zzz(
#version 330 core

uniform vec4 frag_color;
uniform vec4 clip_area;
in vec4 screen_coords;
out vec4 fragment_color;

void main(){
	vec4 n_clip_area = (clip_area * 2 - 1);
	if(length(clip_area) > 0.0 && 
		(screen_coords.x < n_clip_area.x || screen_coords.y < n_clip_area.y ||
	   	 screen_coords.x > n_clip_area.z || screen_coords.y > n_clip_area.w) 
	){
		discard;
	}

	fragment_color = frag_color;
}
)zzz"