uniform sampler2D texture;
uniform float time;  
uniform ivec2 position;  
uniform ivec2 size;       
uniform ivec2 bias;              
varying vec2 texture_coordinate;
void main(void) {
  float cr = (gl_FragCoord.x) * 0.001 - 0.8;
  float ci = (gl_FragCoord.y) * 0.001 + 0.6;
  float ar = cr;
  float ai = ci;
  float tr, ti;
  float col = 0.0;
  float p = 0.0;
  int i = 0;
  for (int i2 = 1; i2 < 29; i2++) {
    tr = ar * ar - ai * ai + cr;
    ti = 2.0 * ar * ai + ci;
    p = tr * tr + ti * ti;
    ar = tr;
    ai = ti;
    if (p > 16.0) {
      i = i2;
      break;
    }
  }
  gl_FragColor = vec4(float(i) * 0.0625, 0, 0, 1);
}
