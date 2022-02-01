#version 330 core

in vec3 color;
in vec2 uv;
in vec3 normal;

uniform sampler2D texture_sampler;
uniform vec3 light_direction;
uniform float light_intensity;

out vec4 out_color;

void main(){

	vec3 n = normalize(normal);
	vec3 l = -normalize(light_direction);

	float lambertian = max(dot(n, l), 0.05f);

	vec3 view_direction = vec3(0.0f, 0.0f, -1.0f);
	vec3 v = -view_direction;
	vec3 r = reflect(-l, n);

	float alpha = 100.0f;
	float specular = pow(dot(r, v), alpha);
	specular = max(specular, 0.0f);
	vec3 final_color;

	vec3 texture_color = texture(texture_sampler, uv).rgb;
	if( lambertian > 0.05f){
		final_color = texture_color * light_intensity * lambertian + specular;
	}else {
		final_color = texture_color * light_intensity * lambertian;
	}

	out_color = vec4(final_color, 1.0f);
}