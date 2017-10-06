#version 310 es
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

#if 0
layout(set = 0, binding = 0, std140) uniform UBO
{
   vec2 center;
   vec2 image;
   vec2 screen;
   float angle;
}global;
#endif
//layout(push_constant) uniform UBO
//{
//   vec2 center;
//   vec2 size;
//   float angle;
//}global;

void main()
{
   gl_Position = Position;
#if 0
   gl_Position.xy *= global.image;
   mat2 rot = mat2(cos(global.angle), -sin(global.angle), sin(global.angle), cos(global.angle));
   gl_Position.xy = rot * gl_Position.xy;
   gl_Position.xy += global.center;
   gl_Position.xy /= global.screen;
#endif

   vTexCoord = TexCoord;
   vColor = Color;
}
