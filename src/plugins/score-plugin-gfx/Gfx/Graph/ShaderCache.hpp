#pragma once
#include <score/gfx/OpenGL.hpp>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QShaderBaker>
#else
#include <QtShaderTools/private/qshaderbaker_p.h>
#endif

#include <unordered_map>

namespace score::gfx
{
/**
 * @brief Cache of baked QShader instances
 */
struct ShaderCache
{
public:
  /**
   * @brief Get a QShader from a source string.
   *
   * @return If there is an error message, it will be in the QString part of the pair.
   */
  static const std::pair<QShader, QString>&
  get(const QByteArray& shader, QShader::Stage stage)
  {
    static ShaderCache self;
    if (auto it = self.shaders.find(shader); it != self.shaders.end())
      return it->second;

    self.baker.setSourceString(shader, stage);
    auto res = self.shaders.insert(
        {shader, {self.baker.bake(), self.baker.errorMessage()}});
    return res.first->second;
  }

private:
  score::GLCapabilities m_caps;
  ShaderCache()
  {
    QShaderVersion::Flags glFlag = m_caps.type == QSurfaceFormat::OpenGLES
                                       ? QShaderVersion::GlslEs
                                       : QShaderVersion::Flag{};
    baker.setGeneratedShaders({
      {QShader::SpirvShader, 100},
          {QShader::GlslShader, QShaderVersion(m_caps.shaderVersion, glFlag)},
#if defined(_WIN32)
          {QShader::HlslShader, QShaderVersion(50)},
#endif
#if defined(__APPLE__)
          {QShader::MslShader, QShaderVersion(12)},
#endif
    });

    baker.setGeneratedShaderVariants({
      QShader::Variant{}, QShader::Variant{},
#if defined(_WIN32)
          QShader::Variant{},
#endif
#if defined(__APPLE__)
          QShader::Variant{},
#endif
    });
  }

  QShaderBaker baker;
  std::unordered_map<QByteArray, std::pair<QShader, QString>> shaders;
};
}
