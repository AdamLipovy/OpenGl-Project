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
vec4 fog_color = vec4(0.3f, 0.3f, 0.25f, 1.0);

const int perm[256] = {
    151, 160, 137, 91, 90, 15,
    131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
    190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
    5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
    223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
    251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

int hash(int i) {
    return perm[int(i)];
}

int fastfloor(float fp) {
    int i = int(fp);
    return (fp < i) ? (i - 1) : (i);
}

float grad(int hash, float x, float y) {
    const int h = hash & 0x3F;  // Convert low 3 bits of hash code
    const float u = h < 4 ? x : y;  // into 8 simple gradient directions,
    const float v = h < 4 ? y : x;
    return ((h % 2 == 1) ? -u : u) + ((h % 4 > 2) ? -2.0f * v : 2.0f * v); // and compute the dot product with (x,y).
}

const float F2 = 0.366025403f;  // F2 = (sqrt(3) - 1) / 2
const float G2 = 0.211324865f;  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)

float noise(float x, float y) {
    float n0, n1, n2;   // Noise contributions from the three corners

    // Skewing/Unskewing factors for 2D


    // Skew the input space to determine which simplex cell we're in
    const float s = (x + y) * F2;  // Hairy factor for 2D
    const float xs = x + s;
    const float ys = y + s;
    const int i = fastfloor(xs);
    const int j = fastfloor(ys);

    // Unskew the cell origin back to (x,y) space
    const float t = float(i + j) * G2;
    const float X0 = i - t;
    const float Y0 = j - t;
    const float x0 = x - X0;  // The x,y distances from the cell origin
    const float y0 = y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1;  // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0 > y0) {   // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        i1 = 1;
        j1 = 0;
    } else {   // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        i1 = 0;
        j1 = 1;
    }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6

    const float x1 = x0 - i1 + G2;            // Offsets for middle corner in (x,y) unskewed coords
    const float y1 = y0 - j1 + G2;
    const float x2 = x0 - 1.0f + 2.0f * G2;   // Offsets for last corner in (x,y) unskewed coords
    const float y2 = y0 - 1.0f + 2.0f * G2;

    // Work out the hashed gradient indices of the three simplex corners
    const int gi0 = hash(i + hash(j));
    const int gi1 = hash(i + i1 + hash(j + j1));
    const int gi2 = hash(i + 1 + hash(j + 1));

    // Calculate the contribution from the first corner
    float t0 = 0.5f - x0*x0 - y0*y0;
    if (t0 < 0.0f) {
        n0 = 0.0f;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    // Calculate the contribution from the second corner
    float t1 = 0.5f - x1*x1 - y1*y1;
    if (t1 < 0.0f) {
        n1 = 0.0f;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    // Calculate the contribution from the third corner
    float t2 = 0.5f - x2*x2 - y2*y2;
    if (t2 < 0.0f) {
        n2 = 0.0f;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 45.23065f * (n0 + n1 + n2);
}

void main() {

	vec3 color_sum = vec3(0.0);

    // return;
    for(int i = 0; i < light_count; i++) {
        Light light = lights[i];

        vec3 to_light = light.position.xyz - fs_position;
        float d = length(to_light);
        if(i > 0 && d > 1) { continue; }
        
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
            objColor = color_map[0].rgb;
        }

        vec3 ambient = object.ambient_color.rgb * light.ambient_color.rgb;
        vec3 diffuse = object.diffuse_color.rgb * objColor * light.diffuse_color.rgb;
        vec3 specular = object.specular_color.rgb * light.specular_color.rgb * pow(NdotH, 1024);

        vec3 color = vec3(0.0);
        if (i == 0){
            color = (diffuse.rgb + specular);
        } else {
            color = (1 / (2 + d * d)) * (ambient.rgb + NdotL * diffuse.rgb + specular);
        }

        color_sum += color;
    }

    float distance = length(fs_position - camera.position);
    float fog_factor = clamp((end - distance) / (end - start), 0.0, 1.0);
    final_color = vec4(max(noise(fs_position.x / 2, fs_position.z / 2), 0.7) * color_sum, 1.0);
    final_color = mix(fog_color, final_color, fog_factor);
}
