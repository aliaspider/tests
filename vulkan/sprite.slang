#version 310 es
precision highp float;

#ifdef VERTEX_SHADER

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 coords;
layout(location = 2) in vec2 tex_size;
layout(location = 0) out vec4 pos_out;
layout(location = 1) out vec4 coords_out;
layout(location = 2) out vec2 tex_size_out;
void main()
{
   pos_out = pos;
   coords_out = coords;
   tex_size_out = tex_size;
}
#endif

#ifdef GEOMETRY_SHADER
#extension GL_EXT_geometry_shader : require
layout(points) in;
layout(location = 0) in vec4 pos[1];
layout(location = 1) in vec4 coords[1];
layout(location = 2) in vec2 tex_size[1];
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 vTexCoord;
layout(push_constant) uniform PC
{
   vec2 vp_size;
}pc;

void main()
{
   gl_Position.zw = vec2(0.0f, 1.0f);

#if 1
   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].z, coords[0].y) / tex_size[0];
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, 0.0))* 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].x, coords[0].y) / tex_size[0];
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(pos[0].z, pos[0].w))* 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].z, coords[0].w) / tex_size[0];
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, pos[0].w))* 2.0f / pc.vp_size;
   vTexCoord = vec2(coords[0].x, coords[0].w) / tex_size[0];
   EmitVertex();
#else
   gl_Position.xy = vec2(1.0, -1.0);
   vTexCoord = vec2(coords[0].z, coords[0].y) / tex_size[0];
   EmitVertex();

   gl_Position.xy = vec2(-1.0, -1.0);
   vTexCoord = vec2(coords[0].x, coords[0].y) / tex_size[0];
   EmitVertex();

   gl_Position.xy = vec2(1.0, 1.0);
   vTexCoord = vec2(coords[0].z, coords[0].w) / tex_size[0];
   EmitVertex();

   gl_Position.xy = vec2(-1.0, 1.0);
   vTexCoord = vec2(coords[0].x, coords[0].w) / tex_size[0];
//   EmitVertex();
#endif
   EndPrimitive();
}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 3, location = 0) uniform highp sampler2D uTex;

void main()
{
   FragColor = texture(uTex, vTexCoord);
//   FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
#endif
