struct VertexIn {
    @location(0) position: vec2f,
    @location(1) color: vec3f
};

struct VertexOut {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f
};


@vertex
fn vs_main(in: VertexIn) -> VertexOut {
    var out: VertexOut; // create the output struct
    out.position = vec4f(in.position, 0.0, 1.0); // same as what we used to directly return
    out.color = in.color; // forward the color attribute to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOut) -> @location(0) vec4f {
    return vec4f(in.color, 1.0); // use the interpolated color coming from the vertex shader
}