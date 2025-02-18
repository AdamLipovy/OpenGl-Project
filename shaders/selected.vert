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

layout(location = 5) uniform int light_count;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fs_position;
layout(location = 1) out vec3 fs_normal;
layout(location = 2) out int triangle;

void main()
{
	fs_position = vec3(object.model_matrix * vec4(position + data.position.xyz, 1.0));
	fs_normal = transpose(inverse(mat3(object.model_matrix))) * normal;

    gl_Position = camera.projection * camera.view * object.model_matrix * vec4(position + data.position.xyz, 1.0);
	triangle = gl_VertexID / 3;
}
