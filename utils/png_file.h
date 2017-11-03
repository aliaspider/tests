#ifndef PNG_FILE_H
#define PNG_FILE_H

#include <libpng16/png.h>

typedef struct
{
   FILE* file;
   png_struct* handle;
   png_info* info;
   int width;
   int height;
   png_bytepp rows;
}png_file_t;

void png_file_init(const char* filename, png_file_t* dst);
void png_file_free(png_file_t* png);
void png_file_read(png_file_t* png, uint8_t *dst, int pitch);
#endif // PNG_FILE_H
