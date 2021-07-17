#include <Gfx/GfxContext.hpp>
#include <Gfx/Graph/Graph.hpp>
#include <Gfx/Settings/Model.hpp>

#include <score/application/ApplicationContext.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/tools/Bind.hpp>

#include <ossia/detail/flicks.hpp>
#include <ossia/detail/logger.hpp>

#include <QGuiApplication>
namespace Gfx
{

GfxContext::GfxContext(const score::DocumentContext& ctx)
    : m_context{ctx}
{
  new_edges.container.reserve(100);
  edges.container.reserve(100);

  auto& settings = m_context.app.settings<Gfx::Settings::Model>();
  con(settings,
      &Gfx::Settings::Model::GraphicsApiChanged,
      this,
      &GfxContext::recompute_graph);
  con(settings,
      &Gfx::Settings::Model::RateChanged,
      this,
      &GfxContext::recompute_graph);
  con(settings,
      &Gfx::Settings::Model::VSyncChanged,
      this,
      &GfxContext::recompute_graph);

  m_graph = new score::gfx::Graph;

  double rate = m_context.app.settings<Gfx::Settings::Model>().getRate();
  rate = 1000. / qBound(1.0, rate, 1000.);

  QMetaObject::invokeMethod(
      this,
      [this, rate] { m_timer = startTimer(rate); },
      Qt::QueuedConnection);
}

GfxContext::~GfxContext()
{
#if defined(SCORE_THREADED_GFX)
  if (m_thread.isRunning())
    m_thread.exit(0);
  m_thread.wait();
#endif

  delete m_graph;
}

int32_t GfxContext::register_node(
    std::unique_ptr<score::gfx::Node> node)
{
  auto next = index++;

  tick_commands.enqueue(Command{Command::ADD_NODE, next, std::move(node)});

  return next;
}

void GfxContext::unregister_node(int32_t idx)
{
  tick_commands.enqueue(Command{Command::REMOVE_NODE, idx, {}});
}

void GfxContext::recompute_edges()
{
  m_graph->clearEdges();

  for (auto edge : edges)
  {
    auto source_node_it = this->nodes.find(edge.first.node);
    if(source_node_it != this->nodes.end())
    {
    auto sink_node_it = this->nodes.find(edge.second.node);
    if(sink_node_it != this->nodes.end())
    {
      assert(source_node_it->second);
      assert(sink_node_it->second);

      auto source_port = source_node_it->second->output[edge.first.port];
      auto sink_port = sink_node_it->second->input[edge.second.port];

      m_graph->addEdge(source_port, sink_port);
    }
    }
  }
}

void GfxContext::recompute_graph()
{
  if (m_timer != -1)
    killTimer(m_timer);
  m_timer = -1;
  m_graph->setVSyncCallback({});

  recompute_edges();

  auto& settings = m_context.app.settings<Gfx::Settings::Model>();
  auto api = settings.graphicsApiEnum();

  m_graph->createAllRenderLists(api);

  const bool vsync = settings.getVSync() && m_graph->canDoVSync();

  // rate in fps
  double rate = m_context.app.settings<Gfx::Settings::Model>().getRate();
  rate = 1000. / qBound(1.0, rate, 1000.);

  if (vsync)
  {
#if defined(SCORE_THREADED_GFX)
    if (api == Vulkan)
    {
      //:startTimer(rate);
      moveToThread(&m_thread);
      m_thread.start();
    }
#endif
    m_graph->setVSyncCallback([this] { updateGraph(); });
  }
  else
  {
    QMetaObject::invokeMethod(
        this,
        [this, rate] { m_timer = startTimer(rate); },
        Qt::QueuedConnection);
  }
}

void GfxContext::recompute_connections()
{
  recompute_graph();
  // FIXME for more performance
  /*
  recompute_edges();
  // m_graph->setupOutputs(m_api);
  m_graph->relinkGraph();
  */
}

void GfxContext::update_inputs()
{
  score::gfx::Message msg;
  while (tick_messages.try_dequeue(msg))
  {
    if (auto it = nodes.find(msg.node_id); it != nodes.end())
    {
      auto& node = it->second;
      node->process(msg);
    }
  }
}

void GfxContext::run_commands()
{
  std::vector<std::unique_ptr<score::gfx::Node>> nursery;

  bool recompute = false;
  Command cmd;
  while(tick_commands.try_dequeue(cmd))
  {
    switch(cmd.cmd)
    {
      case Command::ADD_NODE:
      {
        m_graph->addNode(cmd.node.get());
        nodes[cmd.index] = {std::move(cmd.node)};

        recompute = true;
        break;
      }
      case Command::REMOVE_NODE:
      {
        // Remove all edges involving that node
        for (auto it = this->edges.begin(); it != this->edges.end();)
        {
          if (it->first.node == cmd.index || it->second.node == cmd.index)
            it = this->edges.erase(it);
          else
            ++it;
        }

        auto it = nodes.find(cmd.index);
        if (it != nodes.end())
        {
          m_graph->removeNode(it->second.get());

          // Needed because when removing edges in recompute_graph,
          // they remove themselves from the nodes / ports in their dtor
          // thus if the items are deleted before that, it would crash
          nursery.push_back(std::move(it->second));

          nodes.erase(it);
        }
        recompute = true;
        break;
      }
      case Command::RELINK:
      {
        recompute = true;
        break;
      }
    }

  }

  if(recompute)
  {
    recompute_graph();
  }

  nursery.clear();
}
void GfxContext::updateGraph()
{
  run_commands();

  update_inputs();

  if (edges_changed)
  {
    {
      std::lock_guard l{edges_lock};
      std::swap(edges, new_edges);
    }
    recompute_connections();
    edges_changed = false;
  }
}

void GfxContext::timerEvent(QTimerEvent*)
{
  updateGraph();
}


}
