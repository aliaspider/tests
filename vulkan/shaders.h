
#define VS_SHADER_FILE xstr(SHADER_FILE.vert.inc)
#define PS_SHADER_FILE xstr(SHADER_FILE.frag.inc)
#define GS_SHADER_FILE xstr(SHADER_FILE.geom.inc)

const uint32_t vs_code [] =
#include VS_SHADER_FILE
   ;
const uint32_t ps_code [] =
#include PS_SHADER_FILE
   ;
const uint32_t gs_code [] =
#include GS_SHADER_FILE
   ;
