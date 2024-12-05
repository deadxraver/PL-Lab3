#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "image.h"
#include <stdlib.h>

#define BLANK_IMAGE ( struct image ) { 0, 0, 0 }

enum direction90 {
  RIGHT = 0,
  LEFT
};

enum direction {
  HORIZONTAL = 0,
  VERTICAL
};

struct image create_new_image( uint64_t height, uint64_t width );

struct image none( struct image source );

struct image cw90( struct image source );

struct image ccw90( struct image source );

struct image fliph( struct image source );

struct image flipv( struct image source );

#endif // !TRANSFORM_H
