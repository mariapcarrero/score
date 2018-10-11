#pragma once
#include <Library/LibraryInterface.hpp>
#include <Device/Loading/IScoreDeviceLoader.hpp>
#include <Device/Loading/JamomaDeviceLoader.hpp>
#include <Explorer/Explorer/Widgets/DeviceEditDialog.hpp>
#include <Device/Protocol/ProtocolList.hpp>
#include <score/document/DocumentContext.hpp>
#include <ossia-qt/js_utilities.hpp>
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <score/command/Dispatchers/CommandDispatcher.hpp>
#include <Explorer/Commands/Add/AddDevice.hpp>
#include <Explorer/Commands/Add/LoadDevice.hpp>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QFileInfo>
#include <QQmlProperty>

#include <Protocols/Mapper/MapperDevice.hpp>
#if defined(OSSIA_PROTOCOL_OSC)
#include <Protocols/OSC/OSCProtocolFactory.hpp>
#endif
#if defined(OSSIA_PROTOCOL_OSCQUERY)
#include <Protocols/Minuit/MinuitProtocolFactory.hpp>
#endif
#if defined(OSSIA_PROTOCOL_MINUIT)
#include <Protocols/OSCQuery/OSCQueryProtocolFactory.hpp>
#endif
#if defined(OSSIA_PROTOCOL_HTTP)
#include <Protocols/HTTP/HTTPProtocolFactory.hpp>
#include <Protocols/HTTP/HTTPSpecificSettings.hpp>
#include <ossia-qt/http/http_protocol.hpp>
#endif
#if defined(OSSIA_PROTOCOL_WEBSOCKETS)
#include <Protocols/WS/WSProtocolFactory.hpp>
#include <Protocols/WS/WSSpecificSettings.hpp>
#include <ossia-qt/websocket-generic-client/ws_generic_client_protocol.hpp>
#endif
#if defined(OSSIA_PROTOCOL_SERIAL)
#include <Protocols/Serial/SerialProtocolFactory.hpp>
#include <Protocols/Serial/SerialSpecificSettings.hpp>
#include <ossia-qt/serial/serial_protocol.hpp>
#endif
namespace Protocols
{
class OSCLibraryHandler final
    : public Library::LibraryInterface
{
  SCORE_CONCRETE("8d4c06e2-851b-4d5f-82f2-68056a50c370")

  QSet<QString> acceptedFiles() const noexcept override
  {
    return {"json", "xml", "device"};
  }

  bool onDoubleClick(const QString& path, const score::DocumentContext& ctx) override
  {
    Device::Node n{Device::DeviceSettings{}, nullptr};
    bool ok = (path.endsWith(".json") && Device::loadDeviceFromIScoreJSON(path, n)) ||
              (path.endsWith(".xml") && Device::loadDeviceFromXML(path, n)) ||
              (path.endsWith(".device") && Device::loadDeviceFromJamomaJSON(path, n));
    if(!ok)
      return false;

    Device::ProtocolFactoryList fact;
#if defined(OSSIA_PROTOCOL_OSC)
    fact.insert(std::make_unique<Engine::Network::OSCProtocolFactory>());
#endif
#if defined(OSSIA_PROTOCOL_MINUIT)
    fact.insert(std::make_unique<Engine::Network::MinuitProtocolFactory>());
#endif
#if defined(OSSIA_PROTOCOL_OSCQUERY)
    fact.insert(std::make_unique<Engine::Network::OSCQueryProtocolFactory>());
#endif

    Device::DeviceSettings set;
    set.name = QFileInfo(path).baseName();
    set.protocol = Engine::Network::OSCProtocolFactory::static_concreteKey();
    auto dialog = std::make_unique<Explorer::DeviceEditDialog>(fact, nullptr);
    dialog->setSettings(set);
    auto code = static_cast<QDialog::DialogCode>(dialog->exec());
    if (code == QDialog::Accepted)
    {
      auto& devplug = ctx.plugin<Explorer::DeviceDocumentPlugin>();
      auto& model = devplug.explorer();

      auto deviceSettings = dialog->getSettings();
      if (!model.checkDeviceInstantiatable(deviceSettings))
      {
        if (!model.tryDeviceInstantiation(deviceSettings, *dialog))
        {
          return true;
        }
      }
      ossia::net::sanitize_device_name(deviceSettings.name);
      n.get<Device::DeviceSettings>() = deviceSettings;

      CommandDispatcher<>{ctx.commandStack}.submit(
          new Explorer::Command::LoadDevice{devplug, std::move(n)});
    }
    return true;
  }
};


class QMLLibraryHandler final
    : public Library::LibraryInterface
{
  SCORE_CONCRETE("fee42cea-ff1a-48ef-a0da-922773081779")

  QSet<QString> acceptedFiles() const noexcept override
  {
    return {"qml"};
  }

  bool onDoubleClick(const QString& path, const score::DocumentContext& ctx) override
  {
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly))
      return true;
    auto content = f.readAll();

    QQmlEngine e;
    QQmlComponent c{&e};
    c.setData(content, QUrl());

    std::unique_ptr<QObject> obj{c.create()};

    if(!obj)
      return true;

    Device::ProtocolFactoryList fact;
    Device::DeviceSettings set;
    set.name = QFileInfo(f).baseName();
    if (dynamic_cast<Protocols::Mapper*>(obj.get()))
    {
      fact.insert(std::make_unique<Protocols::MapperProtocolFactory>());
      set.protocol = Protocols::MapperProtocolFactory::static_concreteKey();
      set.deviceSpecificSettings = QVariant::fromValue(Protocols::MapperSpecificSettings{content});
    }
#if defined(OSSIA_PROTOCOL_SERIAL)
    else if(dynamic_cast<ossia::net::Serial*>(obj.get()))
    {
      fact.insert(std::make_unique<Engine::Network::SerialProtocolFactory>());
      set.protocol = Engine::Network::SerialProtocolFactory::static_concreteKey();
      set.deviceSpecificSettings = QVariant::fromValue(Engine::Network::SerialSpecificSettings{{}, content});
    }
#endif
#if defined(OSSIA_PROTOCOL_HTTP)
    else if(dynamic_cast<ossia::net::HTTP*>(obj.get()))
    {
      fact.insert(std::make_unique<Engine::Network::HTTPProtocolFactory>());
      set.protocol = Engine::Network::HTTPProtocolFactory::static_concreteKey();
      set.deviceSpecificSettings = QVariant::fromValue(Engine::Network::HTTPSpecificSettings{content});
    }
#endif
#if defined(OSSIA_PROTOCOL_WEBSOCKETS)
    else if(dynamic_cast<ossia::net::WS*>(obj.get()))
    {
      fact.insert(std::make_unique<Engine::Network::WSProtocolFactory>());
      set.protocol = Engine::Network::WSProtocolFactory::static_concreteKey();
      set.deviceSpecificSettings = QVariant::fromValue(Engine::Network::WSSpecificSettings{QQmlProperty(obj.get(), "host").read().toString(), content});
    }
#endif

    if(set.protocol == UuidKey<Device::ProtocolFactory>{})
      return false;

    auto dialog = std::make_unique<Explorer::DeviceEditDialog>(fact, nullptr);
    dialog->setSettings(set);
    auto code = static_cast<QDialog::DialogCode>(dialog->exec());
    if (code == QDialog::Accepted)
    {
      auto& devplug = ctx.plugin<Explorer::DeviceDocumentPlugin>();
      auto& model = devplug.explorer();

      auto deviceSettings = dialog->getSettings();
      if (!model.checkDeviceInstantiatable(deviceSettings))
      {
        if (!model.tryDeviceInstantiation(deviceSettings, *dialog))
        {
          return true;
        }
      }
      ossia::net::sanitize_device_name(deviceSettings.name);

      CommandDispatcher<>{ctx.commandStack}.submit(
          new Explorer::Command::AddDevice{devplug, deviceSettings});
    }
    return true;
  }
};
}
