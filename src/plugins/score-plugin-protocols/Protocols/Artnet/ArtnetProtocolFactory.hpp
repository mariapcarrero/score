#pragma once
#include <ossia/detail/config.hpp>
#if defined(OSSIA_PROTOCOL_ARTNET)
#include <Explorer/DefaultProtocolFactory.hpp>

namespace Protocols
{

class ArtnetProtocolFactory final : public DefaultProtocolFactory
{
  SCORE_CONCRETE("1c199b75-8052-4d5b-9f85-1b2b0d7e26a9")

  QString prettyName() const noexcept override;
  QString category() const noexcept override;
  Device::DeviceEnumerator*
  getEnumerator(const score::DocumentContext& ctx) const override;

  Device::DeviceInterface* makeDevice(
      const Device::DeviceSettings& settings,
      const Explorer::DeviceDocumentPlugin& plug,
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
}
#endif
