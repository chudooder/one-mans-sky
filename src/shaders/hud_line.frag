R"zzz(
#version 330 core

uniform vec4 frag_color;
uniform vec4 clip_area;
in vec4 screen_coords;
out vec4 fragment_color;

void main(){
	vec2 clip_min = vec2((clip_area.x - 400.0) / 400.0, (clip_area.y - 300.0) / 300.0);
	vec2 clip_max = vec2((clip_area.z - 400.0) / 400.0, (clip_area.w - 300.0) / 300.0);
	if(length(clip_area) > 0.0 && 
		(screen_coords.x < clip_min.x || screen_coords.y < clip_min.y ||
	   	 screen_coords.x > clip_max.x || screen_coords.y > clip_max.y) 
	){
		discard;
	}

	fragment_color = frag_color;
}
)zzz"