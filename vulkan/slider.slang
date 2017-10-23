#version 310 es
precision highp float;

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
layout(push_constant) uniform PC
{
   vec2 vp_size;
}pc;

//layout(set = 0, binding = 0, std140) uniform UBO
//{
//};
layout(set = 0, binding = 2, std140) buffer g_buffer
{
   vec4 v[4];
};

void main()
{
   gl_Position.zw = vec2(0.0f, 1.0f);


   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(pos[0].z, 0.0);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, 0.0))* 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, 0.0);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, pos[0].w))* 2.0f / pc.vp_size;
   vTexCoord = vec2(pos[0].z, pos[0].w);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, pos[0].w))* 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, pos[0].w);
   EmitVertex();

   EndPrimitive();

   vTexCoord = vec2(1.0);
   float height = pos[0].w * info[0].y;
   float off = pos[0].w * info[0].x;

   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(pos[0].z, 0.0)) * 2.0f / pc.vp_size;
   EmitVertex();

   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(0.0, 0.0))* 2.0f / pc.vp_size;
   EmitVertex();

   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(pos[0].z, height))* 2.0f / pc.vp_size;
   EmitVertex();

   gl_Position.xy = - 1.0f + (vec2(0.0, off) + pos[0].xy + vec2(0.0, height))* 2.0f / pc.vp_size;
   EmitVertex();

   EndPrimitive();

}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 1, location = 0) uniform highp sampler2D uTex;

void main()
{
//   if(fract((vTexCoord.y - fract(vTexCoord.y)) / 2.0) == 0.5)
   if(fract((vTexCoord.y) / 2.0) >= 0.5)
      FragColor = vec4(0.0, 0.50, 1.0, 1.0);
   else
      FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
#endif
