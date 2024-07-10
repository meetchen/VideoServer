#include "live/base/CodecUtils.h"


using namespace vdse::live;


bool CodecUtils::IsCodecHeader(const PacketPtr &packet)
{
    if (packet->PacketSize() > 1)
    {
        auto p = packet->Data() + 1;
        if (*p == '0')
        {
            return true;
        }
    }
    return false;
}
