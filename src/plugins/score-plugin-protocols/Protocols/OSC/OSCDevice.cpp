// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "OSCDevice.hpp"

#include <Device/Protocol/DeviceSettings.hpp>
#include <Explorer/DeviceList.hpp>
#include <Explorer/DeviceLogging.hpp>
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Protocols/OSC/OSCSpecificSettings.hpp>

#include <score/application/ApplicationContext.hpp>
#include <score/document/DocumentContext.hpp>

#include <ossia/network/generic/generic_device.hpp>
#include <ossia/network/generic/generic_parameter.hpp>
#include <ossia/network/rate_limiting_protocol.hpp>
#include <ossia/protocols/osc/osc_factory.hpp>

#include <memory>

namespace Protocols
{

OSCDevice::OSCDevice(
    const Device::DeviceSettings& settings,
    const ossia::net::network_context_ptr& ctx)
    : OwningDeviceInterface{settings}
    , m_ctx{ctx}
{
  using namespace ossia;
  m_capas.canLearn = true;
  m_capas.hasCallbacks = false;
}

bool OSCDevice::reconnect()
{
  disconnect();

  try
  {
    const OSCSpecificSettings& stgs
        = settings().deviceSpecificSettings.value<OSCSpecificSettings>();
    if(auto proto = ossia::net::make_osc_protocol(m_ctx, stgs.configuration))
    {
      if (stgs.rate)
      {
        auto rate = std::make_unique<ossia::net::rate_limiting_protocol>(
            std::chrono::milliseconds{*stgs.rate}, std::move(proto));
        m_dev = std::make_unique<ossia::net::generic_device>(
            std::move(rate), settings().name.toStdString());
      }
      else
      {
        m_dev = std::make_unique<ossia::net::generic_device>(
            std::move(proto), settings().name.toStdString());
      }

      deviceChanged(nullptr, m_dev.get());
      setLogging_impl(Device::get_cur_logging(isLogging()));
    }
    else
    {
      qDebug() << "Could not create OSC protocol";
    }
  }
  catch (std::exception& e)
  {
    qDebug() << "OSC Protocol error: " << e.what();
  }
  catch (...)
  {
    SCORE_TODO;
  }

  return connected();
}

void OSCDevice::recreate(const Device::Node& n)
{
  for (auto& child : n)
  {
    addNode(child);
  }
}

bool OSCDevice::isLearning() const
{
  auto& proto
      = static_cast<ossia::net::osc_protocol_base&>(m_dev->get_protocol());
  return proto.learning();
}

void OSCDevice::setLearning(bool b)
{
  if (!m_dev)
    return;
  auto& proto
      = static_cast<ossia::net::osc_protocol_base&>(m_dev->get_protocol());
  auto& dev = *m_dev;
  if (b)
  {
    dev.on_node_created.connect<&DeviceInterface::nodeCreated>(
        (DeviceInterface*)this);
    dev.on_node_removing.connect<&DeviceInterface::nodeRemoving>(
        (DeviceInterface*)this);
    dev.on_node_renamed.connect<&DeviceInterface::nodeRenamed>(
        (DeviceInterface*)this);
    dev.on_parameter_created.connect<&DeviceInterface::addressCreated>(
        (DeviceInterface*)this);
    dev.on_attribute_modified.connect<&DeviceInterface::addressUpdated>(
        (DeviceInterface*)this);
  }
  else
  {
    dev.on_node_created.disconnect<&DeviceInterface::nodeCreated>(
        (DeviceInterface*)this);
    dev.on_node_removing.disconnect<&DeviceInterface::nodeRemoving>(
        (DeviceInterface*)this);
    dev.on_node_renamed.disconnect<&DeviceInterface::nodeRenamed>(
        (DeviceInterface*)this);
    dev.on_parameter_created.disconnect<&DeviceInterface::addressCreated>(
        (DeviceInterface*)this);
    dev.on_attribute_modified.disconnect<&DeviceInterface::addressUpdated>(
        (DeviceInterface*)this);
  }

  proto.set_learning(b);
}
}
