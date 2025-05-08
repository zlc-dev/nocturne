struct VertexIn {
    @location(0) position: vec2f,
    @location(1) color: vec3f
};

struct VertexOut {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f
};


@vertex
fn vs_main(@location(0) position: vec3f) -> @builtin(position) vec4f {
    return vec4f(position, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 0.8, 1.0);
}