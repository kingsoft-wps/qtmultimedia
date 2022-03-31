// Make sure to also modify externalsampler_gles.frag when modifying this

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    mat4 colorMatrix;
    float opacity;
    float width;
    // HDR metadata required for tonemapping
    float masteringWhite; // in PQ values
    float maxLumPQ; // in PQ values
    float maxLum;
} ubuf;
