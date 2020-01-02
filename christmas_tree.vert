#version 150

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec4 a_Vertex;

void main(void) {
	gl_Position = projection_matrix * modelview_matrix * a_Vertex;
	gl_PointSize = 3.0f;
}
