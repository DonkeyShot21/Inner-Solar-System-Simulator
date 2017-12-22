
#version 330


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

layout (location = 0) in vec3 a_vertex;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texcoord;

out vec4 vertex; // vertex position in eye space
out vec3 normal; // the eye space normal
out vec2 st;

void main(void)
{
  vertex = model_matrix * vec4(a_vertex, 1.0);
	normal = normalize(normal_matrix * a_normal);


  st = a_texcoord;

	gl_Position = projection * view * vertex;
}
