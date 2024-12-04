#include "bmp.h"
#include "image.h"

#include <stdint.h>
#include <stdlib.h>

static enum read_status read_pixels( FILE* in, struct image* img, struct bmp_header* header ) {
  uint32_t row_size = img->width * sizeof( struct pixel );
  uint32_t padding = (4 - row_size % 4) % 4;

  img->data = malloc( img->width * img->height * sizeof( struct pixel ) );
  if ( !img->data ) {
    fprintf( stderr, "allocation error" );
    return READ_MALLOC_ERROR;
  }
  fseek( in, header->bOffBits, SEEK_SET );
  for ( size_t i = 0; i < img->height; i++ ) {
    for ( size_t j = 0; j < img->width; j++ ) {
      fread( &img->data[i * img->width + j], sizeof( struct pixel ), 1, in );
    }
    fseek( in, padding, SEEK_CUR );
  }
  return READ_OK;
}

enum read_status from_bmp( FILE* in, struct image* img ) {
  if ( !in ) {
    fprintf( stderr, "No such file\n" );
    return READ_FILE_NOT_EXISTS;
  }
  struct bmp_header header;
  if ( fread( &header, sizeof( struct bmp_header ), 1, in ) != 1) {
    fprintf( stderr, "Invalid header\n" );
    return READ_INVALID_HEADER;
  }
  if ( header.bfType != BMP_SIGNATURE ) {
    fprintf( stderr, "Not a .bmp file\n" );
    return READ_INVALID_SIGNATURE;
  }
  if ( header.biBitCount != BMP_BITS ) {
    fprintf( stderr, "Incorrect bits depth\n" );
    return READ_INVALID_BITS;
  }
  img->width = header.biWidth;
  img->height = header.biHeight;
  return read_pixels(in, img, &header );
}

enum write_status to_bmp( FILE* out, struct image const* img ) {
  if ( !out ) return WRITE_ERROR;
  uint32_t padding = (4 - (sizeof(struct pixel) * img->width) % 4) % 4;
  struct bmp_header header = ( struct bmp_header ) {
    .bfType = BMP_SIGNATURE,
    .bfileSize = img->height * ( img->width * sizeof( struct pixel ) ) + sizeof( struct bmp_header ),
    .bOffBits = sizeof( struct bmp_header ),
    .biSize = 40,
    .biWidth = img->width,
    .biHeight = img->height,
    .biPlanes = 1,
    .biBitCount = BMP_BITS,
    .biCompression = 0,
    .biSizeImage = img->height * (img->width * sizeof(struct pixel) + padding)
  };
  if ( fwrite( &header, sizeof( struct bmp_header ), 1, out ) != 1 ) {
    fprintf( stderr, "failed to write header\n" );
    return WRITE_ERROR;
  }

  struct pixel garbage_pixel;
  for ( size_t i = 0; i < img->height; i++ ) {
    for ( size_t j = 0; j < img->width; j++ ) {
      if ( fwrite( img->data + i * img->width + j, sizeof( struct pixel ), 1, out ) != 1 ) {
        fprintf( stderr, "failed to write\n" );
        return WRITE_ERROR;
      }
    }
    for ( uint32_t n = 0; n < padding; n++ ) {
        if ( fwrite( &garbage_pixel, 1, 1, out ) != 1 ) {
        fprintf( stderr, "failed to write\n" );
        return WRITE_ERROR;
      }
    }
  }

  return WRITE_OK;
}
