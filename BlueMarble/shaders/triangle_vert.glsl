#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec2 in_uv;

uniform mat4 model_view_projection;
uniform mat4 matrix_normal;

out vec3 color;
out vec2 uv;
out vec3 normal;


void main(){
	normal = vec3(matrix_normal * vec4(in_normal, 0.0f));
	color = in_color;
	uv = in_uv;
	gl_Position = model_view_projection * vec4(in_position, 1.0f);
}