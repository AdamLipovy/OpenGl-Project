#version 450

struct HexTile {
    vec4 position;
	ivec4 data1;
	ivec4 data2;
    vec4 river1;
    vec4 river2;
    vec4 river3;
    vec4 river4;
};

layout(binding = 0, std140) uniform Camera {
	mat4 projection;
	mat4 view;
	vec3 position;
} camera;

struct Light {
	vec4 position;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
};

layout(binding = 1, std430) buffer Lights {
	Light lights[];
};

layout(binding = 2, std140) uniform Object {
	mat4 model_matrix;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
} object;


layout(binding = 3, std430) buffer HexData{
	HexTile hexData[];
};

layout(binding = 4, std430) buffer HexPos{
	vec3 hexPos[];
};

layout(binding = 5, std430) buffer Colors {
    vec4 color_map[];
};

layout(location = 6) uniform int light_count;

layout(location = 0) in vec3 fs_position;
layout(location = 1) in vec3 fs_normal;
layout(location = 2) flat in int instance;
layout(location = 3) flat in int triangle;
layout(location = 4) in float increaseMap;

layout(location = 0) out vec4 final_color;

float start = 0.5f;
float end = 15.0f;
vec3 fog_color = vec3(0.7f, 0.7f, 0.75f);
uniform mat4 gWorld;


void main() {

	vec3 color_sum = vec3(0.0);

    // return;
    for(int i = 0; i < light_count; i++) {
        Light light = lights[i];

        vec3 to_light = light.position.xyz - fs_position;
        float d = length(to_light);
        float attenuation = clamp(1.0 / d, 0.0, 1.0);
        
        vec3 light_vector = light.position.xyz - fs_position * light.position.w;
        vec3 L = normalize(light_vector);
        vec3 E = normalize(camera.position - fs_position);
        vec3 N = normalize(fs_normal);
        vec3 H = normalize(L + E);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0001);

        vec3 objColor;

        if (triangle == 0) {
            objColor = color_map[hexData[instance].data1.z].rgb;
        } else if (triangle == 1) {
            objColor = color_map[hexData[instance].data1.w].rgb;
        } else if (triangle == 2) {
            objColor = color_map[hexData[instance].data2.x].rgb;
        } else if (triangle == 3) {
            objColor = color_map[hexData[instance].data2.y].rgb;
        } else if (triangle == 4) {
            objColor = color_map[hexData[instance].data2.z].rgb;
        } else if (triangle == 5) {
            objColor = color_map[hexData[instance].data2.w].rgb;
        } else {
            objColor = color_map[6].rgb;
        }

        vec3 ambient = object.ambient_color.rgb * light.ambient_color.rgb;
        vec3 diffuse = object.diffuse_color.rgb * objColor * light.diffuse_color.rgb;
        vec3 specular = object.specular_color.rgb * light.specular_color.rgb;

        vec3 color = vec3(0.0);
        if (i == 0){
            color = (attenuation / 10.0) * (ambient.rgb + NdotL * diffuse.rgb + pow(NdotH, object.specular_color.w) * specular);
        } else {
            color = (attenuation / 1000.0) * ambient.rgb + NdotL * diffuse.rgb + pow(NdotH, object.specular_color.w) * specular;
        }

        color_sum += color;
    }

    color_sum = pow(color_sum, vec3(1.0 / 2.2)); // gamma correction
    final_color = vec4(color_sum, 1.0);

    float distance = length(fs_position - camera.position);
    float fog_factor = clamp((end - distance) / (end - start), 0.0, 1.0);
    final_color = mix(vec4(fog_color, 1.0), final_color, fog_factor);
}
