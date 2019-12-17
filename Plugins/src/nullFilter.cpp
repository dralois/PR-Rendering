#ifndef ai_common_mtds
#include <ai.h>
#endif
#include <string.h>
#include <iostream>

AI_FILTER_NODE_EXPORT_METHODS(CustomNullFilterMtd);

static char* WIDTH("width");

node_parameters
{
   AiParameterFlt("width", 2.0f);
}

node_initialize
{
   AiFilterInitialize(node, true, NULL, NULL);
}

node_update
{
   AiFilterUpdate(node, AiNodeGetFlt(node, WIDTH));
}

node_finish
{
   AiFilterDestroy(node);
}

filter_output_type
{
   switch (input_type)
   {
   case AI_TYPE_RGBA:
      return AI_TYPE_INT;
   default:
      return AI_TYPE_NONE;
   }
}

filter_pixel
{
   const float width = 1;

   float aweight = 0.0f;
   float avalue = 0.0f;

   while (AiAOVSampleIteratorGetNext(iterator))
   {
      avalue += AiAOVSampleIteratorGetRGBA(iterator).r;
      aweight += 1;
   }

   if (aweight != 0.0f)
      avalue /= aweight;
   *((int *)data_out) = avalue;
}