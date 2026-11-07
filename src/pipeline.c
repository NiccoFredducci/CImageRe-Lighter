#include "pipeline.h"
#include "passes/depth_pass.h"
#include <stddef.h>

void pipeline_run(Image original, PipelineOutputs *out)
{
    pipeline_unload(out);

    out->original = original;

    out->depth = depth_pass(original);
}

Image pipeline_get_image(PipelineOutputs *out, OutputKind kind)
{
    switch (kind) {

        case OUTPUT_ORIGINAL:
            return out->original;

        case OUTPUT_DEPTH:
            return out->depth;

        case OUTPUT_NORMALS:
        case OUTPUT_ALBEDO:
        case OUTPUT_ROUGHNESS:
        case OUTPUT_METALLIC:
        case OUTPUT_NEUTRAL:
        case OUTPUT_RELIT:
            return GenImageColor(
                out->original.width,
                out->original.height,
                BLACK
            );

        default:
            return out->original;
    }
}

void pipeline_unload(PipelineOutputs *out)
{
    if (out->depth.data != NULL)
        UnloadImage(out->depth);

    out->depth = (Image){0};
}

const char *pipeline_output_name(OutputKind kind)
{
    static const char *names[] = {
        "Original",
        "Depth",
        "Normals",
        "Albedo",
        "Roughness",
        "Metallic",
        "Neutral materials",
        "Relit"
    };

    if (kind < 0 || kind >= OUTPUT_COUNT)
        return "Unknown";

    return names[kind];
}
