// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Process/Execution/ProcessComponent.hpp>
#include <Process/ExecutionContext.hpp>

#include <score/document/DocumentInterface.hpp>
#include <score/model/EntityMap.hpp>
#include <score/model/IdentifiedObjectMap.hpp>
#include <score/model/Identifier.hpp>
#include <score/widgets/MessageBox.hpp>

#include <core/document/Document.hpp>

#include <ossia/dataflow/graph/graph_interface.hpp>
#include <ossia/dataflow/graph_edge.hpp>
#include <ossia/editor/loop/loop.hpp>
#include <ossia/editor/scenario/scenario.hpp>
#include <ossia/editor/scenario/time_event.hpp>
#include <ossia/editor/scenario/time_interval.hpp>
#include <ossia/editor/scenario/time_sync.hpp>
#include <ossia/editor/scenario/time_value.hpp>
#include <ossia/editor/state/state.hpp>

#include <Scenario/Document/Event/EventExecution.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/Event/ExecutionStatus.hpp>
#include <Scenario/Document/Interval/IntervalDurations.hpp>
#include <Scenario/Document/Interval/IntervalExecution.hpp>
#include <Scenario/Document/State/StateExecution.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/TimeSync/TimeSyncExecution.hpp>
#include <Scenario/Document/TimeSync/TimeSyncModel.hpp>
#include <Scenario/Execution/score2OSSIA.hpp>
#include <Scenario/ExecutionChecker/CSPCoherencyCheckerInterface.hpp>
#include <Scenario/ExecutionChecker/CSPCoherencyCheckerList.hpp>
#include <Scenario/ExecutionChecker/CoherencyCheckerFactoryInterface.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <Scenario/Process/ScenarioExecution.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <wobjectimpl.h>

#include <vector>
W_OBJECT_IMPL(Execution::ScenarioComponentBase)

namespace Execution
{
ScenarioComponentBase::ScenarioComponentBase(
    Scenario::ProcessModel& element,
    const Context& ctx,
    QObject* parent)
    : ProcessComponent_T<
        Scenario::ProcessModel,
        ossia::scenario>{element, ctx, "ScenarioComponent", nullptr}
    , m_ctx{ctx}
    , m_graph{element}
{
  this->setObjectName("OSSIAScenarioElement");

  if (element.hasCycles())
  {
    score::warning(
        nullptr,
        "Warning !",
        "A scenario has cycles. It won't be executing. Look for the red "
        "transitions.");
    throw std::runtime_error("Processus cannot execute");
  }

  // Setup of the OSSIA API Part
  m_ossia_process = std::make_shared<ossia::scenario>();
  node = m_ossia_process->node;
  connect(
      this,
      &ScenarioComponentBase::sig_eventCallback,
      this,
      &ScenarioComponentBase::eventCallback,
      Qt::QueuedConnection);
}

ScenarioComponentBase::~ScenarioComponentBase() { }

ScenarioComponent::ScenarioComponent(
    Scenario::ProcessModel& proc,
    const Context& ctx,
    QObject* parent)
    : ScenarioComponentHierarchy{score::lazy_init_t{}, proc, ctx, parent}
{
}

void ScenarioComponent::lazy_init()
{
  // set-up the ports
  const Context& ctx = system();
  auto ossia_sc = std::dynamic_pointer_cast<ossia::scenario>(m_ossia_process);

  ScenarioComponentHierarchy::init();

  if (auto fact
      = ctx.doc.app.interfaces<Scenario::CSPCoherencyCheckerList>().get())
  {
    m_checker = fact->make(process(), ctx.doc.app, m_properties);
    if (m_checker)
    {
      m_properties.timesyncs[Id<Scenario::TimeSyncModel>(0)].date = 0;
      m_checker->computeDisplacement(m_pastTn, m_properties);
    }
  }

  /* TODO reinstate settings
  if (ctx.doc.app.settings<Settings::Model>().getScoreOrder())
  {
    std::vector<ossia::edge_ptr> edges_to_add;
    edges_to_add.reserve(m_ossia_intervals.size() * 2);

    for (auto& state : m_ossia_states)
    {
      // Add link between previous interval and state & first state process
      // And between state's last process and interval's first process
      auto& state_model = state.second->state();
      auto prev = state_model.previousInterval();
      auto next = state_model.nextInterval();
      auto& state_procs = state.second->processes();
      std::shared_ptr<ossia::graph_node> state_first_node, state_last_node;

      if (state.second->node())
      {
        state_first_node = state.second->node();

        if (state_procs.empty())
          state_last_node = state.second->node();
      }

      if (!state_procs.empty())
      {
        if (!state_first_node)
        {
          state_first_node
              = state_procs.at(state_model.stateProcesses.begin()->id())
                    ->OSSIAProcess()
                    .node;
        }

        state_last_node
            = state_procs.at(state_model.stateProcesses.rbegin()->id())
                  ->OSSIAProcess()
                  .node;
      }

      if (!state_first_node || !state_last_node)
        continue;

      if (prev)
      {
        auto prev_itv = m_ossia_intervals.at(*prev);
        SCORE_ASSERT(prev_itv->OSSIAInterval()->node);

        edges_to_add.push_back(ossia::make_edge(
            ossia::dependency_connection{}, ossia::outlet_ptr{},
            ossia::inlet_ptr{}, prev_itv->OSSIAInterval()->node,
            state_first_node));
      }

      if (next)
      {
        auto next_itv = m_ossia_intervals.at(*next);
        SCORE_ASSERT(next_itv->OSSIAInterval()->node);
        if (!next_itv->processes().empty())
        {
          auto proc = next_itv->processes().at(
              next_itv->interval().processes.begin()->id());
          if (auto ossia_proc = proc->OSSIAProcessPtr())
          {
            if (auto node = ossia_proc->node)
            {
              edges_to_add.push_back(ossia::make_edge(
                  ossia::dependency_connection{}, ossia::outlet_ptr{},
                  ossia::inlet_ptr{}, state_last_node, node));
            }
          }
        }
        else
        {
          edges_to_add.push_back(ossia::make_edge(
              ossia::dependency_connection{}, ossia::outlet_ptr{},
              ossia::inlet_ptr{}, state_last_node,
              next_itv->OSSIAInterval()->node));
        }
      }
    }

    std::weak_ptr<ossia::graph_interface> g_weak = ctx.execGraph;

    in_exec([edges = std::move(edges_to_add), g_weak] {
      if (auto g = g_weak.lock())
      {
        for (auto& c : edges)
        {
          g->connect(std::move(c));
        }
      }
    });
  }
  */
}

void ScenarioComponent::cleanup()
{
  clear();
  ProcessComponent::cleanup();
}

void ScenarioComponentBase::stop()
{
  for (const auto& itv : m_ossia_intervals)
  {
    itv.second->stop();
  }
  m_executingIntervals.clear();
  ProcessComponent::stop();
}

std::function<void()> ScenarioComponentBase::removing(
    const Scenario::IntervalModel& e,
    IntervalComponent& c)
{
  auto it = m_ossia_intervals.find(e.id());
  if (it != m_ossia_intervals.end())
  {
    std::shared_ptr<ossia::scenario> proc
        = std::dynamic_pointer_cast<ossia::scenario>(m_ossia_process);

    std::shared_ptr<ossia::time_event> start_ev, end_ev;
    auto start_ev_it = m_ossia_timeevents.find(
        Scenario::startEvent(e, this->process()).id());
    if (start_ev_it != m_ossia_timeevents.end())
      start_ev = start_ev_it->second->OSSIAEvent();
    auto end_ev_it
        = m_ossia_timeevents.find(Scenario::endEvent(e, this->process()).id());
    if (end_ev_it != m_ossia_timeevents.end())
      end_ev = end_ev_it->second->OSSIAEvent();

    m_ctx.executionQueue.enqueue(
        [proc, cstr = c.OSSIAInterval(), start_ev, end_ev] {
          if (cstr)
          {
            if (start_ev)
            {
              auto& next = start_ev->next_time_intervals();
              auto next_it = ossia::find(next, cstr);
              if (next_it != next.end())
                next.erase(next_it);
            }

            if (end_ev)
            {
              auto& prev = end_ev->previous_time_intervals();
              auto prev_it = ossia::find(prev, cstr);
              if (prev_it != prev.end())
                prev.erase(prev_it);
            }

            proc->remove_time_interval(cstr);
          }
        });

    c.cleanup(it->second);

    return [=] { m_ossia_intervals.erase(it); };
  }
  return {};
}

std::function<void()> ScenarioComponentBase::removing(
    const Scenario::TimeSyncModel& e,
    TimeSyncComponent& c)
{
  // FIXME this will certainly break stuff WRT member variables, coherency
  // checker, etc.
  auto it = m_ossia_timesyncs.find(e.id());
  if (it != m_ossia_timesyncs.end())
  {
    if (e.id() != Scenario::startId<Scenario::TimeSyncModel>())
    {
      std::shared_ptr<ossia::scenario> proc
          = std::dynamic_pointer_cast<ossia::scenario>(m_ossia_process);
      m_ctx.executionQueue.enqueue(
          [proc, tn = c.OSSIATimeSync()] { proc->remove_time_sync(tn); });
    }
    it->second->cleanup();

    return [=] { m_ossia_timesyncs.erase(it); };
  }
  return {};
}

std::function<void()> ScenarioComponentBase::removing(
    const Scenario::EventModel& e,
    EventComponent& c)
{
  auto ev_it = m_ossia_timeevents.find(e.id());
  if (ev_it != m_ossia_timeevents.end())
  {
    // Timesync still exists
    auto tn_it = m_ossia_timesyncs.find(e.timeSync());
    if (tn_it != m_ossia_timesyncs.end() && tn_it->second)
    {
      if (auto tn = tn_it->second->OSSIATimeSync())
      {
        m_ctx.executionQueue.enqueue([tn, ev = c.OSSIAEvent()] {
          tn->remove(ev);
          ev->cleanup();
        });
        c.cleanup();
        return [=] { m_ossia_timeevents.erase(ev_it); };
      }
    }

    // Timesync does not exist anymore:
    m_ctx.executionQueue.enqueue([ev = c.OSSIAEvent()] { ev->cleanup(); });
    c.cleanup();
    return [=] { m_ossia_timeevents.erase(ev_it); };
  }
  return {};
}

std::function<void()> ScenarioComponentBase::removing(
    const Scenario::StateModel& e,
    StateComponent& c)
{
  auto it = m_ossia_states.find(e.id());
  if (it != m_ossia_states.end())
  {
    c.onDelete();
    return [=] { m_ossia_states.erase(it); };
  }
  return {};
}

static void ScenarioIntervalCallback(bool, ossia::time_value) { }

template <>
IntervalComponent*
ScenarioComponentBase::make<IntervalComponent, Scenario::IntervalModel>(
    Scenario::IntervalModel& cst)
{
  std::shared_ptr<ossia::scenario> proc
      = std::dynamic_pointer_cast<ossia::scenario>(m_ossia_process);

  // Create the mapping object
  std::shared_ptr<IntervalComponent> elt
      = std::make_shared<IntervalComponent>(cst, proc, m_ctx, this);
  m_ossia_intervals.insert({cst.id(), elt});

  // Find the elements related to this interval.
  auto& te = m_ossia_timeevents;

  SCORE_ASSERT(
      te.find(process().state(cst.startState()).eventId()) != te.end());
  SCORE_ASSERT(te.find(process().state(cst.endState()).eventId()) != te.end());
  auto ossia_sev = te.at(process().state(cst.startState()).eventId());
  auto ossia_eev = te.at(process().state(cst.endState()).eventId());

  // Create the time_interval
  auto dur = elt->makeDurations();
  auto ossia_cst = std::make_shared<ossia::time_interval>(
      smallfun::function<void(bool, ossia::time_value), 32>{
          &ScenarioIntervalCallback},
      *ossia_sev->OSSIAEvent(),
      *ossia_eev->OSSIAEvent(),
      dur.defaultDuration,
      dur.minDuration,
      dur.maxDuration);

  elt->onSetup(elt, ossia_cst, dur);

  const bool prop = cst.graphal() ? false : cst.outlet->propagate();
  m_ctx.executionQueue.enqueue(
      [g = system().execGraph, proc, ossia_sev, ossia_eev, ossia_cst, prop] {
        if (auto sev = ossia_sev->OSSIAEvent())
          sev->next_time_intervals().push_back(ossia_cst);
        if (auto eev = ossia_eev->OSSIAEvent())
          eev->previous_time_intervals().push_back(ossia_cst);

        proc->add_time_interval(ossia_cst);

        if(prop)
        {
          auto cable = ossia::make_edge(
              ossia::immediate_glutton_connection{},
              ossia_cst->node->root_outputs()[0],
              proc->node->root_inputs()[0],
              ossia_cst->node,
              proc->node);
          g->connect(cable);
        }
      });
  return elt.get();
}

template <>
StateComponent*
ScenarioComponentBase::make<StateComponent, Scenario::StateModel>(
    Scenario::StateModel& st)
{
  auto& events = m_ossia_timeevents;
  SCORE_ASSERT(events.find(st.eventId()) != events.end());
  auto ossia_ev = events.at(st.eventId());

  auto elt = std::make_shared<StateComponent>(
      st, ossia_ev->OSSIAEvent(), m_ctx, this);

  m_ctx.executionQueue.enqueue([elt] { elt->onSetup(); });

  m_ossia_states.insert({st.id(), elt});

  return elt.get();
}

template <>
EventComponent*
ScenarioComponentBase::make<EventComponent, Scenario::EventModel>(
    Scenario::EventModel& ev)
{
  // Create the component
  auto elt = std::make_shared<EventComponent>(ev, m_ctx, this);
  m_ossia_timeevents.insert({ev.id(), elt});

  // Find the parent time sync for the new event
  auto& nodes = m_ossia_timesyncs;
  SCORE_ASSERT(nodes.find(ev.timeSync()) != nodes.end());
  auto tn = nodes.at(ev.timeSync());

  // Create the event
  std::weak_ptr<EventComponent> weak_ev = elt;

  std::weak_ptr<ScenarioComponentBase> thisP
      = std::dynamic_pointer_cast<ScenarioComponentBase>(shared_from_this());
  auto ev_cb = [weak_ev, thisP](ossia::time_event::status st) {
    if (auto elt = weak_ev.lock())
    {
      if (auto sc = thisP.lock())
      {
        sc->sig_eventCallback(elt, st);
      }
    }
  };
  auto ossia_ev = std::make_shared<ossia::time_event>(
      std::move(ev_cb), *tn->OSSIATimeSync(), ossia::expression_ptr{});

  elt->onSetup(
      ossia_ev,
      elt->makeExpression(),
      (ossia::time_event::offset_behavior)(ev.offsetBehavior()));

  connect(
      &ev,
      &Scenario::EventModel::timeSyncChanged,
      this,
      [=](const Id<Scenario::TimeSyncModel>& old_ts_id,
          const Id<Scenario::TimeSyncModel>& new_ts_id) {
        auto old_ts = m_ossia_timesyncs.at(old_ts_id);
        auto new_ts = m_ossia_timesyncs.at(new_ts_id);
        SCORE_ASSERT(old_ts);
        SCORE_ASSERT(new_ts);
        SCORE_ASSERT(old_ts->OSSIATimeSync());
        SCORE_ASSERT(new_ts->OSSIATimeSync());

        m_ctx.executionQueue.enqueue([old_t = old_ts->OSSIATimeSync(),
                                      new_t = new_ts->OSSIATimeSync(),
                                      ossia_ev] {
          old_t->remove(ossia_ev);
          new_t->insert(new_t->get_time_events().end(), ossia_ev);
          ossia_ev->set_time_sync(*new_t);
        });
      });

  // The event is inserted in the API edition thread
  m_ctx.executionQueue.enqueue(
      [event = ossia_ev, time_sync = tn->OSSIATimeSync()] {
        SCORE_ASSERT(event);
        SCORE_ASSERT(time_sync);
        time_sync->insert(time_sync->get_time_events().begin(), event);
      });

  return elt.get();
}

template <>
TimeSyncComponent*
ScenarioComponentBase::make<TimeSyncComponent, Scenario::TimeSyncModel>(
    Scenario::TimeSyncModel& tn)
{
  // Create the object
  auto elt = std::make_shared<TimeSyncComponent>(tn, m_ctx, this);
  m_ossia_timesyncs.insert({tn.id(), elt});

  bool must_add = false;
  // The OSSIA API already creates the start time sync so we must use it if
  // available
  std::shared_ptr<ossia::time_sync> ossia_tn;
  if (tn.id() == Scenario::startId<Scenario::TimeSyncModel>())
  {
    ossia_tn = OSSIAProcess().get_start_time_sync();
  }
  else
  {
    ossia_tn = std::make_shared<ossia::time_sync>();
    must_add = true;
  }

  // Setup the object
  elt->onSetup(ossia_tn, elt->makeTrigger());

  // What happens when a time sync's status change
  ossia_tn->triggered.add_callback([thisP = shared_from_this(), elt] {
    auto& sub = static_cast<ScenarioComponentBase&>(*thisP);
    return sub.timeSyncCallback(
        elt.get(),
        ossia::
            time_value{} /* TODO WTF sub.m_parent_interval.OSSIAInterval()->get_date()*/);
  });

  // Changing the running API structures
  if (must_add)
  {
    auto ossia_sc
        = std::dynamic_pointer_cast<ossia::scenario>(m_ossia_process);
    m_ctx.executionQueue.enqueue(
        [ossia_sc, ossia_tn] { ossia_sc->add_time_sync(ossia_tn); });
    /*
    if(Scenario::previousIntervals(tn, process()).empty())
    {
      if(!tn.active())
      {
        auto ghost_end = std::make_shared<ossia::time_event>(
              ossia::time_event::exec_callback{},
              *ossia_tn,
              ossia::expressions::make_expression_true());

        auto duration = m_ctx.time(tn.date());
        auto ghost_itv = std::make_shared<ossia::time_interval>(
              ossia::time_interval::exec_callback{},
              *m_ghost_start, *ghost_end
              , duration, duration, duration);

        // todo on interval added
        // todo on TS moved
        // todo on TS removed
        ossia_tn->insert(ossia_tn->get_time_events().end(), ghost_end);
        m_ctx.executionQueue.enqueue(
              [ossia_sc, ghost_itv, ghost_start=m_ghost_start, ghost_end] {

          ghost_start->next_time_intervals().push_back(ghost_itv);
          ghost_end->previous_time_intervals().push_back(ghost_itv);

          ossia_sc->add_time_interval(ghost_itv);
        });
      }
    }
    */
  }

  return elt.get();
}

void ScenarioComponentBase::startIntervalExecution(
    const Id<Scenario::IntervalModel>& id)
{
  auto& cst = process().intervals.at(id);
  if (m_executingIntervals.find(id) == m_executingIntervals.end())
    m_executingIntervals.insert(std::make_pair(cst.id(), &cst));

  auto it = m_ossia_intervals.find(id);
  if (it != m_ossia_intervals.end())
    it->second->executionStarted();

  cst.setExecutionState(Scenario::IntervalExecutionState::Enabled);
}

void ScenarioComponentBase::disableIntervalExecution(
    const Id<Scenario::IntervalModel>& id)
{
  auto& cst = process().intervals.at(id);
  cst.setExecutionState(Scenario::IntervalExecutionState::Disabled);
}

void ScenarioComponentBase::stopIntervalExecution(
    const Id<Scenario::IntervalModel>& id)
{
  m_executingIntervals.erase(id);
  auto it = m_ossia_intervals.find(id);
  if (it != m_ossia_intervals.end())
    it->second->executionStopped();
}

void ScenarioComponentBase::eventCallback(
    std::shared_ptr<EventComponent> ev,
    ossia::time_event::status newStatus)
{
  auto the_event = const_cast<Scenario::EventModel*>(ev->scoreEvent());
  if (!the_event)
    return;

  the_event->setStatus(
      static_cast<Scenario::ExecutionStatus>(newStatus), process());

  for (auto& state : the_event->states())
  {
    auto& score_state = process().states.at(state);

    if (auto& c = score_state.previousInterval())
    {
      m_properties.intervals[*c].status
          = static_cast<Scenario::ExecutionStatus>(newStatus);
    }

    switch (newStatus)
    {
      case ossia::time_event::status::NONE:
        break;
      case ossia::time_event::status::PENDING:
        break;
      case ossia::time_event::status::HAPPENED:
      {
        // Stop the previous intervals clocks,
        // start the next intervals clocks
        if (score_state.previousInterval())
        {
          stopIntervalExecution(*score_state.previousInterval());
        }

        if (score_state.nextInterval())
        {
          startIntervalExecution(*score_state.nextInterval());
        }
        break;
      }

      case ossia::time_event::status::DISPOSED:
      {
        // TODO disable the intervals graphically
        if (score_state.nextInterval())
        {
          disableIntervalExecution(*score_state.nextInterval());
        }
        break;
      }
      default:
        SCORE_TODO;
        break;
    }
  }
}

void ScenarioComponentBase::timeSyncCallback(
    TimeSyncComponent* tn,
    ossia::time_value date)
{
  if (m_checker)
  {
    m_pastTn.push_back(tn->scoreTimeSync().id());

    // Fix Timenode
    auto& curTnProp = m_properties.timesyncs[tn->scoreTimeSync().id()];
    curTnProp.date = double(date.impl);
    curTnProp.date_max = curTnProp.date;
    curTnProp.date_min = curTnProp.date;
    curTnProp.status = Scenario::ExecutionStatus::Happened;

    // Fix previous intervals
    auto previousCstrs
        = Scenario::previousIntervals(tn->scoreTimeSync(), process());

    for (auto& cstrId : previousCstrs)
    {
      auto& startTn
          = Scenario::startTimeSync(process().interval(cstrId), process());
      auto& cstrProp = m_properties.intervals[cstrId];

      cstrProp.newMin.setMSecs(
          curTnProp.date - m_properties.timesyncs[startTn.id()].date);
      cstrProp.newMax = cstrProp.newMin;

      cstrProp.status = Scenario::ExecutionStatus::Happened;
    }

    // Compute new values
    m_checker->computeDisplacement(m_pastTn, m_properties);

    // Update intervals
    for (auto& cstr : m_ossia_intervals)
    {
      auto ossiaCstr = cstr.second->OSSIAInterval();

      auto tmin = m_properties.intervals[cstr.first].newMin.msec();
      ossiaCstr->set_min_duration(ossia::time_value{int64_t(tmin)});

      auto tmax = m_properties.intervals[cstr.first].newMax.msec();
      ossiaCstr->set_max_duration(ossia::time_value{int64_t(tmax)});
    }
  }
}
}
