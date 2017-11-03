
#include <stdint.h>
#include <string.h>
#include "png_file.h"

void png_file_init(const char* filename, png_file_t* dst)
{
   dst->file = fopen(filename, "rb");

   dst->handle = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   dst->info = png_create_info_struct(dst->handle);
   png_init_io(dst->handle, dst->file);

   png_read_png(dst->handle, dst->info, PNG_TRANSFORM_IDENTITY, NULL);

   dst->width = png_get_image_width(dst->handle, dst->info);
   dst->height = png_get_image_height(dst->handle, dst->info);
   dst->rows = png_get_rows(dst->handle, dst->info);
}

void png_file_free(png_file_t* png)
{
   png_destroy_info_struct(png->handle, &png->info);
   png_destroy_read_struct(&png->handle, &png->info, NULL);
   png_free(png->handle, NULL);
   memset(png, 0, sizeof(*png));
}

void png_file_read(png_file_t* png, uint8_t* dst, int pitch)
{
   int i;
   for (i = 0; i < png->height; i++)
   {
//      uint8_t* in = png->rows[i];
//      uint32_t* out = (uint32_t*)(dst + i * pitch);q
//      int x;
//      for(x = 0; x < png->width; x++)
//      {
//         uint8_t r = *(in++);
//         uint8_t g = *(in++);
//         uint8_t b = *(in++);
//         *(out++) = (r << 0) | (g << 8) | (b << 16) | 0xFF000000;
//      }
      memcpy(dst + i * pitch, png->rows[i], png->width * sizeof(uint32_t));

   }
}
