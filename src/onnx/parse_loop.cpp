#include "migraphx/errors.hpp"
#include "migraphx/instruction_ref.hpp"
#include "migraphx/iterator_for.hpp"
#include <migraphx/onnx/op_parser.hpp>
#include <migraphx/onnx/onnx_parser.hpp>
#include <migraphx/onnx/checks.hpp>
#include <migraphx/ranges.hpp>
#include <migraphx/instruction.hpp>
#include <migraphx/make_op.hpp>

namespace migraphx {
inline namespace MIGRAPHX_INLINE_NS {
namespace onnx {

static void add_parameter_prefix(module& mod, const std::string& name, const std::string& prefix)
{
    auto ins = mod.get_parameter(name);
    if(ins == mod.end())
    {
        MIGRAPHX_THROW("PARSE_LOOP: parameter \"" + name + "\" does not exist for module \"" +
                       mod.name());
    }
    auto s               = ins->get_shape();
    std::string mgx_name = "@mgx_" + mod.name() + prefix + name;
    auto mgx_ins         = mod.add_parameter(mgx_name, s);
    mod.replace_instruction(ins, mgx_ins);
    mod.remove_instruction(ins);
}

struct parse_loop : op_parser<parse_loop>
{
    std::vector<op_desc> operators() const { return {{"Loop"}}; }

    std::vector<instruction_ref> parse(const op_desc& /*opd*/,
                                       onnx_parser& parser,
                                       const onnx_parser::node_info& info,
                                       std::vector<instruction_ref> args) const
    {
        // if(not(args.at(0)->get_shape().scalar() and args.at(1)->get_shape().scalar()))
        // {
        //     MIGRAPHX_THROW("PARSE_LOOP: max_iter_num and cond inputs must be scalar!");
        // }

        // default value of the max_iter_num
        int64_t max_iter_num = parser.max_iter_num;
        auto arg_iters       = args.at(0)->eval();
        if(not arg_iters.empty())
        {
            max_iter_num = arg_iters.at<int64_t>();
        }

        // retrieve the subgraph
        const auto& sub_graph = info.attributes.at("body").g();
        std::string mod_name  = info.name + "_loop";
        module_ref sub_mod    = parser.prog.create_module(mod_name);

        // parse the sub_graph
        parser.parse_graph(sub_mod, sub_graph, info.instructions);

        auto pnames = sub_mod->get_parameter_names();
        // add prefix for the iter_no
        add_parameter_prefix(*sub_mod, pnames.at(0), "_iter_");
        add_parameter_prefix(*sub_mod, pnames.at(1), "_cond_");

        auto ret = info.add_instruction(
            make_op("loop", {{"max_iter_num", max_iter_num}}), args, {sub_mod});
        auto out_s = ret->get_shape();
        assert(out_s.type() == shape::tuple_type);

        const auto& vec_shapes = out_s.sub_shapes();
        std::vector<instruction_ref> out_inss;
        for(std::size_t i = 0; i < vec_shapes.size(); ++i)
        {
            auto r = info.add_instruction(make_op("get_tuple_elem", {{"index", i}}), ret);
            out_inss.push_back(r);
        }

        return out_inss;
    }
};

} // namespace onnx
} // namespace MIGRAPHX_INLINE_NS
} // namespace migraphx