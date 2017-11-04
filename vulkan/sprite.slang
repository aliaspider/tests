#version 310 es
precision highp float;

#ifdef VERTEX_SHADER

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 coords;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 effect;
layout(location = 0) out vec4 pos_out;
layout(location = 1) out vec4 coords_out;
layout(location = 2) out vec4 color_out;
layout(location = 3) out vec2 effect_out;
void main()
{
   pos_out = pos;
   coords_out = coords;
   color_out = color;
   effect_out = effect;
}
#endif

#ifdef GEOMETRY_SHADER
#extension GL_EXT_geometry_shader : require
layout(points) in;
layout(location = 0) in vec4 pos[1];
layout(location = 1) in vec4 coords[1];
layout(location = 2) in vec4 color[1];
layout(location = 3) in vec2 effect[1];
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec4 pos_out;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out vec4 color_out;
layout(location = 3) out vec2 effect_out;
layout(push_constant) uniform PC
{
   vec2 vp_size;
} pc;

void main()
{
   gl_Position.zw = vec2(0.0f, 1.0f);
   color_out = color[0];
   effect_out = effect[0];
   pos_out.zw = pos[0].zw;

#if 1
   pos_out.xy = vec2(pos[0].z, 0.0);
   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].x + coords[0].z, coords[0].y);
   EmitVertex();

   pos_out.xy = vec2(0.0, 0.0);
   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].x, coords[0].y);
   EmitVertex();


   pos_out.xy = vec2(pos[0].z, pos[0].w);
   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, pos[0].w)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].x + coords[0].z, coords[0].y + coords[0].w);
   EmitVertex();

   pos_out.xy = vec2(0.0, pos[0].w);
   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, pos[0].w)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].x, coords[0].y + coords[0].w);
   EmitVertex();
#else
   gl_Position.xy = vec2(1.0, -1.0);
   vTexCoord = vec2(coords[0].z, coords[0].y);
   EmitVertex();

   gl_Position.xy = vec2(-1.0, -1.0);
   vTexCoord = vec2(coords[0].x, coords[0].y);
   EmitVertex();

   gl_Position.xy = vec2(1.0, 1.0);
   vTexCoord = vec2(coords[0].z, coords[0].w);
   EmitVertex();

   gl_Position.xy = vec2(-1.0, 1.0);
   vTexCoord = vec2(coords[0].x, coords[0].w);
   EmitVertex();
#endif
   EndPrimitive();
}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 effect;
layout(location = 0) out vec4 FragColor;

layout(set = 2, binding = 0, location = 0) uniform highp sampler2D uTex;

layout(set = 2, binding = 1, std140) uniform UBO
{
   vec2 size;
   int format;
   int ignore_alpha;
} tex;

#define edge   effect.x
#define gloss  effect.y
#define size pos.zw
#define center (size / 2.0)

void main()
{
   if (tex.format == 0)
      FragColor = color;
   else if (tex.format > 8 && tex.format < 16)
      FragColor = texture(uTex, vTexCoord).r * color;
   else
      FragColor = texture(uTex, vTexCoord) * color;

//      FragColor = texture(uTex, vTexCoord);
//   FragColor = vec4(1.0, 1.0, 1.0, 1.0);

   if (tex.ignore_alpha != 0)
      FragColor.a = color.a;

   vec2 border = vec2(2.0, 2.0);
   float gloss_alpha = 0.0;

   if(gloss != 0.0)
   {
      vec2 coords = pos.xy / size.xy;
      gloss_alpha = clamp((coords.y  + 0.3), 0.0, 1.0) ;
      gloss_alpha = exp(-3.14 * gloss_alpha * gloss_alpha);

   }
//#define size vec2(40.0, 70.0)
//#undef center
//#define center vec2(20.0, 25.0)
   if(edge != 0.0)
   {
      float r;
//      if (edge == 1.0/255.0)
         r = min(size.x, size.y) / 5.0;
//      else
//         r = edge * 255.0;

      r = 15.0;

      vec2 coords = pos.xy;

      coords = clamp(abs(pos.xy - center), center - vec2(r), center) - center + vec2(r);
      FragColor.a *= step(dot(coords, coords), r * r);
//      coords2 = clamp(abs(vec2(vTexCoord.x / 2.0, vTexCoord.y) - vec2(3.0 * size.x / 4.0 ,size.y / 2.0)),
//      vec2(size.x / 4.0, size.y / 2.0) - vec2(r), vec2(size.x / 4.0, size.y / 2.0)) - vec2(size.x / 4.0, size.y / 2.0) + vec2(r);

      gloss_alpha *= step(dot(coords, coords), (r - border.x) * (r - border.y));

//      FG.a *= step(-vTexCoord.x, r * 1.5 - size.x);

   }
//   gloss_alpha = 1.0;
//   gloss_alpha = vTexCoord.y / size.y;

   FragColor.rgb = vec3(1.0) * gloss_alpha + FragColor.rgb * (1.0 - gloss_alpha);


}
#endif

