#include "spiral.h"

#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengl.h>

#include "shader.h"

typedef struct spiral_ctx_ {
	int num_slices;
	int num_rotations;
	int cycle_time_ms;
	float y_max;
	float slope;

	GLuint vbo_id;
	GLuint vao_id;
	vec4 *verticies;
	vec4 color;
} spiral_ctx;

/* This represents the bounding cone for the tree */
static float
get_r_for_point (spiral_ctx *ctx, float y)
{
	/* ( y - b ) / m = r */
	return (y - 2) / ctx->slope;
}

static float
get_y_speed (spiral_ctx *ctx)
{
	return (ctx->y_max / ctx->cycle_time_ms);
}

static void
spiral_update_point (spiral_ctx *ctx, vec4 point)
{
	float r_clamp = get_r_for_point(ctx, point[1]);

	/* Each one also gets rotated around the trunk.
	 * 3 rotations from start to finish should make a decent tree.
	 * The radius calculated above is the distance from the trunk
	 * the point must be. Need to turn the radius into an (x, z)
	 * coordinate to have it around the tree. Starting from
	 * y=0, want the spiral to also start from x=1 -> cos(3x) to
	 * know the x-component of the vertex.
	 */
	point[0] = r_clamp * cosf((ctx->num_rotations * 2.0f * point[1]));
	point[2] = r_clamp * sinf((ctx->num_rotations * 2.0f * point[1]) + GLM_PI);
}

static void
spiral_init_verticies (spiral_ctx *ctx)
{
	size_t i;

	for (i = 0; i < ctx->num_slices; i++) {
		ctx->verticies[i][1] = (ctx->y_max/ctx->num_slices) * i;
		ctx->verticies[i][3] = 1.0f;

		spiral_update_point(ctx, ctx->verticies[i]);
	}
}

static void
spiral_init_opengl (spiral_ctx *ctx)
{
	glGenBuffers(1, &ctx->vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ctx->verticies[0]) * ctx->num_slices, ctx->verticies, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &ctx->vao_id);
	glBindVertexArray(ctx->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_id);
	glVertexAttribPointer(get_vertex_attribute(),
	                      4,
	                      GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(get_vertex_attribute());
}

spiral
spiral_init(spiral_init_ctx *init)
{
	spiral_ctx *ctx = malloc(sizeof(spiral_ctx));
	assert(ctx);

	ctx->num_slices = init->num_slices;
	ctx->num_rotations = init->num_rotations;
	ctx->cycle_time_ms = init->cycle_time_ms;
	ctx->y_max = init->y_max;
	ctx->slope = init->slope;
	memcpy(ctx->color, init->color, sizeof(ctx->color));

	ctx->verticies = malloc(sizeof(vec4) * ctx->num_slices);
	assert(ctx->verticies);

	spiral_init_verticies(ctx);
	spiral_init_opengl(ctx);

	return (spiral)ctx;
}

/* To render the christmas tree:
 * 1) Slice up the y-axis into discrete steps. Each one has a single point
 *	  in it's plane (or n points eqidistant from each other)
 * 2) Based on which slice this is, that determines how far away (r)
 *		from the y-axis the point is (r = (y-b)/m) to find the bounding cone
 *      of the spiral.
 *    Each slice also is at a different position on the circle from the
 *      ones surrounding it.
 * 3) Each time step, update the points to move along the y-axis. Once
 *      a given point has surpassed a y-max, reset it to y=0 and continue.
 */
void
spiral_update (spiral _ctx, uint32_t delta_ms)
{
	spiral_ctx *ctx = _ctx;
	size_t i;

	for (i = 0; i < ctx->num_slices; i++) {
		ctx->verticies[i][1] += get_y_speed(ctx) * delta_ms;

		if (ctx->verticies[i][1] > ctx->y_max) {
			ctx->verticies[i][1] = 0.0f;
		}

		spiral_update_point(ctx, ctx->verticies[i]);
	}
}

void
spiral_render (spiral _ctx)
{
	spiral_ctx *ctx = _ctx;

	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ctx->verticies[0]) * ctx->num_slices, ctx->verticies, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(get_vertex_attribute());

	glUniform4fv(get_color_uniform_attribute(),
			     1, (GLfloat *)ctx->color);

	glBindVertexArray(ctx->vao_id);
	glDrawArrays(GL_POINTS, 0, ctx->num_slices);
	glDisableVertexAttribArray(get_vertex_attribute());
}

void
spiral_free(spiral _ctx)
{
	spiral_ctx *ctx = _ctx;

	glDeleteVertexArrays(1, &ctx->vao_id);
	glDeleteBuffers(1, &ctx->vbo_id);

	free(ctx->verticies);
}
