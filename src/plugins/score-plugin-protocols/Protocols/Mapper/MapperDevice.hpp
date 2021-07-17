#pragma once

#if __has_include(<QQmlEngine>)
#include <Device/Protocol/DeviceInterface.hpp>
#include <Device/Protocol/DeviceSettings.hpp>
#include <Device/Protocol/ProtocolSettingsWidget.hpp>
#include <Explorer/DefaultProtocolFactory.hpp>

#include <verdigris>

inline QDataStream&
operator<<(QDataStream& st, const std::vector<ossia::net::node_base*>& p)
{
  return st;
}
inline QDataStream&
operator>>(QDataStream& st, std::vector<ossia::net::node_base*>& p)
{
  return st;
}
Q_DECLARE_METATYPE(std::vector<ossia::net::node_base*>)
W_REGISTER_ARGTYPE(std::vector<ossia::net::node_base*>)
class QCodeEditor;
namespace Protocols
{
class Mapper : public QObject
{
};

struct MapperSpecificSettings
{
  QString text;
};

class MapperProtocolFactory final : public Protocols::DefaultProtocolFactory
{
  SCORE_CONCRETE("910e2d87-a087-430d-b725-c988fe2bea01")

public:
  ~MapperProtocolFactory();

private:
  QString prettyName() const noexcept override;
  QString category() const noexcept override;
  Device::DeviceEnumerator*
  getEnumerator(const score::DocumentContext& ctx) const override;

  Device::DeviceInterface* makeDevice(
      const Device::DeviceSettings& settings,
      const Explorer::DeviceDocumentPlugin& plugin,
      const score::DocumentContext& ctx) override;

  const Device::DeviceSettings& defaultSettings() const noexcept override;

  Device::ProtocolSettingsWidget* makeSettingsWidget() override;

  QVariant
  makeProtocolSpecificSettings(const VisitorVariant& visitor) const override;

  void serializeProtocolSpecificSettings(
      const QVariant& data,
      const VisitorVariant& visitor) const override;

  bool checkCompatibility(
      const Device::DeviceSettings& a,
      const Device::DeviceSettings& b) const noexcept override;
};

class MapperProtocolSettingsWidget : public Device::ProtocolSettingsWidget
{
public:
  MapperProtocolSettingsWidget(QWidget* parent = nullptr);

  Device::DeviceSettings getSettings() const override;

  void setSettings(const Device::DeviceSettings& settings) override;

protected:
  void setDefaults();

protected:
  QLineEdit* m_name{};
  QCodeEditor* m_codeEdit{};
};
}

Q_DECLARE_METATYPE(Protocols::MapperSpecificSettings)
W_REGISTER_ARGTYPE(Protocols::MapperSpecificSettings)
#endif
