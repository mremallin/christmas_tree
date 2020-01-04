/*
 * Christmas Tree - Brought to you by SDL2 and OpenGL
 * Inspired by: https://github.com/anvaka/atree#
 *
 * Mike Mallin, 2019 - 2020
 */

#ifndef __SPIRAL_H__
#define __SPIRAL_H__

#include <stdint.h>
#include <cglm/cglm.h>

/** Opaque handle to a spiral */
typedef void *spiral;

typedef struct spiral_init_ctx_ {
	/** The number of points making up a given spiral. */
	int num_slices;
	/** Number of rotations to complete before intersecting with y_max */
	int num_rotations;
	/** How long for a given point to travel from 0 -> y_max */
	int cycle_time_ms;
	/** OpenGL coordinate for y_max */
	float y_max;
	/** Slope of the branches */
	float slope;
	/** Offset in radians around the y-axis */
	float starting_angle_offset;
	/** {r,g,b,a} of the given spiral */
	vec4 color;
} spiral_init_ctx;

/** Initializes a spiral */
spiral spiral_init(spiral_init_ctx *);

/** Frees resources held by a spiral */
void spiral_free(spiral);

/** Updates a given spiral based on the amount of time (ms) elapsed */
void spiral_update(spiral, uint32_t delta_ms);

/** Draws the spiral using the active shader */
void spiral_render(spiral);

#endif /* __SPIRAL_H__ */
