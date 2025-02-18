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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fs_position;
layout(location = 1) out vec3 fs_normal;
layout(location = 2) out int instance;
layout(location = 3) out int triangle;
layout(location = 4) out float increaseMap;

void main()
{
	fs_position = vec3(object.model_matrix * vec4(position + hexPos[gl_InstanceID], 1.0));
	fs_normal = transpose(inverse(mat3(object.model_matrix))) * normal;

    increaseMap = 1; //noise(hexPos[gl_InstanceID].x + position.x, hexPos[gl_InstanceID].z + position.z);

    gl_Position = camera.projection * camera.view * object.model_matrix * vec4(position + hexPos[gl_InstanceID], 1.0);
	instance = gl_InstanceID;
	triangle = gl_VertexID / 3;
	if(gl_VertexID < 18){
		triangle = (triangle + hexData[instance].data1.x) % 6;
	}
}
