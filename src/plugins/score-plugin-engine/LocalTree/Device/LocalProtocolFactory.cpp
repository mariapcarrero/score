// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "LocalProtocolFactory.hpp"

#include "LocalDevice.hpp"
#include "LocalProtocolSettingsWidget.hpp"

#include <Device/Protocol/DeviceSettings.hpp>
#include <LocalTree/Device/LocalSpecificSettings.hpp>
#include <LocalTree/LocalTreeDocumentPlugin.hpp>

#include <QDebug>
#include <QObject>
namespace Device
{
class DeviceInterface;
class ProtocolSettingsWidget;
}
struct VisitorVariant;

namespace Protocols
{
QString LocalProtocolFactory::prettyName() const noexcept
{
  return QObject::tr("Local");
}

QString LocalProtocolFactory::category() const noexcept
{
  return StandardCategories::util;
}

Device::DeviceEnumerator*
LocalProtocolFactory::getEnumerator(const score::DocumentContext& ctx) const
{
  return nullptr;
}

Device::DeviceInterface* LocalProtocolFactory::makeDevice(
    const Device::DeviceSettings& settings,
    const Explorer::DeviceDocumentPlugin& plugin,
    const score::DocumentContext& ctx)
{
  qDebug() << "updating local" << settings.name;
  auto doc = ctx.findPlugin<LocalTree::DocumentPlugin>();
  if (doc)
  {
    doc->localDevice().updateSettings(settings);
    return &doc->localDevice();
  }
  else
    return nullptr;
}

const Device::DeviceSettings& LocalProtocolFactory::static_defaultSettings()
{
  static const Device::DeviceSettings settings = [&]() {
    Device::DeviceSettings s;
    s.protocol = static_concreteKey(); // Todo check for un-set protocol.
    s.name = "score";
    LocalSpecificSettings specif;
    specif.oscPort = 6666;
    specif.wsPort = 9999;
    s.deviceSpecificSettings = QVariant::fromValue(specif);
    return s;
  }();

  return settings;
}

const Device::DeviceSettings&
LocalProtocolFactory::defaultSettings() const noexcept
{
  return static_defaultSettings();
}

Device::ProtocolSettingsWidget* LocalProtocolFactory::makeSettingsWidget()
{
  return new LocalProtocolSettingsWidget;
}

QVariant LocalProtocolFactory::makeProtocolSpecificSettings(
    const VisitorVariant& visitor) const
{
  return makeProtocolSpecificSettings_T<LocalSpecificSettings>(visitor);
}

void LocalProtocolFactory::serializeProtocolSpecificSettings(
    const QVariant& data,
    const VisitorVariant& visitor) const
{
  serializeProtocolSpecificSettings_T<LocalSpecificSettings>(data, visitor);
}

bool LocalProtocolFactory::checkCompatibility(
    const Device::DeviceSettings& a,
    const Device::DeviceSettings& b) const noexcept
{
  return a.name != b.name;
}

}
