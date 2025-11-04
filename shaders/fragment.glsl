#version 430 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D screen;

void main() {
    fragColor = texture(screen, uv);
}
