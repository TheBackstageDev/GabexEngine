#pragma once

#include "stb/std_image.hpp"

#include <string>

namespace GWIN
{
    struct Image
    {
        unsigned short height, width, channels;
    };

    class GWImageLoader
    {
    public:
        GWImageLoader();
        ~GWImageLoader();


    private:
    };
}