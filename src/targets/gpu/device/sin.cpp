#include <migraph/gpu/device/sin.hpp>
#include <migraph/gpu/device/nary.hpp>

namespace migraph {
inline namespace MIGRAPH_INLINE_NS {
namespace gpu {
namespace device {

void sin(hipStream_t stream, const argument& result, const argument& arg)
{
    nary(stream, result, arg)([](auto x) { return sinf(x); });
}

} // namespace device
} // namespace gpu
} // namespace MIGRAPH_INLINE_NS
} // namespace migraph
