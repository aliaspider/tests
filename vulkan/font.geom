#version 310 es
#extension GL_EXT_geometry_shader : require

precision highp float;
layout(points) in;
layout(location = 0) in uint id[1];
layout(location = 1) in vec3 color[1];
layout(location = 2) in vec2 position[1];

layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

const vec2 p0 = vec2(1.0f, -1.0f);
const vec2 p1 = vec2(-1.0f, -1.0f);
const vec2 p2 = vec2(1.0f, 1.0f);
const vec2 p3 = vec2(-1.0f, 1.0f);
const vec2 t0 = vec2(1.0f, 0.0f);
const vec2 t1 = vec2(0.0f, 0.0f);
const vec2 t2 = vec2(1.0f, 1.0f);
const vec2 t3 = vec2(0.0f, 1.0f);

const vec2 screen_size = vec2(640.0, 480.0);
const vec2 atlas_size = vec2(640.0, 480.0);
const vec2 glyph_size = vec2(640.0 / 16.0, 480.0 / 16.0);

//OpDecorate %wgsize BuiltIn WorkgroupSize

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
   vec2 pos = (2.0 * (position[0] / screen_size)) - vec2(1.0);
   pos.x += 12.0f * 2.0f / screen_size.x;


   gl_Position.xy = pos + p0 * glyph_size / screen_size;
   vTexCoord = vec2(float(col) / 16.0, float(row) / 16.0) + (t0 * glyph_size + vec2(1.0, 3.0)) / atlas_size;
   EmitVertex();

   gl_Position.xy = pos + p1 * glyph_size / screen_size;
   vTexCoord = vec2(float(col) / 16.0, float(row) / 16.0) + (t1 * glyph_size + vec2(1.0, 3.0)) / atlas_size;
   EmitVertex();

   gl_Position.xy = pos + p2 * glyph_size / screen_size;
   vTexCoord = vec2(float(col) / 16.0, float(row) / 16.0) + (t2 * glyph_size + vec2(1.0, 3.0)) / atlas_size;
   EmitVertex();

   gl_Position.xy = pos + p3 * glyph_size / screen_size;
   vTexCoord = vec2(float(col) / 16.0, float(row) / 16.0) + (t3 * glyph_size + vec2(1.0, 3.0)) / atlas_size;
   EmitVertex();

   EndPrimitive();
}
