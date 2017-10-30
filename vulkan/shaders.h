
const uint32_t vs_code [] =
#include xstr(SHADER_FILE.vert.inc)
   ;
const uint32_t ps_code [] =
#include xstr(SHADER_FILE.frag.inc)
   ;
const uint32_t gs_code [] =
#include xstr(SHADER_FILE.geom.inc)
   ;

#define SHADER_INFO \
.shaders.vs.code = vs_code, \
.shaders.vs.code_size = sizeof(vs_code), \
.shaders.ps.code = ps_code, \
.shaders.ps.code_size = sizeof(ps_code), \
.shaders.gs.code = gs_code, \
.shaders.gs.code_size = sizeof(gs_code)
