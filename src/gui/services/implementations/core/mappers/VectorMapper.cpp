#include "VectorMapper.h"

namespace gui::coremappers {

core::Vec3 toCore(Vec3 value)
{
    return {static_cast<double>(value.x), static_cast<double>(value.y), static_cast<double>(value.z)};
}

Vec3 toGui(core::Vec3 value)
{
    return {static_cast<float>(value.x), static_cast<float>(value.y), static_cast<float>(value.z)};
}

}  // namespace gui::coremappers
