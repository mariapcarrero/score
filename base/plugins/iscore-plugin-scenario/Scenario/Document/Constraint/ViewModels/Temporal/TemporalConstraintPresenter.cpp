#include <Process/ProcessList.hpp>
#include <QGraphicsScene>
#include <QList>
#include <QApplication>
#include <Scenario/Commands/Constraint/AddProcessToConstraint.hpp>
#include <Scenario/DialogWidget/AddProcessDialog.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <Scenario/Document/Constraint/ViewModels/Temporal/TemporalConstraintView.hpp>
#include <Scenario/Document/Constraint/ViewModels/Temporal/TemporalConstraintViewModel.hpp>
#include <Scenario/Document/ScenarioDocument/ScenarioDocumentModel.hpp>
#include <Scenario/Document/ScenarioDocument/ScenarioDocumentPresenter.hpp>
#include <iscore/document/DocumentInterface.hpp>
#include <iscore/command/Dispatchers/CommandDispatcher.hpp>

#include <Scenario/Document/Constraint/Rack/RackPresenter.hpp>
#include <Scenario/Document/Constraint/Rack/RackView.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>

#include "TemporalConstraintHeader.hpp"
#include "TemporalConstraintPresenter.hpp"
#include "TemporalConstraintView.hpp"
#include <Process/ProcessContext.hpp>
#include <Scenario/Application/Drops/ScenarioDropHandler.hpp>
#include <Scenario/Document/Constraint/ViewModels/ConstraintHeader.hpp>
#include <Scenario/Document/Constraint/ViewModels/ConstraintPresenter.hpp>
#include <Scenario/Document/Constraint/ViewModels/ConstraintViewModel.hpp>
#include <iscore/model/ModelMetadata.hpp>
#include <iscore/selection/Selectable.hpp>
#include <iscore/tools/Todo.hpp>
class QColor;
class QObject;
class QString;

namespace Scenario
{
TemporalConstraintPresenter::TemporalConstraintPresenter(
    const TemporalConstraintViewModel& cstr_model,
    const Process::ProcessPresenterContext& ctx,
    QGraphicsItem* parentobject,
    QObject* parent)
  : ConstraintPresenter{cstr_model,
                        new TemporalConstraintView{*this, parentobject},
                        new TemporalConstraintHeader, ctx, parent}
{
  TemporalConstraintView& v = *Scenario::view(this);
  con(v, &TemporalConstraintView::constraintHoverEnter, this,
      &TemporalConstraintPresenter::constraintHoverEnter);

  con(v, &TemporalConstraintView::constraintHoverLeave, this,
      &TemporalConstraintPresenter::constraintHoverLeave);

  con(v, &ConstraintView::requestOverlayMenu,
      this, &TemporalConstraintPresenter::on_requestOverlayMenu);

  con(cstr_model.model(), &ConstraintModel::executionStateChanged, &v,
      &TemporalConstraintView::setExecutionState);

  const auto& metadata = m_viewModel.model().metadata();
  con(metadata, &iscore::ModelMetadata::LabelChanged, &v,
      &TemporalConstraintView::setLabel);

  con(metadata, &iscore::ModelMetadata::ColorChanged, &v,
      [&](iscore::ColorRef c) {
    v.setLabelColor(c);
    v.setColor(c);
  });

  con(metadata, &iscore::ModelMetadata::NameChanged, this,
      [&](const QString& name) { m_header->setText(name); });

  v.setLabel(metadata.getLabel());
  v.setLabelColor(metadata.getColor());
  v.setColor(metadata.getColor());
  m_header->setText(metadata.getName());
  v.setExecutionState(m_viewModel.model().executionState());

  con(m_viewModel.model().selection, &Selectable::changed, &v,
      &TemporalConstraintView::setFocused);
  con(m_viewModel.model(), &ConstraintModel::focusChanged, &v,
      &TemporalConstraintView::setFocused);

  // Drop
  con(v, &TemporalConstraintView::dropReceived, this,
      [=](const QPointF& pos, const QMimeData* mime) {
    m_context.app.interfaces<Scenario::ConstraintDropHandlerList>()
        .drop(m_viewModel.model(), mime);
  });

  // Header set-up
  auto header = static_cast<TemporalConstraintHeader*>(m_header);

  connect(
        header, &TemporalConstraintHeader::constraintHoverEnter, this,
        &TemporalConstraintPresenter::constraintHoverEnter);
  connect(
        header, &TemporalConstraintHeader::constraintHoverLeave, this,
        &TemporalConstraintPresenter::constraintHoverLeave);
  connect(
        header, &TemporalConstraintHeader::shadowChanged, &v,
        &TemporalConstraintView::setShadow);

  connect(
        header, &TemporalConstraintHeader::dropReceived, this,
        [=](const QPointF& pos, const QMimeData* mime) {
    m_context.app.interfaces<Scenario::ConstraintDropHandlerList>()
        .drop(m_viewModel.model(), mime);
  });
  // Go to full-view on double click
  connect(header, &TemporalConstraintHeader::doubleClicked, this, [this]() {
    using namespace iscore::IDocument;
    auto& base
        = get<ScenarioDocumentPresenter>(*documentFromObject(m_viewModel.model()));

    base.setDisplayedConstraint(m_viewModel.model());
  });


  con(m_viewModel, &ConstraintViewModel::rackShown, this,
      &ConstraintPresenter::on_rackShown);

  con(m_viewModel, &ConstraintViewModel::rackHidden, this,
      &ConstraintPresenter::on_rackHidden);

  con(m_viewModel, &ConstraintViewModel::lastRackRemoved, this,
      &ConstraintPresenter::on_noRacks);

  constraint.racks.added
      .connect<ConstraintPresenter, &ConstraintPresenter::on_racksChanged>(
        this);
  constraint.racks.removed
      .connect<ConstraintPresenter, &ConstraintPresenter::on_racksChanged>(
        this);
  on_racksChanged();
}

TemporalConstraintPresenter::~TemporalConstraintPresenter()
{
  auto view = Scenario::view(this);
  // TODO deleteGraphicsObject
  if (view)
  {
    auto sc = view->scene();

    if (sc && sc->items().contains(view))
    {
      sc->removeItem(view);
    }

    view->deleteLater();
  }

  delete m_rack;
}

void TemporalConstraintPresenter::on_requestOverlayMenu(QPointF)
{
  auto& fact = m_context.app.interfaces<Process::ProcessFactoryList>();
  auto dialog = new AddProcessDialog{fact, QApplication::activeWindow()};

  connect(
        dialog, &AddProcessDialog::okPressed, this,
        [&] (const auto& key) {
    auto cmd
        = Scenario::Command::make_AddProcessToConstraint(this->model(), key);

    CommandDispatcher<> d{m_context.commandStack};
    emit d.submitCommand(cmd);
  });

  dialog->launchWindow();
  dialog->deleteLater();
}


void TemporalConstraintPresenter::on_rackShown(const OptionalId<RackModel>& rackId)
{
  clearRackPresenter();
  if (rackId)
  {
    createRackPresenter(m_viewModel.model().smallViewRack());

    m_header->setState(ConstraintHeader::State::RackShown);
  }
  else
  {
    m_header->setState(ConstraintHeader::State::RackHidden);
  }
  updateHeight();
}

void TemporalConstraintPresenter::on_rackHidden()
{
  clearRackPresenter();

  m_header->setState(ConstraintHeader::State::RackHidden);
  updateHeight();
}

void TemporalConstraintPresenter::on_noRacks()
{
  // m_header->hide();
  clearRackPresenter();

  m_header->setState(ConstraintHeader::State::Hidden);
  updateHeight();
}

void TemporalConstraintPresenter::on_racksChanged(const RackModel&)
{
  on_racksChanged();
}

void TemporalConstraintPresenter::on_racksChanged()
{
  auto& constraint = m_viewModel.model();
  if (m_viewModel.isRackShown())
  {
    on_rackShown(*m_viewModel.shownRack());
  }
  else if (!constraint.processes.empty()) // TODO why isn't this when there
    // are racks but hidden ?
  {
    on_rackHidden();
  }
  else
  {
    on_noRacks();
  }
}

void TemporalConstraintPresenter::updateScaling()
{
  ConstraintPresenter::updateScaling();
  updateHeight();
}

void TemporalConstraintPresenter::on_zoomRatioChanged(ZoomRatio val)
{
  ConstraintPresenter::on_zoomRatioChanged(val);

  if (rack())
  {
    rack()->on_zoomRatioChanged(m_zoomRatio);
  }
}

void TemporalConstraintPresenter::on_defaultDurationChanged(const TimeVal& v)
{
  ConstraintPresenter::on_defaultDurationChanged(v);

  if (rack())
  {
    rack()->setWidth(m_view->defaultWidth());
  }
}

void TemporalConstraintPresenter::clearRackPresenter()
{
  if (m_rack)
  {
    m_rack->deleteLater();
    m_rack = nullptr;
  }
}

void TemporalConstraintPresenter::createRackPresenter(const RackModel& rackModel)
{
  auto rackView = new RackView{m_view};
  rackView->setPos(0, ConstraintHeader::headerHeight());

  // Cas par défaut
  m_rack = new RackPresenter{rackModel, rackView, m_context, this};

  m_rack->on_zoomRatioChanged(m_zoomRatio);

  connect(
        m_rack, &RackPresenter::askUpdate, this,
        &ConstraintPresenter::updateHeight);

  connect(m_rack, &RackPresenter::pressed,
          this, &ConstraintPresenter::pressed);
  connect(m_rack, &RackPresenter::moved,
          this, &ConstraintPresenter::moved);
  connect(m_rack, &RackPresenter::released,
          this, &ConstraintPresenter::released);
}

void TemporalConstraintPresenter::updateHeight()
{
  if (rack() && m_viewModel.isRackShown())
  {
    m_view->setHeight(rack()->height() + ConstraintHeader::headerHeight());
  }
  else if (rack() && !m_viewModel.isRackShown())
  {
    m_view->setHeight(ConstraintHeader::headerHeight());
  }
  else
  {
    m_view->setHeight(8);
  }

  updateChildren();
  emit heightChanged();
}

RackPresenter* TemporalConstraintPresenter::rack() const
{
  return m_rack;
}
}
