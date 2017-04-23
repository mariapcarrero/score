#pragma once
#include <Process/TimeValue.hpp>
#include <Process/ZoomHelper.hpp>

#include <QPoint>
#include <QString>
#include <QTimer>
#include <iscore/model/Identifier.hpp>
#include <iscore_plugin_scenario_export.h>
#include <nano_signal_slot.hpp>

namespace Process
{
struct ProcessPresenterContext;
}

namespace Scenario
{
class ConstraintHeader;
class ConstraintModel;
class ConstraintView;
class ConstraintViewModel;
class RackModel;
class RackPresenter;

class ISCORE_PLUGIN_SCENARIO_EXPORT ConstraintPresenter : public QObject,
                                                          public Nano::Observer
{
  Q_OBJECT

public:
  ConstraintPresenter(
      const ConstraintViewModel& model,
      ConstraintView* view,
      ConstraintHeader* header,
      const Process::ProcessPresenterContext& ctx,
      QObject* parent);
  virtual ~ConstraintPresenter();
  virtual void updateScaling();

  bool isSelected() const;

  const ConstraintModel& model() const;
  const ConstraintViewModel& abstractConstraintViewModel() const
  {
    return m_viewModel;
  }

  ConstraintView* view() const;

  virtual void on_zoomRatioChanged(ZoomRatio val);
  ZoomRatio zoomRatio() const
  {
    return m_zoomRatio;
  }

  const Id<ConstraintModel>& id() const;

  virtual void on_defaultDurationChanged(const TimeVal&);
  void on_minDurationChanged(const TimeVal&);
  void on_maxDurationChanged(const TimeVal&);

  void on_playPercentageChanged(double t);

signals:
  void pressed(QPointF) const;
  void moved(QPointF) const;
  void released(QPointF) const;

  void askUpdate();
  void heightChanged();           // The vertical size
  void heightPercentageChanged(); // The vertical position

protected:
  // Process presenters are in the slot presenters.
  const ConstraintViewModel& m_viewModel;
  ConstraintView* m_view{};
  ConstraintHeader* m_header{};

  const Process::ProcessPresenterContext& m_context;

private:
  void updateChildren();
  void updateBraces();

  ZoomRatio m_zoomRatio{};
};

// TODO concept: constraint view model.

template <typename T>
const typename T::viewmodel_type* viewModel(const T* obj)
{
  return static_cast<const typename T::viewmodel_type*>(
      &obj->abstractConstraintViewModel());
}

template <typename T>
const typename T::view_type* view(const T* obj)
{
  return static_cast<const typename T::view_type*>(obj->view());
}

template <typename T>
typename T::view_type* view(T* obj)
{
  return static_cast<typename T::view_type*>(obj->view());
}

template <typename T>
const typename T::viewmodel_type& viewModel(const T& obj)
{
  return static_cast<const typename T::viewmodel_type&>(
      obj.abstractConstraintViewModel());
}

template <typename T>
typename T::view_type& view(const T& obj)
{
  return static_cast<typename T::view_type&>(*obj.view());
}

template <typename T>
typename T::view_type& view(T& obj)
{
  return static_cast<typename T::view_type&>(*obj.view());
}
}
