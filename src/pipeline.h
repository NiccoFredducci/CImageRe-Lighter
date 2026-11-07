#ifndef PIPELINE_H
#define PIPELINE_H

#include "raylib.h"

typedef enum {
    OUTPUT_ORIGINAL = 0,
    OUTPUT_DEPTH,
    OUTPUT_NORMALS,
    OUTPUT_ALBEDO,
    OUTPUT_ROUGHNESS,
    OUTPUT_METALLIC,
    OUTPUT_NEUTRAL,
    OUTPUT_RELIT,
    OUTPUT_COUNT
} OutputKind;

typedef struct {
    Image original;
    Image depth;
    Image normals;
    Image albedo;
    Image roughness;
    Image metallic;
    Image neutral;
    Image relit;
} PipelineOutputs;

void pipeline_run(Image original, PipelineOutputs *out);

Image pipeline_get_image(PipelineOutputs *out, OutputKind kind);

void pipeline_unload(PipelineOutputs *out);

const char *pipeline_output_name(OutputKind kind);

#endif
