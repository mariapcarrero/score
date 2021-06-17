#pragma once
#include <Gfx/Graph/CommonUBOs.hpp>
#include <Gfx/Graph/Mesh.hpp>
#include <Gfx/Graph/RenderState.hpp>
#include <Gfx/Graph/Uniforms.hpp>
#include <Gfx/Graph/Utils.hpp>

#include <ossia/detail/flat_map.hpp>

#include <score_plugin_gfx_export.h>

#include <algorithm>
#include <optional>
#include <vector>

#include <unordered_map>

namespace score::gfx
{
class RenderList;
struct Graph;
class GenericNodeRenderer;
class NodeRenderer;

/**
 * @brief Root data model for visual nodes.
 */
class SCORE_PLUGIN_GFX_EXPORT Node
{
public:
  explicit Node();
  virtual ~Node();

  /**
   * @brief Create a renderer in a given context for this node.
   */
  virtual NodeRenderer* createRenderer(RenderList& r) const noexcept = 0;

  /**
   * @brief Mesh corresponding to this node.
   */
  virtual const Mesh& mesh() const noexcept = 0;

  /**
   * @brief Input ports of that node.
   */
  std::vector<Port*> input;
  /**
   * @brief Output ports of that node.
   *
   * Most of the time there will be a single image output.
   */
  ossia::small_pod_vector<Port*, 1> output;

  /**
   * @brief Map associating each RenderList to a Renderer for this model.
   */
  ossia::flat_map<RenderList*, score::gfx::NodeRenderer*> renderedNodes;

  bool addedToGraph{};
};

/**
 * @brief Common base class for nodes that map to score processes.
 */
class SCORE_PLUGIN_GFX_EXPORT ProcessNode : public Node
{
public:
  using Node::Node;

  /**
   * @brief Used to notify a material change from the model to the renderers.
   */
  std::atomic_int64_t materialChanged{0};

  /**
   * @brief Every node matching with a score process will have such an UBO.
   *
   * It has useful informations, such as timing, sample rate, mouse position etc.
   */
  ProcessUBO standardUBO{};
};

/**
 * @brief Common base class for most single-pass, simple nodes.
 */
class SCORE_PLUGIN_GFX_EXPORT NodeModel : public score::gfx::ProcessNode
{
  friend class GenericNodeRenderer;

public:
  explicit NodeModel();
  virtual ~NodeModel();

  virtual score::gfx::NodeRenderer*
  createRenderer(RenderList& r) const noexcept;

  void setShaders(const QShader& vert, const QShader& frag);

protected:
  std::unique_ptr<char[]> m_materialData;

  QShader m_vertexS;
  QShader m_fragmentS;

  friend class GenericNodeRenderer;
};
}
