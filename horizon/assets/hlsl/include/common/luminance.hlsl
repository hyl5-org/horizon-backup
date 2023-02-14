#ifndef __LUMINANCE__
#define __LUMINANCE__

float Luminance(float3 color) { return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b; }

#endif
