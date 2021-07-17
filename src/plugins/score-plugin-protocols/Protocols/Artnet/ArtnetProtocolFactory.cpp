#include <ossia/detail/config.hpp>
#if defined(OSSIA_PROTOCOL_ARTNET)
#include "ArtnetDevice.hpp"
#include "ArtnetProtocolFactory.hpp"
#include "ArtnetProtocolSettingsWidget.hpp"
#include "ArtnetSpecificSettings.hpp"

#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <State/Widgets/AddressFragmentLineEdit.hpp>

#include <score/application/ApplicationContext.hpp>
#include <score/widgets/SignalUtils.hpp>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QObject>

namespace Protocols
{

QString ArtnetProtocolFactory::prettyName() const noexcept
{
  return QObject::tr("Artnet");
}

QString ArtnetProtocolFactory::category() const noexcept
{
  return StandardCategories::lights;
}

Device::DeviceEnumerator*
ArtnetProtocolFactory::getEnumerator(const score::DocumentContext& ctx) const
{
  return nullptr;
}

Device::DeviceInterface* ArtnetProtocolFactory::makeDevice(
    const Device::DeviceSettings& settings,
    const Explorer::DeviceDocumentPlugin& plugin,
    const score::DocumentContext& ctx)
{
  return new ArtnetDevice{settings, plugin.networkContext()};
}

const Device::DeviceSettings&
ArtnetProtocolFactory::defaultSettings() const noexcept
{
  static const Device::DeviceSettings& settings = [&]() {
    Device::DeviceSettings s;
    s.protocol = concreteKey();
    s.name = "Artnet";
    ArtnetSpecificSettings settings;
    s.deviceSpecificSettings = QVariant::fromValue(settings);
    return s;
  }();

  return settings;
}

Device::ProtocolSettingsWidget* ArtnetProtocolFactory::makeSettingsWidget()
{
  return new ArtnetProtocolSettingsWidget;
}

QVariant ArtnetProtocolFactory::makeProtocolSpecificSettings(
    const VisitorVariant& visitor) const
{
  return makeProtocolSpecificSettings_T<ArtnetSpecificSettings>(visitor);
}

void ArtnetProtocolFactory::serializeProtocolSpecificSettings(
    const QVariant& data,
    const VisitorVariant& visitor) const
{
  serializeProtocolSpecificSettings_T<ArtnetSpecificSettings>(data, visitor);
}

bool ArtnetProtocolFactory::checkCompatibility(
    const Device::DeviceSettings& a,
    const Device::DeviceSettings& b) const noexcept
{
  return false; //  TODO
}
}
#endif
