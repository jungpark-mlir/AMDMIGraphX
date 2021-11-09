#ifndef MIGRAPHX_GUARD_RTGLIB_GPU_MLIR_HPP
#define MIGRAPHX_GUARD_RTGLIB_GPU_MLIR_HPP

#include <string>
#include <migraphx/config.hpp>

namespace migraphx {
inline namespace MIGRAPHX_INLINE_NS {
struct module;
namespace gpu {

std::string dump_mlir(const module& m);

} // namespace gpu
} // namespace MIGRAPHX_INLINE_NS
} // namespace migraphx

#endif
