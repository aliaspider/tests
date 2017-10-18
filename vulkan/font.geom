#version 310 es
//#version 200
#extension GL_EXT_geometry_shader : require

precision highp float;
layout(points) in;
layout(location = 0) in uint id[1];
layout(location = 1) in vec3 color[1];
layout(location = 2) in vec2 position[1];

layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

layout(set = 0, binding = 0, std140) uniform UBO
{
   vec2 vp_size;
   vec2 tex_size;
   vec4 glyph_metrics[256]; // (x, y, w, h)
   float advance[256];
};
layout(set = 0, binding = 2, std140) buffer g_buffer
{
   coherent int posx;
   coherent int posy;
   coherent int lastid;
};

void main()
{
   uint c = id[0] & 0xFFu;
   uint col = c & 0xFu;
   uint row = c >> 4u;

   gl_Position.zw = vec2(0.0f, 1.0f);
   vColor = vec4(color[0], 1.0);
   vec2 pos = (2.0 * ((position[0] + glyph_metrics[c].xy) / vp_size)) - vec2(1.0);
   vec2 coords = vec2(float(col) / 16.0, float(row) / 16.0);

   gl_Position.xy = pos + 2.0 * vec2(glyph_metrics[c].z, 0.0) / vp_size;
   vTexCoord      = coords +    vec2(glyph_metrics[c].z, 0.0) / tex_size;
   EmitVertex();

   gl_Position.xy = pos + 2.0 * vec2(0.0, 0.0) / vp_size;
   vTexCoord      = coords +    vec2(0.0, 0.0) / tex_size;
   EmitVertex();

   gl_Position.xy = pos + 2.0 * vec2(glyph_metrics[c].z, glyph_metrics[c].w) / vp_size;
   vTexCoord      = coords +    vec2(glyph_metrics[c].z, glyph_metrics[c].w) / tex_size;
   EmitVertex();

   gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;
   vTexCoord      = coords +    vec2(0.0, glyph_metrics[c].w) / tex_size;
   EmitVertex();

   EndPrimitive();
}
