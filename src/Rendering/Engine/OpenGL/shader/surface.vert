#version 440

layout(std140, binding = 0) uniform Transforms
{
	// transform
	mat4 model;
	mat4 view;
	mat4 proj;
	// illumination
	vec4 ambient;
	vec4 intensity;
	vec4 direction;
	vec4 camera;
	// parameters
	int width;
	int height;
	int index;
} uRenderParams;

layout(location = 0) in vec3 in_vert;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_norm;

// instance transform
layout(location = 3) in vec3 in_translation;
layout(location = 4) in vec3 in_scaling;
layout(location = 5) in mat3 in_rotation; // location 5-7 is used for rotation matrix

out VertexData
{
	vec3 position;
	vec3 normal;
	vec3 color;

	int  instanceID;
} vs_out;

// whether to use vertex normal
uniform bool uVertexNormal = false;
// is instance rendering?
uniform bool uInstanced = false;

void main(void) {
	vs_out.color = in_color;
	vs_out.instanceID = -1;

	vec3 position = in_vert;

	if(uInstanced) {
		// apply instance transform
		vec3 scaled = in_scaling * in_vert;
		vec3 rotated = in_rotation * scaled;
		position = rotated + in_translation;
		
		vs_out.instanceID = gl_InstanceID;
	}

	vec4 worldPos = uRenderParams.model * vec4(position, 1.0);
	vec4 cameraPos = uRenderParams.view * worldPos;

	vs_out.position = cameraPos.xyz;

	if(uVertexNormal)
	{
		mat4 MV = uRenderParams.view * uRenderParams.model;
		mat4 N = transpose(inverse(MV));
		vs_out.normal = (N * vec4(in_norm, 0)).xyz;
	}

	gl_Position = uRenderParams.proj * cameraPos;
}