#include "Executor.hpp"

#include <Gfx/Filter/Process.hpp>
#include <Gfx/GfxApplicationPlugin.hpp>
#include <Gfx/GfxContext.hpp>
#include <Gfx/GfxExec.hpp>
#include <Gfx/Graph/ISFNode.hpp>
#include <Gfx/TexturePort.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/ExecutionContext.hpp>
#include <Process/ExecutionSetup.hpp>

#include <score/document/DocumentContext.hpp>

#include <ossia/dataflow/port.hpp>
namespace Gfx::Filter
{
class filter_node final : public gfx_exec_node
{
public:
  filter_node(
      const isf::descriptor& isf,
      const QShader& vert,
      const QShader& frag,
      GfxExecutionAction& ctx)
      : gfx_exec_node{ctx}
  {
    auto n = std::make_unique<score::gfx::ISFNode>(isf, vert, frag);

    id = exec_context->ui->register_node(std::move(n));
  }

  void set_script(
      const isf::descriptor& isf,
      const QShader& vert,
      const QShader& frag)
  {
    exec_context->ui->unregister_node(id);

    auto n = std::make_unique<score::gfx::ISFNode>(isf, vert, frag);

    id = exec_context->ui->register_node(std::move(n));
  }

  ~filter_node() { exec_context->ui->unregister_node(id); }

  std::string label() const noexcept override { return "Gfx::filter_node"; }
};

ProcessExecutorComponent::ProcessExecutorComponent(
    Gfx::Filter::Model& element,
    const Execution::Context& ctx,
    QObject* parent)
    : ProcessComponent_T{element, ctx, "gfxExecutorComponent", parent}
{
  try
  {
    const auto& shader = element.processedProgram();
    const auto& desc = shader.descriptor;

    this->node = std::make_shared<filter_node>(
        desc,
        shader.compiledVertex,
        shader.compiledFragment,
        ctx.doc.plugin<DocumentPlugin>().exec);

    Execution::Transaction commands{system()};
    setup_node(commands);
    commands.run_all_in_exec();

    m_ossia_process = std::make_shared<ossia::node_process>(this->node);

    m_oldInlets = process().inlets();
    m_oldOutlets = process().outlets();

    connect(&element, &Filter::Model::programChanged, this, &ProcessExecutorComponent::on_shaderChanged);
  }
  catch (...)
  {
  }
}

void ProcessExecutorComponent::on_shaderChanged()
{
  auto& setup = system().setup;
  auto& element = this->process();
  Execution::Transaction commands{system()};
  auto n = std::dynamic_pointer_cast<filter_node>(this->node);

  // 0. Unregister all the previous inlets / outlets
  setup.unregister_node_soft(m_oldInlets, m_oldOutlets, node);

  // 1. Recreate ports
  auto [inls, outls] = setup_node(commands);

  // 2. Change the script
  const auto& shader = element.processedProgram();
  commands.push_back([n,
                      shader = std::make_unique<ProcessedProgram>(shader)] { n->set_script(shader->descriptor, shader->compiledVertex, shader->compiledFragment); });

  // 3. Register the inlets / outlets
  for (std::size_t i = 0; i < inls.size(); i++)
  {
    setup.register_inlet(*process().inlets()[i], inls[i], node, commands);
  }
  for (std::size_t i = 0; i < outls.size(); i++)
  {
    setup.register_outlet(*process().outlets()[i], outls[i], node, commands);
  }

  commands.run_all();

  m_oldInlets = process().inlets();
  m_oldOutlets = process().outlets();
}

std::pair<ossia::inlets, ossia::outlets> ProcessExecutorComponent::setup_node(Execution::Transaction& commands)
{
  const Execution::Context& ctx = system();
  auto& element = this->process();

  auto n = std::dynamic_pointer_cast<filter_node>(this->node);
  int script_index = ++n->script_index;


  // 1. Create new inlet & outlet arrays
  ossia::inlets inls;
  ossia::outlets outls;

  std::vector<std::shared_ptr<gfx_exec_node::control>> controls;
  std::vector<Execution::ExecutionCommand> controlSetups;

  std::size_t control_index = 0;
  std::weak_ptr<gfx_exec_node> weak_node = n;
  for (auto& ctl : element.inlets())
  {
    if (auto ctrl = qobject_cast<Process::ControlInlet*>(ctl))
    {
      auto inletport = new ossia::value_inlet;
      auto control = std::make_shared<gfx_exec_node::control>();
      control->value = ctrl->value();
      control->port = &**inletport;
      control->changed = true;
      controls.push_back(control);

      inls.push_back(inletport);
      ctl->setupExecution(*inletport);

      // TODO assert that we aren't going to connect twice
      QObject::disconnect(ctrl, nullptr, this, nullptr);
      QObject::connect(
          ctrl,
          &Process::ControlInlet::valueChanged,
          this,
          con_unvalidated{ctx, control_index, script_index, weak_node});
      control_index++;
    }
    else if (auto ctrl = qobject_cast<Process::AudioInlet*>(ctl))
    {
      inls.push_back(new ossia::audio_inlet);
    }
    else if (auto ctrl = qobject_cast<Gfx::TextureInlet*>(ctl))
    {
      inls.push_back(new ossia::texture_inlet);
    }
  }

  outls.push_back(new ossia::texture_outlet);
  commands.push_back([n, inls, outls, controls] () mutable {
                       for(auto& inl : n->root_inputs()) delete inl;
                       for(auto& outl : n->root_outputs()) delete outl;

                       using namespace std;
                       swap(n->root_inputs(), inls);
                       swap(n->root_outputs(), outls);
                       swap(n->controls, controls);
                     });

  return {std::move(inls), std::move(outls)};
}
}
