#ifndef __SHADER_H__
#define __SHADER_H__

GLuint get_vertex_attribute(void);

GLuint
get_vertex_uniform_projection(void);
GLuint
get_vertex_uniform_modelview(void);

void
initialize_shaders(void);

void
deinit_shaders(void);

#endif /* __SHADER_H__ */
