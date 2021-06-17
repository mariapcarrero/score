#pragma once

#include <Gfx/Graph/Node.hpp>
#include <ossia/detail/packed_struct.hpp>

namespace score::gfx
{
/**
 * @brief A node that renders an image to screen.
 */
struct ImagesNode : NodeModel
{
public:
  explicit ImagesNode(std::vector<score::gfx::Image> dec);
  virtual ~ImagesNode();

  const Mesh& mesh() const noexcept override;
  score::gfx::NodeRenderer*
  createRenderer(RenderList& r) const noexcept override;

  class Renderer;
  struct UBO
  {
    int currentImageIndex{};
    float opacity{1.};
    float position[2]{0.5, 0.5};
    float scale[2]{1., 1.};
  } ubo;

private:
  const TexturedTriangle& m_mesh = TexturedTriangle::instance();
  std::vector<score::gfx::Image> images;
};
struct FullScreenImageNode : NodeModel
{
public:
  explicit FullScreenImageNode(QImage dec);
  virtual ~FullScreenImageNode();

  const Mesh& mesh() const noexcept override;
  score::gfx::NodeRenderer*
  createRenderer(RenderList& r) const noexcept override;

  class Renderer;
private:
  const TexturedTriangle& m_mesh = TexturedTriangle::instance();
  QImage m_image;
};
}
