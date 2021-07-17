#pragma once

#include <Gfx/GfxContext.hpp>
#include <Gfx/GfxDevice.hpp>
#include <Gfx/GfxExecContext.hpp>
#include <Process/ExecutionContext.hpp>
#include <State/ValueConversion.hpp>

#include <ossia/dataflow/graph_edge.hpp>
#include <ossia/dataflow/graph_node.hpp>
#include <ossia/dataflow/port.hpp>

#include <score_plugin_gfx_export.h>

namespace Gfx
{

template <typename Vector>
int64_t index_of(
    Vector&& v,
    const typename std::remove_reference_t<Vector>::value_type& t)
{
  if (auto it = ossia::find(v, t); it != v.end())
  {
    return std::distance(v.begin(), it);
  }
  return -1;
}

class SCORE_PLUGIN_GFX_EXPORT gfx_exec_node
    : public ossia::graph_node
{
public:
  struct control
  {
    ossia::value value{};
    ossia::value_port* port{};
    bool changed{};
  };
  std::vector<std::shared_ptr<control>> controls;
  GfxExecutionAction* exec_context{};
  gfx_exec_node(GfxExecutionAction& e_ctx)
      : exec_context{&e_ctx}
  {
  }

  const std::shared_ptr<control>& add_control()
  {
    auto inletport = new ossia::value_inlet;
    controls.push_back(std::make_shared<control>());
    controls.back()->port = &**inletport;
    m_inlets.push_back(inletport);
    return controls.back();
  }

  void add_texture()
  {
    auto inletport = new ossia::texture_inlet;
    m_inlets.push_back(inletport);
  }

  void add_audio()
  {
    auto inletport = new ossia::audio_inlet;
    m_inlets.push_back(inletport);
  }

  ~gfx_exec_node();

  int32_t id{-1};
  std::atomic_int32_t script_index{0};
  void run(const ossia::token_request& tk, ossia::exec_state_facade) noexcept;
};

struct control_updater
{
  std::shared_ptr<gfx_exec_node::control> ctrl;
  ossia::value v;

  void operator()() noexcept
  {
    ctrl->value = std::move(v);
    ctrl->changed = true;
  }
};

struct con_unvalidated
{
  const Execution::Context& ctx;
  const std::size_t i;
  const int32_t script_index{};
  std::weak_ptr<gfx_exec_node> weak_node;
  void operator()(const ossia::value& val)
  {
    if (auto node = weak_node.lock())
    {
      // Check for the case where the node controls have changed
      // due to the script changing
      if(script_index != node->script_index)
        return;
      SCORE_ASSERT(i < node->controls.size());
      ctx.executionQueue.enqueue(control_updater{node->controls[i], val});
    }
  }
};

}
