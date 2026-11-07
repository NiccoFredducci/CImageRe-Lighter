#include "depth_pass.h"

Image depth_pass(Image input)
{
    return GenImageColor(
        input.width,
        input.height,
        BLACK
    );
}