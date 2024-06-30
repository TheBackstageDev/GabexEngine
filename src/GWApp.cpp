#include "GWApp.hpp"

namespace GWIN
{
    void GWapp::run()
    {
        renderSystem.run();
        vkDeviceWaitIdle(GDevice.device());
    }
}