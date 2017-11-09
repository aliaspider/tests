
#include <3ds.h>

#include "ctr/gpu_old.h"
#include "ctr_gu.h"
#include "common.h"
#include "video.h"
#include "input.h"

#define COLOR_ABGR(r, g, b, a) (((unsigned)(a) << 24) | ((b) << 16) | ((g) << 8) | ((r) << 0))

#define CTR_TOP_FRAMEBUFFER_WIDTH   400
#define CTR_TOP_FRAMEBUFFER_HEIGHT  240

extern const u8 sprite_shbin[];
extern const u32 sprite_shbin_size;

typedef struct
{
   float v;
   float u;
   float y;
   float x;
} ctr_scale_vector_t;

typedef struct
{
   s16 x0, y0, x1, y1;
   s16 u0, v0, u1, v1;
} ctr_vertex_t;

typedef enum
{
   CTR_VIDEO_MODE_NORMAL,
   CTR_VIDEO_MODE_800x240,
   CTR_VIDEO_MODE_400x240,
   CTR_VIDEO_MODE_3D
}ctr_video_mode_enum;

typedef struct ctr_video
{
   void* drawbuffer;
   void* depthbuffer;

   uint32_t* display_list;
   int display_list_size;
   void* texture_linear;
   void* texture_swizzled;
   int texture_width;
   int texture_height;

   ctr_scale_vector_t scale_vector;
   ctr_vertex_t* frame_coords;

   DVLB_s*         dvlb;
   shaderProgram_s shader;

   struct
   {
      int x,y,width, height;
   }vp;

   bool rgb32;
   bool vsync;
   bool smooth;
   bool menu_texture_enable;
   bool menu_texture_frame_enable;
   unsigned rotation;
   bool keep_aspect;
   bool should_resize;
   bool lcd_buttom_on;
   bool msg_rendering_enabled;

   ctr_video_mode_enum video_mode;

   bool p3d_event_pending;
   bool ppf_event_pending;
   volatile bool vsync_event_pending;

   struct
   {
      ctr_vertex_t* buffer;
      ctr_vertex_t* current;
      int size;
   }vertex_cache;

} ctr_video_t;

static inline void ctr_set_scale_vector(ctr_scale_vector_t* vec,
      int viewport_width, int viewport_height,
      int texture_width, int texture_height)
{
   vec->x = -2.0 / viewport_width;
   vec->y = -2.0 / viewport_height;
   vec->u =  1.0 / texture_width;
   vec->v = -1.0 / texture_height;
}
ctr_video_t ctr;

static void ctr_vsync_hook(ctr_video_t* ctr)
{
   ctr->vsync_event_pending = false;
}
static void video_init()
{
   DEBUG_LINE();

   ctr.vp.x                = 0;
   ctr.vp.y                = 0;
   ctr.vp.width            = CTR_TOP_FRAMEBUFFER_WIDTH;
   ctr.vp.height           = CTR_TOP_FRAMEBUFFER_HEIGHT;

   ctr.drawbuffer = vramAlloc(CTR_TOP_FRAMEBUFFER_WIDTH * CTR_TOP_FRAMEBUFFER_HEIGHT  * sizeof(uint32_t));
   ctr.display_list_size = 0x4000;
   ctr.display_list = linearAlloc(ctr.display_list_size * sizeof(uint32_t));
   GPU_Reset(NULL, ctr.display_list, ctr.display_list_size);

   ctr.vertex_cache.size = 0x1000;
   ctr.vertex_cache.buffer = linearAlloc(ctr.vertex_cache.size * sizeof(ctr_vertex_t));
   ctr.vertex_cache.current = ctr.vertex_cache.buffer;

   ctr.rgb32 = module.screen_format == screen_format_ARGB8888;
   ctr.texture_width = module.output_width;
   ctr.texture_height = module.output_height;
   ctr.texture_linear =
         linearMemAlign(ctr.texture_width * ctr.texture_height * (ctr.rgb32? 4:2), 128);
   ctr.texture_swizzled =
         linearMemAlign(ctr.texture_width * ctr.texture_height * (ctr.rgb32? 4:2), 128);

   ctr.frame_coords = linearAlloc(3 * sizeof(ctr_vertex_t));
   ctr.frame_coords->x0 = 0;
   ctr.frame_coords->y0 = 0;
   ctr.frame_coords->x1 = CTR_TOP_FRAMEBUFFER_WIDTH;
   ctr.frame_coords->y1 = CTR_TOP_FRAMEBUFFER_HEIGHT;
   ctr.frame_coords->u0 = 0;
   ctr.frame_coords->v0 = 0;
   ctr.frame_coords->u1 = CTR_TOP_FRAMEBUFFER_WIDTH;
   ctr.frame_coords->v1 = CTR_TOP_FRAMEBUFFER_HEIGHT;
   GSPGPU_FlushDataCache(ctr.frame_coords, sizeof(ctr_vertex_t));

   memset(ctr.texture_linear, 0x00, ctr.texture_width * ctr.texture_height * (ctr.rgb32? 4:2));

   ctr.dvlb = DVLB_ParseFile((u32*)sprite_shbin, sprite_shbin_size);
   ctrGuSetVshGsh(&ctr.shader, ctr.dvlb, 2, 2);
   shaderProgramUse(&ctr.shader);

   GPU_SetViewport(NULL,
                   VIRT_TO_PHYS(ctr.drawbuffer),
                   0, 0, CTR_TOP_FRAMEBUFFER_HEIGHT, CTR_TOP_FRAMEBUFFER_WIDTH);

   GPU_DepthMap(-1.0f, 0.0f);
   GPU_SetFaceCulling(GPU_CULL_NONE);
   GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
   GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
   GPU_SetBlendingColor(0, 0, 0, 0);
#if 0
   GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
#endif
   GPU_SetDepthTestAndWriteMask(false, GPU_ALWAYS, GPU_WRITE_COLOR);

   GPUCMD_AddMaskedWrite(GPUREG_EARLYDEPTH_TEST1, 0x1, 0);
   GPUCMD_AddWrite(GPUREG_EARLYDEPTH_TEST2, 0);

   GPU_SetAlphaBlending(GPU_BLEND_ADD, GPU_BLEND_ADD,
                        GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA,
                        GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
   GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);

   GPU_SetTextureEnable(GPU_TEXUNIT0);

   GPU_SetTexEnv(0, GPU_TEXTURE0, GPU_TEXTURE0, 0, 0, GPU_REPLACE, GPU_REPLACE, 0);
   GPU_SetTexEnv(1, GPU_PREVIOUS, GPU_PREVIOUS, 0, 0, 0, 0, 0);
   GPU_SetTexEnv(2, GPU_PREVIOUS, GPU_PREVIOUS, 0, 0, 0, 0, 0);
   GPU_SetTexEnv(3, GPU_PREVIOUS, GPU_PREVIOUS, 0, 0, 0, 0, 0);
   GPU_SetTexEnv(4, GPU_PREVIOUS, GPU_PREVIOUS, 0, 0, 0, 0, 0);
   GPU_SetTexEnv(5, GPU_PREVIOUS, GPU_PREVIOUS, 0, 0, 0, 0, 0);

   ctrGuSetAttributeBuffers(2,
                            VIRT_TO_PHYS(ctr.frame_coords),
                            CTRGU_ATTRIBFMT(GPU_SHORT, 4) << 0 |
                            CTRGU_ATTRIBFMT(GPU_SHORT, 4) << 4,
                            sizeof(ctr_vertex_t));
   GPUCMD_Finalize();
   ctrGuFlushAndRun(true);

   ctr.p3d_event_pending = true;
   ctr.ppf_event_pending = false;

   ctr.smooth        = video.filter;
   ctr.vsync         = video.vsync;


//   refresh_rate = (32730.0 * 8192.0) / 4481134.0;

   gspSetEventCallback(GSPGPU_EVENT_VBlank0, (ThreadFunc)ctr_vsync_hook, &ctr, false);

}


static void video_render()
{
//   DEBUG_LINE();
}

static void video_destroy()
{
   DEBUG_LINE();
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}

const video_t video_3ds =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

