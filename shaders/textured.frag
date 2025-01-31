#version 450

layout(binding = 0, std140) uniform Camera {
    mat4 projection;
    mat4 view;
    vec3 position;
}
camera;

struct Light {
	vec4 position;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
};

layout(binding = 1, std430) buffer Lights {
	Light lights[];
};

struct Object {
    mat4 model_matrix;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
};

layout(binding = 2, std430) buffer Objects {
    Object objects[];
};

layout(location = 3) uniform bool has_texture = false;
layout(location = 4) uniform int light_count;

layout(binding = 5) uniform sampler2D albedo_texture;

layout(location = 0) in vec3 fs_position;
layout(location = 1) in vec3 fs_normal;
layout(location = 2) in vec2 fs_texture_coordinate;
layout(location = 3) in flat int instance;

layout(location = 0) out vec4 final_color;

float start = 0.5f;
float end = 15.0f;
vec4 fog_color = vec4(0.3f, 0.3f, 0.25f, 1.0);

void main() {

	vec3 color_sum = vec3(0.0);
	vec3 ambient_sum = vec3(0.0);
	vec3 diffuse_sum = vec3(0.0);
	vec3 specular_sum = vec3(0.0);

    Object object = objects[instance];

	for(int i = 0; i < light_count; i++) {
		Light light = lights[i];

        vec3 to_light = light.position.xyz - fs_position;
        float d = length(to_light);
        if(i != 0 && d > 1) { continue; }
        
        vec3 light_vector = light.position.xyz - fs_position * light.position.w;
        vec3 L = normalize(light_vector);
        vec3 E = normalize(camera.position - fs_position);
        vec3 N = normalize(fs_normal);
        vec3 H = normalize(L + E);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0001);

		vec3 ambient = object.ambient_color.rgb * light.ambient_color.rgb;
		vec3 diffuse = object.diffuse_color.rgb * (has_texture ? texture(albedo_texture, fs_texture_coordinate).rgb : vec3(1.0)) *
                        light.diffuse_color.rgb;
		vec3 specular = object.specular_color.rgb * light.specular_color.rgb * pow(NdotH, 1024);

		vec3 color;
        if (i == 0)
            color = (ambient.rgb + NdotL * diffuse.rgb + specular);
        else
            color = (1 / (5 + d * d * 10)) * (ambient.rgb + NdotL * diffuse.rgb + specular);

		color_sum += color;
        ambient_sum += ambient;
        diffuse_sum = NdotL * diffuse.rgb;
        specular_sum += specular;
	}

    float distance = length(fs_position - camera.position);
    float fog_factor = clamp((end - distance) / (end - start), 0.0, 1.0);
	final_color = vec4(color_sum, 1.0);
    final_color = mix(fog_color, final_color, fog_factor);

}
