/*
 * Christmas Tree - Brought to you by SDL2 and OpenGL
 * Inspired by: https://github.com/anvaka/atree#
 *
 * Mike Mallin, 2019 - 2020
 */

#version 150

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform vec4 a_Color;

in vec4 a_Vertex;
out vec4 frag_Color;

void main(void) {
    gl_Position = projection_matrix * modelview_matrix * a_Vertex;
    gl_PointSize = 1.5f;
    frag_Color = a_Color;
}
