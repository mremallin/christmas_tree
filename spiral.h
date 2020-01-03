#ifndef __SPIRAL_H__
#define __SPIRAL_H__

#include <stdint.h>

typedef void *spiral;

typedef struct spiral_init_ctx_ {
	int num_slices;
	int num_rotations;
	int cycle_time_ms;
	float y_max;
	float slope;
} spiral_init_ctx;

spiral spiral_init(spiral_init_ctx *);

void spiral_free(spiral);

void spiral_update(spiral, uint32_t delta_ms);

void spiral_render(spiral);

#endif /* __SPIRAL_H__ */
