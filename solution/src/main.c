#include "bmp.h"
#include "transform.h"

#include <stdio.h>
#include <string.h>

int main( int argc, char** argv ) {
  if ( argc == 2 && ( strcmp( argv[1], "help" ) == 0 || strcmp( argv[1], "-help" ) == 0 || strcmp( argv[1], "--help" ) == 0 || strcmp( argv[1], "-h" ) == 0 ) ){
    printf(
      "Usage: %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n",
      "./image-transform <source-image> <transformed-image> <tranformation>",
      "none", "ничего не делать, сохраняется исходное изображение",
      "cw90", "повернуть изображение на 90 градусов по часовой стрелке",
      "ccw90", "повернуть изображение на 90 градусов против часовой стрелке",
      "fliph", "отразить изображение по горизониали",
      "flipv", "отразить изображене по вертикали"
    );
    return 0;
  }
  if ( argc != 4 ) {
    fprintf( stderr, "Wrong number of arguements!\n%s <source-image> <transformed-image> <tranformation>\n", argv[0] );
    return -1;
  }
  const char* source_image = argv[1];
  const char* transformed_image = argv[2];
  const char* transformation = argv[3];

  FILE* file = fopen( source_image, "rb" );
  if ( !file ) {
    fprintf(stderr, "No such file\n");
    return READ_FILE_NOT_EXISTS;
  }
  struct image img = { .data = NULL };
  enum read_status read_stat = from_bmp( file, &img );
  fclose( file );
  if ( read_stat ) {
    if ( img.data ) free( img.data );
    return read_stat;
  }

  struct image new_image = { .data = NULL };
  if ( !strcmp( transformation, "none" ) ) new_image = none( img );
  else if ( !strcmp( transformation, "cw90" ) ) new_image = cw90( img );
  else if ( !strcmp( transformation, "ccw90" ) ) new_image = ccw90( img );
  else if ( !strcmp( transformation, "fliph" ) ) new_image = fliph( img );
  else if ( !strcmp( transformation, "flipv" ) ) new_image = flipv( img );
  else {
    fprintf( stderr, "%s arg does not exist\n", transformation );
    free( img.data );
    return -1;
  }
  
  file = fopen( transformed_image, "wb" );
  enum write_status wr_stat = to_bmp( file, &new_image );
  free( new_image.data );
  if ( wr_stat ) {
    fprintf(stderr, "Failed to write to destination image\n");
  }
  fclose( file );
  return wr_stat;
}
