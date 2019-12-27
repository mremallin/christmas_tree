#version 320

uniform sampler2D tex0;
uniform float border_size = 0.01f;
uniform float disc_radius = 0.5f;
uniform vec4 disc_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec2 disc_center = vec2(0.5, 0.5);
void main (void)
{
  vec2 uv = gl_TexCoord[0].xy;
  
  vec4 bkg_color = texture2D(tex0,uv * vec2(1.0, -1.0));

  // Offset uv with the center of the circle.
  uv -= disc_center;

  float dist = sqrt(dot(uv, uv));
  float t = smoothstep(disc_radius+border_size, disc_radius-border_size, dist);
  gl_FragColor = mix(bkg_color, disc_color, t);
}
