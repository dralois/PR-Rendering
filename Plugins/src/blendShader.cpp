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
    p_rend_image,
    p_force_scene,
    p_Kd_bcolor
};

node_parameters
{
    AiParameterPtr("mask", NULL);
    AiParameterPtr("blend_image", NULL);
    AiParameterPtr("rend_image", NULL);
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

bool comp(float a, float b)
{
    return (a < b);
}

// TODO
shader_evaluate
{
    using namespace cv;

    Mat mask = *(Mat *)AiShaderEvalParamPtr(p_mask);
    Mat blend_image = *(Mat *)AiShaderEvalParamPtr(p_blend_image);
    Mat rend_image = *(Mat *)AiShaderEvalParamPtr(p_rend_image);

    // std::cout << blend_image.cols << "\t" << rend_image.cols << "\t" << blend_image.rows << "\t" << rend_image.rows << std::endl;
    // exit(-1);
    AtRGB rgba;

    // Pixel X/Y
    int x = sg->x;
    int y = sg->y;

    // std::cout << (unsigned int)mask.at<char>(y,x) <<  "\n";
    Vec3b p = blend_image.at<Vec3b>(y, x);
    Vec3b r = rend_image.at<Vec3b>(y, x);
    float avg = 0;

    AtRGB bodyC = AiShaderEvalParamRGB(p_Kd_bcolor);

    if(false && (float)r[0] <= 0 && (float)r[1] <= 0 && (float)r[2] <= 0){
        if (!AiShaderEvalParamBool(p_force_scene))
        {
            int ctr = 0;
            for (int i = -3; i < 4; i++)
            {
                for (int j = -3; j < 4; j++)
                {
                    int y_ = y + i;
                    int x_ = x + j;

                    if(x_ >= 0 && y_ >= 0 && x_ < blend_image.cols && y_ < blend_image.rows){
                        Vec3b curr = blend_image.at<Vec3b>(y_, x_);
                        avg += curr[0] + curr[1] + curr[2];
                        ctr += 1;
                    }
                }
            }
            if(ctr > 0){
                avg = avg / (1.2f * ctr * 255);
            }
        }
        if (avg > 1.3)
            avg = 1.3;
        if (avg < 0.35)
            avg = 0.35;


        /*float mask_sum = mask.at<uchar>(2 * y, 2 * x);
        mask_sum += mask.at<uchar>(2 * y + 1, 2 * x);
        mask_sum += mask.at<uchar>(2 * y, 2 * x + 1);
        mask_sum += mask.at<uchar>(2 * y + 1, 2 * x + 1);
        */
        // float mask_val = mask.at<uchar>(2 * y, 2 * x);
        // std::cout << mask_val << std::endl;
        // float body_ratio = float(mask_val > 0);
        float mask_sum = 0;
        int ctr = 0;

        for (int i = -1; i < 2; i++)
        {
            for (int j = -1; j < 2; j++)
            {
                int y_ = i + y * 2;
                int x_ = j + x * 2;

                if(x_ >= 0 && y_ >= 0 && x_ < mask.cols && y_ < mask.rows){
                    mask_sum += float(mask.at<uchar>(y_, x_) > 0);
                    ++ctr;
                }
            }
        }
        float body_ratio = 0;
        if(ctr > 0){
            body_ratio = mask_sum / ctr;
        }

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

        //blend scene with rgb
        rgba.r = bodyC.r * body_ratio * 1.5 * avg + (((float)p[2]) / 255.0) * (1.0 - body_ratio);
        rgba.g = bodyC.g * body_ratio * 1.5 * avg + (((float)p[1]) / 255.0) * (1.0 - body_ratio);
        rgba.b = bodyC.b * body_ratio * 1.5 * avg + (((float)p[0]) / 255.0) * (1.0 - body_ratio);
    }
    else{
        if(float(mask.at<uchar>(2 * y, 2 * x)) > 0){
            // float l_body = std::max(bodyC.r, bodyC.g, bodyC.b) + std::min(bodyC.r, bodyC.g, bodyC.b) / 2.f;
            float l_diff = 0;
            int ctr = 0;
            // light diff averaging from neighbors
            /*for (int i = -1; i < 2; i++)
            {
                for (int j = -1; j < 2; j++)
                {
                    int y_ = i + y;
                    int x_ = j + x;
                    if(x_ >= 0 && y_ >= 0 && x_ < rend_image.cols && y_ < rend_image.rows){
                        Vec3b r_ = rend_image.at<Vec3b>(y_, x_);
                        Vec3b p_ = blend_image.at<Vec3b>(y_, x_);

                        float l_rend = (std::max({r_[0], r_[1], r_[2]}, comp) + std::min({r_[0], r_[1], r_[2]}, comp)) / 2.f;
                        float l_scene = (std::max({p_[0], p_[1], p_[2]}, comp) + std::min({p_[0], p_[1], p_[2]}, comp)) / 2.f;
                        l_diff += (l_rend - l_scene) / 255.f;
                        ++ctr;
                    }
                }
            }
            l_diff = l_diff / ctr;
            */
            rgba.r = (bodyC.r + l_diff);
            rgba.g = (bodyC.g + l_diff);
            rgba.b = (bodyC.b + l_diff);
        }
        else{
            rgba.r = (((float)p[2]) / 255.0);
            rgba.g = (((float)p[1]) / 255.0);
            rgba.b = (((float)p[0]) / 255.0);
        }
    }
    sg->out.RGB = rgba;
}
