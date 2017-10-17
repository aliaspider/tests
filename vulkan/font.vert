#version 310 es
layout(location = 0) in uint id;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 position;
layout(location = 0) out uint id_out;
layout(location = 1) out vec3 color_out;
layout(location = 2) out vec2 position_out;

void main()
{
   id_out = id;
   color_out = color;
   position_out = position;
}
