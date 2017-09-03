#include "Executor.hpp"

#include <Skeleton/Process.hpp>
#include <Device/Protocol/DeviceList.hpp>
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Engine/iscore2OSSIA.hpp>
#include <ossia/editor/state/state_element.hpp>

namespace Skeleton
{
ProcessExecutor::ProcessExecutor(
    const Device::DeviceList& devices):
  m_devices{devices}
{
}


void ProcessExecutor::start(ossia::state&)
{
}

void ProcessExecutor::stop()
{
}

void ProcessExecutor::pause()
{
}

void ProcessExecutor::resume()
{
}

ossia::state_element ProcessExecutor::state(ossia::time_value date, double pos)
{
  State::Address address{"my_device", {"a", "banana"}};
  ossia::value value = std::abs(qrand()) % 100;
  State::Message m;
  m.address = address;
  m.value = value;

  if(auto res = Engine::iscore_to_ossia::message(m, m_devices))
  {
    if(unmuted())
      return *res;
    return {};
  }
  else
  {
    return {};
  }
}

ProcessExecutorComponent::ProcessExecutorComponent(
    Engine::Execution::ConstraintComponent& parentConstraint,
    Skeleton::Model& element,
    const Engine::Execution::Context& ctx,
    const Id<iscore::Component>& id,
    QObject* parent):
  ProcessComponent_T{
    parentConstraint, element, ctx, id, "SkeletonExecutorComponent", parent}
{
  m_ossia_process = std::make_shared<ProcessExecutor>(ctx.devices.list());
}

}
