#version 450

layout(binding = 0, std140) uniform Camera {
	mat4 projection;
	mat4 view;
	vec3 position;
} camera;

layout(binding = 1, std140) uniform Light {
	vec4 position;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
} light;

layout(binding = 2, std140) uniform Object {
	mat4 model_matrix;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
} object;

layout(binding = 3, std140) uniform TileData{
    vec4 position;
	ivec4 data1;
	ivec4 data2;
    vec4 river1;
    vec4 river2;
    vec4 river3;
    vec4 river4;
} data;

layout(binding = 4, std430) buffer Colors {
    vec4 color_map[];
};

layout(binding = 5, std140) buffer positionMatrix{
	mat4 posMat; 
};

layout(location = 5) uniform int light_count;

layout(location = 0) in vec3 fs_position;
layout(location = 1) in vec3 fs_normal;
layout(location = 2) flat in int triangle;

layout(location = 0) out vec4 final_color;

void main() {

    vec3 pseudoLight = fs_position + vec3(0, 5, 0);

    // return;
    vec3 to_light = pseudoLight - fs_position;
    float d = length(to_light);
    float attenuation = clamp(1.0 / d, 0.0, 1.0);

    vec3 L = normalize(pseudoLight);
    vec3 E = normalize(camera.position - fs_position);
    vec3 N = normalize(fs_normal);
    vec3 H = normalize(L + E);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0001);

    vec3 objColor;

    if (triangle == 0) {
        objColor = color_map[data.data1.z].rgb;
    } else if (triangle == 1) {
        objColor = color_map[data.data1.w].rgb;
    } else if (triangle == 2) {
        objColor = color_map[data.data2.x].rgb;
    } else if (triangle == 3) {
        objColor = color_map[data.data2.y].rgb;
    } else if (triangle == 4) {
        objColor = color_map[data.data2.z].rgb;
    } else if (triangle == 5) {
        objColor = color_map[data.data2.w].rgb;
    } else {
        objColor = color_map[0].rgb;
    }

    vec3 ambient = object.ambient_color.rgb * light.ambient_color.rgb;
    vec3 diffuse = object.diffuse_color.rgb * objColor * light.diffuse_color.rgb;
    vec3 specular = object.specular_color.rgb * light.specular_color.rgb * pow(NdotH, 1024);

    vec3 color = (ambient + NdotL * diffuse.rgb + specular);

    final_color = vec4(color, 1.0);
}
