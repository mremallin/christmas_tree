#version 150

in vec3 a_Vertex;

void main(void) {
	gl_Position = vec4(a_Vertex, 1.0);
	gl_PointSize = 10.0;
}
