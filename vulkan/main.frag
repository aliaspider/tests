#version 310 es
precision highp float;
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 1) uniform highp sampler2D uTex;

void main()
{
//   FragColor = vec4(1.0, 1.0, 1.0, 1.0);
//   FragColor = vColor;
//   FragColor = vColor * texture(uTex, vTexCoord);
//   FragColor = texture(uTex, vTexCoord) + vColor;
   FragColor = texture(uTex, vTexCoord);
//   FragColor = texture(uTex, vec2(-10, 10));
//   FragColor.r = 1.0;
//   FragColor = vec4(0.5, 0.5, 0.5, 0.0);
}
