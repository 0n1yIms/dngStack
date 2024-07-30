#version 430 core

// inputs from compute shader
//
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint gl_LocalInvocationIndex;
//
// more details at https://www.khronos.org/opengl/wiki/Compute_Shader#Inputs

// outputs will need to be either in a shader storage buffer object
// or an image load store
//
// more details at https://www.khronos.org/opengl/wiki/Compute_Shader#Outputs

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(r16ui, binding = 0) uniform readonly uimage2D inTex;
layout(rgba8ui, binding = 1) uniform writeonly uimage2D outTex;

uniform uint blackLevel[];
uniform uint whiteLevel;
uniform vec3 neutralPoint;
uniform mat3 sensorToIntermediate;


uint getPix(int x, int y)
{
    return imageLoad(inTex, ivec2(x, y)).r;
}
int par(int x ,int y)
{
    int a = (x / 2) * 2;
    int b = (y / 2) * 2;
    if(a == x && y == b)
        return 0;
    else if(a != x && y == b)
        return 1;
    else if(a == x && y != b)
        return 2;
    else //if(a != x && y != b)
        return 3;
}

float linearization(int x, int y)
{
    float color = float(getPix(x, y));
    float whitePoint = float(whiteLevel);
    float blackPoint;
    if(par(x, y) == 0)
        blackPoint = float(blackLevel[0]);
    else if(par(x, y) == 1)
        blackPoint = float(blackLevel[1]);
    else if(par(x, y) == 2)
        blackPoint = float(blackLevel[2]);
    else //if(par(x, y) == 3)
        blackPoint = float(blackLevel[3]);
    color = (color - blackPoint) / (whitePoint - blackPoint);// * gm(x, y, par(x, y));
    return color < 0.f ? 0.f : color > 1.f ? 1.f : color;
}

float getMosaic(int x, int y, int a, int b)
{
    int x0 = x + a;
    int y0 = y + b;
    return linearization(x0, y0);
}

vec3 demoBRGGB(int x, int y) {
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    int p = par(x, y);

    if (p == 0)              //par par
    {
        /* B G B
         * G R G
         * B G B
         * */
        b   += getMosaic(x, y, -1,-1);
        g   += getMosaic(x, y,  0,-1);
        b   += getMosaic(x, y,  1,-1);
        g   += getMosaic(x, y, -1, 0);
        r   += getMosaic(x, y,  0, 0);
        g   += getMosaic(x, y,  1, 0);
        b   += getMosaic(x, y, -1, 1);
        g   += getMosaic(x, y,  0, 1);
        b   += getMosaic(x, y,  1, 1);
        g /= 4.f;
        b /= 4.f;
    }
    else if (p == 1)         //impar par
    {
        /*
         * G B G
         * R G R
         * G B G
         * */
        g    += getMosaic(x, y, -1, -1);
        b    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        r    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        r    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        b    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else if (p == 2)         //par impar
    {
        /*
         * G R G
         * B G B
         * G R G
         * */
        g    += getMosaic(x, y, -1, -1);
        r    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        b    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        b    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        r    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else //if (p == 3)         //impar impar
    {
        /*
         * R G R
         * G B G
         * R G R
         * */
        r    += getMosaic(x, y, -1, -1);
        g    += getMosaic(x, y,  0, -1);
        r    += getMosaic(x, y,  1, -1);
        g    += getMosaic(x, y, -1,  0);
        b    += getMosaic(x, y,  0,  0);
        g    += getMosaic(x, y,  1,  0);
        r    += getMosaic(x, y, -1,  1);
        g    += getMosaic(x, y,  0,  1);
        r    += getMosaic(x, y,  1,  1);
        r /= 4.0;
        g /= 4.0;
    }

    return vec3(r, g, b);
}


vec3 demoBBGGR(int x, int y) {
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    int p = par(x, y);

    if (p == 0)              //par par
    {
        /* R G R
         * G B G
         * R G R
         * */
        r   += getMosaic(x, y, -1,-1);
        g   += getMosaic(x, y,  0,-1);
        r   += getMosaic(x, y,  1,-1);
        g   += getMosaic(x, y, -1, 0);
        b   += getMosaic(x, y,  0, 0);
        g   += getMosaic(x, y,  1, 0);
        r   += getMosaic(x, y, -1, 1);
        g   += getMosaic(x, y,  0, 1);
        r   += getMosaic(x, y,  1, 1);
        g /= 4.f;
        r /= 4.f;
    }
    else if (p == 1)         //impar par
    {
        /*
         * G R G
         * B G B
         * G R G
         * */
        g    += getMosaic(x, y, -1, -1);
        r    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        b    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        b    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        r    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else if (p == 2)         //par impar
    {
        /*
         * G B G
         * R G R
         * G B G
         * */
        g    += getMosaic(x, y, -1, -1);
        b    += getMosaic(x, y,  0, -1);
        g    += getMosaic(x, y,  1, -1);
        r    += getMosaic(x, y, -1,  0);
        g    += getMosaic(x, y,  0,  0);
        r    += getMosaic(x, y,  1,  0);
        g    += getMosaic(x, y, -1,  1);
        b    += getMosaic(x, y,  0,  1);
        g    += getMosaic(x, y,  1,  1);
        r /= 2.0;
        g /= 5.0;
        b /= 2.0;
    }
    else //if (p == 3)         //impar impar
    {
        /*
         * B G B
         * G R G
         * B G B
         * */
        b    += getMosaic(x, y, -1, -1);
        g    += getMosaic(x, y,  0, -1);
        b    += getMosaic(x, y,  1, -1);
        g    += getMosaic(x, y, -1,  0);
        r    += getMosaic(x, y,  0,  0);
        g    += getMosaic(x, y,  1,  0);
        b    += getMosaic(x, y, -1,  1);
        g    += getMosaic(x, y,  0,  1);
        b    += getMosaic(x, y,  1,  1);
        b /= 4.0;
        g /= 4.0;
    }

    return vec3(r, g, b);
}

vec3 applyCS(vec3 color)
{
    vec3 c = vec3(  clamp(color.r, 0.f, neutralPoint.r),
                    clamp(color.g, 0.f, neutralPoint.g),
                    clamp(color.b, 0.f, neutralPoint.b)
                );
    c = c * sensorToIntermediate;
    return clamp(c, 0., 1.);
}

vec3 saturation(vec3 color, float sat)
{
    float wp = (color.r / 3.f + color.g / 3.f + color.b / 3.f);

    vec3 c = vec3(0.f, 0.f, 0.f);
    c.r = (color.r - wp) * sat + wp;
    c.g = (color.g - wp) * sat + wp;
    c.b = (color.b - wp) * sat + wp;
    return clamp(c, 0.f, 1.f);
}

vec3 gain(vec3 color, float curve)
{
    vec3 c = vec3(0.f, 0.f, 0.f);
    c.r = pow(color.r, 1.f / curve);
    c.g = pow(color.g, 1.f / curve);
    c.b = pow(color.b, 1.f / curve);
    return c;
}



void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

    vec3 color = demoBBGGR(pos.x, pos.y);
    color = applyCS(color);
    color = saturation(color, 1.2f);
    color = gain(color, 2.2f);

    color = color * 255.f;

    imageStore(outTex, pos, uvec4(color, 255));
}