#version 310 es
//#define highp
#ifndef highp
precision highp float;
#endif

#ifdef VERTEX_SHADER
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 size;

layout(location = 0) out vec2 pos_out;
layout(location = 1) out vec2 size_out;

void main()
{
#if 0
   gl_Position.xy *= global.image;
   mat2 rot = mat2(cos(global.angle), -sin(global.angle), sin(global.angle), cos(global.angle));
   gl_Position.xy = rot * gl_Position.xy;
   gl_Position.xy += global.center;
   gl_Position.xy /= global.screen;
#endif

   pos_out = pos;
   size_out = size;
}
#endif


#ifdef GEOMETRY_SHADER
#extension GL_EXT_geometry_shader : require
layout(points) in;
layout(location = 0) in vec2 pos[1];
layout(location = 1) in vec2 size[1];
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 vTexCoord;
layout(push_constant) uniform PC
{
   vec2 vp_size;
}pc;

layout(set = 0, binding = 0, std140) uniform UBO
{
   vec2 tex_size;
};

void main()
{
   gl_Position.zw = vec2(0.0f, 1.0f);

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(size[0].x, 0.0)) * 2.0f / pc.vp_size;
   vTexCoord = vec2(tex_size.x, 0.0);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, 0.0))* 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, 0.0);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(size[0].x, size[0].y))* 2.0f / pc.vp_size;
   vTexCoord = vec2(tex_size.x, tex_size.y);
   EmitVertex();

   gl_Position.xy = - 1.0f + (pos[0].xy + vec2(0.0, size[0].y))* 2.0f / pc.vp_size;
   vTexCoord = vec2(0.0, tex_size.y);
   EmitVertex();

   EndPrimitive();

}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 3) uniform highp sampler2D uTex;

void main()
{
   FragColor = texture(uTex, vTexCoord);
//   FragColor = texture(uTex[1], vTexCoord);
}
#endif
