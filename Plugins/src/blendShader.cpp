#ifndef ai_common_mtds
#include <ai.h>
#endif
#include <string.h>
#include <iostream>
#include <math.h>

#include <opencv2/highgui.hpp>

AI_SHADER_NODE_EXPORT_METHODS(BlendMethods);


enum BlendParams
{
    p_mask,
    p_blend_image,
    p_force_scene,
    p_Kd_bcolor
};

node_parameters
{
    AiParameterPtr("mask", NULL);
    AiParameterPtr("blend_image", NULL);
    AiParameterBool("force_scene", false);
    AiParameterRGB("Kd_bcolor", 0.f, 0.f, 0.f);
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    using namespace cv;

    Mat mask = *(Mat *)AiShaderEvalParamPtr(p_mask);
    Mat blend_image = *(Mat *)AiShaderEvalParamPtr(p_blend_image);
    AtRGB rgba;

    int x = sg->x;
    int y = sg->y;

    // std::cout << (unsigned int)mask.at<char>(y,x) <<  "\n";
    Vec3b p = blend_image.at<Vec3b>(y, x);
    float avg = 0;
    int steps = 0;
    if (!AiShaderEvalParamBool(p_force_scene))
    {
        for (int i = -3; i < 4; i++)
        {
            for (int j = -3; j < 4; j++)
            {
                Vec3b curr = blend_image.at<Vec3b>(y + i*2, x + j*2);

                avg += curr[0] + curr[1] + curr[2];
            }
        }
        avg = avg / (1.2f * 49.f * 255);
    }
    if (avg > 1.3)
        avg = 1.3;
    if (avg < 0.35)
        avg = 0.35;

    float mask_sum = mask.at<uchar>(2 * y, 2 * x);
    mask_sum += mask.at<uchar>(2 * y + 1, 2 * x);
    mask_sum += mask.at<uchar>(2 * y, 2 * x + 1);
    mask_sum += mask.at<uchar>(2 * y + 1, 2 * x + 1);
    float body_ratio = mask_sum / 1020.0f;


    AtRGB bodyC = AiShaderEvalParamRGB(p_Kd_bcolor);
    if (AiShaderEvalParamBool(p_force_scene))
    {
        AtColor opac;
        if (body_ratio <= 0.5)
        {
            opac.r = 1;
            opac.g = 1;
            opac.b = 1;
        }
        else
        {
            opac.r = 0;
            opac.g = 0;
            opac.b = 0;
        }
        sg->out_opacity = opac;
        body_ratio = 0;
    }
    float scene_ratio = 1.0 - body_ratio;

    rgba.r = bodyC.r * body_ratio * 1.5 * avg;
    rgba.g = bodyC.g * body_ratio * 1.5 * avg;
    rgba.b = bodyC.b * body_ratio * 1.5 * avg;

    rgba.r += (((float)p[2]) / 255.0) * scene_ratio;
    rgba.g += (((float)p[1]) / 255.0) * scene_ratio;
    rgba.b += (((float)p[0]) / 255.0) * scene_ratio;
    
    sg->out.RGB = rgba;
}