#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_radius;
uniform int u_direction;

varying vec2 v_texCoord;

void main() {
    vec4 color = vec4(0.0);
    float total = 0.0;

    for (float i = -u_radius; i <= u_radius; i++) {
        vec2 offset = vec2(i * (1.0 - u_direction), i * u_direction);
        offset /= u_resolution;
        vec2 sampleCoord = v_texCoord + offset;
        sampleCoord = clamp(sampleCoord, vec2(0.0), vec2(1.0));
        color += texture2D(u_texture, sampleCoord);
        total += 1.0;
    }

    gl_FragColor = color / total;
}
