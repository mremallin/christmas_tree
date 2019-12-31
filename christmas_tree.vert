#version 150

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 a_Vertex;

void main(void) {
	gl_Position = projection_matrix * (modelview_matrix * vec4(a_Vertex, 1.0));
	gl_PointSize = 10.0;
}
