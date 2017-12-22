#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model_matrix;

layout (location = 0) in vec3 a_vertex;
out vec4 vertex;  // add this variable for the fragment shader

void main(void) {

    vertex = vec4(a_vertex, 1.0);

	gl_Position = projection * view * model_matrix * vec4(a_vertex, 1.0);

}
