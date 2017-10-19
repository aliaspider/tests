#version 310 es
precision highp float;

#ifdef VERTEX_SHADER
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 1) uniform highp sampler2D uTex[2];
#endif

#ifdef VERTEX_SHADER
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
#endif

#ifdef FRAGMENT_SHADER
void main()
{
   FragColor = texture(uTex[1], vTexCoord);
}
#endif
