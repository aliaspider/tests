#version 310 es
precision highp float;

#ifdef VERTEX_SHADER
layout(location = 0) in uint id;
layout(location = 1) in vec4 color;
layout(location = 0) out uint id_out;
layout(location = 1) out vec4 color_out;

void main()
{
   id_out = id;
   color_out = color;
}
#endif

#ifdef GEOMETRY_SHADER
#extension GL_EXT_geometry_shader : require
layout(points) in;
layout(location = 0) in uint id[1];
layout(location = 1) in vec4 color[1];
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;
layout(push_constant) uniform PC
{
   vec2 vp_size;
}pc;

layout(set = 1, binding = 1, std140) uniform UBO
{
   vec2 glyph_size;
   vec2 tex_size;
};

void main()
{
   uint c = id[0] & 0xFFu;
   uint col = c & 0xFu;
   uint row = c >> 4u;

   gl_Position.zw = vec2(0.0f, 1.0f);
   vColor = color[0];
   vec2 pos = - (0.5 * pc.vp_size);
   pos.x += glyph_size.x * mod(float(gl_PrimitiveIDIn), floor(pc.vp_size.x / glyph_size.x));
   pos.y += glyph_size.y * floor(float(gl_PrimitiveIDIn)/ floor(pc.vp_size.x / glyph_size.x));
   vec2 coords = vec2(float(col), float(row)) * tex_size / 16.0;
#if 1
   gl_Position.xy = (pos + vec2(glyph_size.x, 0.0)) * 2.0 / pc.vp_size;
   vTexCoord      = coords +    vec2(glyph_size.x, 0.0);
   EmitVertex();

   gl_Position.xy = (pos + vec2(0.0, 0.0)) * 2.0 / pc.vp_size;
   vTexCoord      = coords +    vec2(0.0, 0.0);
   EmitVertex();

   gl_Position.xy = (pos + vec2(glyph_size.x, glyph_size.y)) * 2.0 / pc.vp_size;
   vTexCoord      = coords +    vec2(glyph_size.x, glyph_size.y);
   EmitVertex();

   gl_Position.xy = (pos + vec2(0.0, glyph_size.y)) * 2.0 / pc.vp_size;
   vTexCoord      = coords +    vec2(0.0, glyph_size.y);
   EmitVertex();
#else
   gl_Position.xy = vec2(1.0, -1.0) * tex_size / pc.vp_size;
   vTexCoord      = vec2(tex_size.x, 0.0);
   EmitVertex();

   gl_Position.xy = vec2(-1.0, -1.0) * tex_size / pc.vp_size;
   vTexCoord      = vec2(0.0, 0.0);
   EmitVertex();

   gl_Position.xy = vec2(1.0, 1.0) * tex_size / pc.vp_size;
   vTexCoord      = vec2(tex_size.x, tex_size.y);
   EmitVertex();

   gl_Position.xy = vec2(-1.0, 1.0) * tex_size / pc.vp_size;
   vTexCoord      = vec2(0.0, tex_size.y);
   EmitVertex();
#endif
   EndPrimitive();
}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;
layout(location = 0) out vec4 FragColor;
layout(set = 1, binding = 0) uniform highp sampler2D uTex;

void main()
{
//   FragColor = vec4(vColor.rgb, vColor.a * texture(uTex, vTexCoord).r);
   FragColor = vColor* texture(uTex, vTexCoord).r;
//   FragColor = vec4(1.0, 1.0, 1.0, texture(uTex, vTexCoord).r);
//   FragColor = vec4(1.0);
}
#endif
