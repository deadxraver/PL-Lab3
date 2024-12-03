#include "transform.h"

static struct image rotate( struct image source, enum direction90 dir ) {
  struct image new_img = { .height = source.width, .width = source.height, .data = malloc( sizeof( struct pixel ) * source.height * source.width ) };
  for ( size_t i = 0; i < source.height; i++ ) {
    for ( size_t j = 0; j < source.width; j++ ) {
      size_t addr = dir ? (new_img.height - 1 - j) * new_img.width + i : (j + 1) * new_img.width - 1 - i;
      new_img.data[addr] = source.data[i * source.width + j];
    }
  }
  free( source.data );
  return new_img;
}

static struct image flip( struct image source, enum direction dir ) {
  for ( size_t i = 0; i < source.height - source.height / 2 * dir; i++ ) {
    for ( size_t j = 0; j < source.width - source.width / 2 * (1 - dir); j++ ) {
      struct pixel tmp = source.data[i * source.width + j];
      size_t addr = dir ? (source.height - 1 - i) * source.width + j : (i + 1) * source.width - 1 - j;
      source.data[i * source.width + j] = source.data[addr];
      source.data[addr] = tmp;
    }
  }
  return source;
}

struct image none( struct image source ) {
  return source;
}


struct image cw90( struct image source ) {
  return rotate( source, RIGHT );
}

struct image ccw90( struct image source ) {
  return rotate( source, LEFT );
}

struct image flipv( struct image source ) {
  return flip( source, VERTICAL );
}

struct image fliph( struct image source ) {
  return flip( source, HORIZONTAL );
}

