#ifndef __SHADER_H__
#define __SHADER_H__

/**
 * Returns the index where the shader's vertex attribute is bound
 */
GLuint
get_vertex_attribute(void);

/**
 * Returns the index where the shader's color uniform attribute is bound
 */
GLuint
get_color_uniform_attribute(void);

/**
 * Returns the index where the projection matrix is bound
 */
GLuint
get_vertex_uniform_projection(void);

/**
 * Returns the index where the modelview matrix is bound
 */
GLuint
get_vertex_uniform_modelview(void);

/**
 * Initializes the shader. Loads from disk, compiles, links and uploads to GPU. 
 * Will exit() in case of failure.
 */
void
initialize_shaders(void);

/**
 * De-initializes the shader and frees allocated resources.
 */
void
deinit_shaders(void);

#endif /* __SHADER_H__ */
