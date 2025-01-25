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


layout(binding = 3, std430) buffer HexData{
	HexTile hexData[];
};

layout(binding = 4, std430) buffer HexPos{
	vec3 hexPos[];
};

layout(binding = 5, std430) buffer Colors {
    vec4 color_map[];
};

layout(location = 0) in vec3 fs_position;
layout(location = 1) in vec3 fs_normal;
layout(location = 2) flat in int instance;
layout(location = 3) flat in int triangle;
layout(location = 4) in float increaseMap;

layout(location = 0) out vec4 final_color;



void main() {

    // return;

    vec3 light_vector = light.position.xyz - fs_position * light.position.w;
    vec3 N = normalize(fs_normal);
    vec3 E = normalize(camera.position - fs_position);

    vec3 ambient;

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

    ambient = objColor * light.ambient_color.rgb;

    vec3 diffuse = max(dot(N, E), 0.0) * light.diffuse_color.rgb;

    float specularStrength = 1;
    vec3 reflectDir = reflect(-E, N);  
    float spec = pow(max(dot(E, reflectDir), 0.0), 256);
    vec3 specular = specularStrength * spec * light.specular_color.rgb;

    vec3 color = (diffuse + ambient + specular) * objColor;

    final_color = vec4(color, 1.0);
}
