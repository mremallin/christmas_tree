#version 150

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec4 a_Vertex;
in vec4 a_Color;
out vec4 frag_Color;

void main(void) {
	gl_Position = projection_matrix * modelview_matrix * a_Vertex;
	gl_PointSize = 1.5f;
	frag_Color = a_Color;
}
