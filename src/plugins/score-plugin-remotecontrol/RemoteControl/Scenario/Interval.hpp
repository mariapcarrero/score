#pragma once
#include <score/model/ComponentHierarchy.hpp>

#include <RemoteControl/DocumentPlugin.hpp>
#include <RemoteControl/Scenario/Process.hpp>
#include <Scenario/Document/Components/IntervalComponent.hpp>

namespace RemoteControl
{
class IntervalBase
    : public Scenario::GenericIntervalComponent<RemoteControl::DocumentPlugin>
{
  COMMON_COMPONENT_METADATA("b079041c-f11f-49b1-a88f-b2bc070affb1")
public:
  using parent_t
      = Scenario::GenericIntervalComponent<RemoteControl::DocumentPlugin>;
  using DocumentPlugin = RemoteControl::DocumentPlugin;
  using model_t = Process::ProcessModel;
  using component_t = RemoteControl::ProcessComponent;
  using component_factory_list_t = RemoteControl::ProcessComponentFactoryList;

  IntervalBase(
      Scenario::IntervalModel& Interval,
      DocumentPlugin& doc,
      QObject* parent_comp);

  ~IntervalBase();

  ProcessComponent* make(
      ProcessComponentFactory& factory,
      Process::ProcessModel& process);
  ProcessComponent*
  make(Process::ProcessModel& process);

  bool
  removing(const Process::ProcessModel& cst, const ProcessComponent& comp);

  template <typename... Args>
  void added(Args&&...)
  {
  }
  template <typename... Args>
  void removed(Args&&...)
  {
  }
};

class Interval final
    : public score::PolymorphicComponentHierarchy<IntervalBase>
{
public:
  using score::PolymorphicComponentHierarchy<
      IntervalBase>::PolymorphicComponentHierarchyManager;
};
}
