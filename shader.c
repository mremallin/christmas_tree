/*
 * Christmas Tree - Brought to you by SDL2 and OpenGL
 * Inspired by: https://github.com/anvaka/atree#
 *
 * Mike Mallin, 2019 - 2020
 */

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static char *vertex_shader_source;
static char *fragment_shader_source;

static GLuint vertex_shader;
static GLuint vs_uniform_projection_matrix;
static GLuint vs_uniform_modelview_matrix;
static GLuint vs_in_vertex;
static GLuint vs_uniform_color;

static GLuint fragment_shader;
static GLuint shader_program;

GLuint
get_vertex_attribute (void)
{
    return vs_in_vertex;
}

GLuint
get_color_uniform_attribute (void)
{
    return vs_uniform_color;
}

GLuint
get_vertex_uniform_projection (void)
{
    return vs_uniform_projection_matrix;
}

GLuint
get_vertex_uniform_modelview (void)
{
    return vs_uniform_modelview_matrix;
}

static char *
file_to_buf (const char *file_name)
{
    FILE *f;
    size_t length;
    char *buffer;

    f = fopen(file_name, "rb");
    if (f == NULL) {
        return NULL;
    }

    /* Grab length of the file */
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Allocate new buffer and read */
    buffer = malloc(length + 1);
    assert(buffer);

    fread(buffer, length, 1, f);
    fclose(f);
    buffer[length] = '\0';

    return buffer;
}

static void
load_shaders (void)
{
    vertex_shader_source = file_to_buf("christmas_tree.vert");
    if (vertex_shader_source == NULL) {
        fprintf(stderr, "Failed to load vertex shader\n");
        exit(EXIT_FAILURE);
    }

    fragment_shader_source = file_to_buf("christmas_tree.frag");
    if (fragment_shader_source == NULL) {
        fprintf(stderr, "Failed to load fragment shader\n");
        exit(EXIT_FAILURE);
    }
}

static void
compile_shaders (void)
{
    GLint is_compiled;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_shader_source, 0);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == 0) {
        int log_length;
        char *compile_log;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);

        compile_log = malloc(log_length);
        assert(compile_log);

        glGetShaderInfoLog(vertex_shader, log_length, &log_length, compile_log);

        fprintf(stderr, "Vertex shader compilation failure: %s\n", compile_log);
        free(compile_log);
        exit(EXIT_FAILURE);
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar **)&fragment_shader_source, 0);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == 0) {
        int log_length;
        char *compile_log;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);

        compile_log = malloc(log_length);
        assert(compile_log);

        glGetShaderInfoLog(fragment_shader, log_length, &log_length, compile_log);

        fprintf(stderr, "Fragment shader compilation failure: %s\n", compile_log);
        free(compile_log);
        exit(EXIT_FAILURE); 
    }
}

static void
link_shaders (void)
{
    GLint is_linked;

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &is_linked);

    if (is_linked == 0) {
        int log_length;
        char *compile_log;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);

        compile_log = malloc(log_length);
        assert(compile_log);

        glGetProgramInfoLog(shader_program, log_length, &log_length, compile_log);

        fprintf(stderr, "Shader program link failure: %s\n", compile_log);
        free(compile_log);
        exit(EXIT_FAILURE); 
    }
}

static void
load_shader_to_gpu (void)
{
    /* Load to GPU */
    glUseProgram(shader_program);

    /* Attach inputs to the shader program */
    vs_in_vertex = glGetAttribLocation(shader_program, "a_Vertex");
    vs_uniform_color = glGetUniformLocation(shader_program, "a_Color");
    vs_uniform_modelview_matrix = glGetUniformLocation(shader_program, "modelview_matrix");
    vs_uniform_projection_matrix = glGetUniformLocation(shader_program, "projection_matrix");
}

void
initialize_shaders (void)
{
    load_shaders();
    compile_shaders();
    link_shaders();
    load_shader_to_gpu();
}

void
deinit_shaders (void)
{
    if (shader_program) {
        glUseProgram(0);
        /* No statically bound vertex attributes in use to cleanup */
        glDetachShader(shader_program, vertex_shader);
        glDetachShader(shader_program, fragment_shader);
        glDeleteProgram(shader_program);
    }

    if (fragment_shader != 0) {
        glDeleteShader(fragment_shader);
    }

    if (vertex_shader != 0) {
        glDeleteShader(vertex_shader);
    }

    if (fragment_shader_source) {
        free(fragment_shader_source);
    }

    if (vertex_shader_source) {
        free(vertex_shader_source);
    }
}
