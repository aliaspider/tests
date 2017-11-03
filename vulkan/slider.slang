#version 310 es
precision highp float;
//#define NO_TEX
#ifdef VERTEX_SHADER

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 info;
layout(location = 0) out vec4 pos_out;
layout(location = 1) out vec2 info_out;
void main()
{
   pos_out = pos;
   info_out = info;
}
#endif

#ifdef GEOMETRY_SHADER
#extension GL_EXT_geometry_shader : require
layout(points) in;
layout(location = 0) in vec4 pos[1];
layout(location = 1) in vec2 info[1];
layout(triangle_strip, max_vertices = 8) out;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 size;
layout(location = 2) out int path;
layout(push_constant) uniform PC
{
   vec2 vp_size;
} pc;

//layout(set = 0, binding = 0, std140) uniform UBO
//{
//};
//layout(set = 0, binding = 2, std140) buffer g_buffer
//{
//};

void main()
{
   gl_Position.zw = vec2(0.0f, 1.0f);
   path = 0;
   size = vec2(pos[0].z, pos[0].w);


   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(pos[0].z, 0.0);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, 0.0);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, pos[0].w)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(pos[0].z, pos[0].w);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, pos[0].w)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, pos[0].w);
   EmitVertex();

   EndPrimitive();

   path = 1;
   float height = pos[0].w * info[0].y;
   float off = pos[0].w * info[0].x;
   size = vec2(pos[0].z, height);

   // -#
   // --
   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(pos[0].z, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(pos[0].z, 0.0);
   EmitVertex();

   // #-
   // --
   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(0.0, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, 0.0);
   EmitVertex();

   // --
   // -#
   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(pos[0].z, height)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(pos[0].z, height);
   EmitVertex();

   // --
   // #-
   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(0.0, height)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, height);
   EmitVertex();

   EndPrimitive();

}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 size;
layout(location = 2) in flat int path;
layout(location = 0) out vec4 FragColor;
//layout(set = 0, binding = 1, location = 0) uniform highp sampler2D uTex;
layout(set = 2, binding = 0, location = 0) uniform highp sampler2D uTex;
layout(set = 2, binding = 1, std140) uniform UBO
{
   vec2 size;
   int format;
   int ignore_alpha;
} tex;
#ifdef NO_TEX
void main()
{
   if (path == 0)
   {
      if (fract((vTexCoord.y) / 2.0) >= 0.5)
         FragColor = vec4(0.0, 0.50, 1.0, 1.0);
      else
         FragColor = vec4(0.0, 0.0, 0.0, 1.0);

   }
   else
   {
      vec4 BG = vec4(1.0, 0.0, 0.0, 1.0);
      vec4 FG = vec4(1.0, 1.0, 1.0, 1.0);
      vec2 border = vec2(2.0, 2.0);

      vec2 coords = vTexCoord / size;
      FG.a = 1.0 - clamp((coords.x  - 0.35), 0.0, 1.0) ;
      FG.a = exp(-3.14 * FG.a * FG.a);

      float r = size.x / 2.8;
//      float r = 15.0;
      vec2 coords2 = vTexCoord;

      coords2 = clamp(abs(vTexCoord - size / 2.0), size / 2.0 - vec2(r), size / 2.0) - size / 2.0 + vec2(r);
      BG.a *= step(dot(coords2, coords2), r * r);
//      coords2 = clamp(abs(vec2(vTexCoord.x / 2.0, vTexCoord.y) - vec2(3.0 * size.x / 4.0 ,size.y / 2.0)),
//      vec2(size.x / 4.0, size.y / 2.0) - vec2(r), vec2(size.x / 4.0, size.y / 2.0)) - vec2(size.x / 4.0, size.y / 2.0) + vec2(r);
      FG.a *= step(dot(coords2, coords2), (r - border.x) * (r - border.y));

      FragColor = BG.rgba * (1.0 - FG.a) + vec4(FG.rgb * FG.a, FG.a);
   }
}
#else
void main()
{
//   if(fract((vTexCoord.y - fract(vTexCoord.y)) / 2.0) == 0.5)
   if (path == 0)
   {
//      if (fract((vTexCoord.y) / 2.0) >= 0.5)
//         FragColor = vec4(0.0, 0.50, 1.0, 1.0);
//      else
//         FragColor = vec4(0.0, 0.0, 0.0, 1.0);

      vec2 coords = vTexCoord;
      vec2 tile_size = vec2(32.0, 20.0);
      vec2 tile_offset = vec2(32.0, 0.0);
      /* vertical */
      tile_size = tile_size.yx;

      coords.x *= tile_size.x / size.x;
      coords.y = mod(coords.y, tile_size.y);


      coords = coords.yx;
      FragColor = texture(uTex, coords + tile_offset);
//      FragColor = vec4(0.3, 0.3, 0.3, FragColor.a);
   }
   else
   {
      vec2 tile_size = vec2(32.0, 20.0);
      vec2 tile_offset = vec2(0.0, 0.0);


//      vec2 tile_size = vec2(64.0, 32.0);
//      vec2 tile_offset = vec2(192.0, 0.0);
      /* vertical */
      vec2 center = (tile_size.yx / 2.0);
      vec2 coords = (vTexCoord - size / 2.0);
      coords = center + clamp(abs(coords) - (size / 2.0) + center, vec2(0.0), center) * -sign(coords);
      coords = vec2(tile_size.x - coords.y, coords.x);

      FragColor = texture(uTex, coords + tile_offset);
   }
}
#endif
#endif
