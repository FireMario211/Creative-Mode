#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform vec2 u_distOffsets[9];

varying vec2 v_texCoord;

void main() {

    vec4 color = vec4(0.0);

    color += texture2D(u_texture, v_texCoord + u_distOffsets[0]) * 0.05;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[1]) * 0.1;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[2]) * 0.15;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[3]) * 0.2;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[4]) * 0.3;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[5]) * 0.2;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[6]) * 0.15;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[7]) * 0.1;
    color += texture2D(u_texture, v_texCoord + u_distOffsets[8]) * 0.05;

    gl_FragColor = color * 0.77;
}
