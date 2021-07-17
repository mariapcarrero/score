#pragma once
#include <Control/DefaultEffectItem.hpp>
#include <Effect/EffectFactory.hpp>
#include <Pd/Inspector/PdInspectorWidget.hpp>
#include <Pd/PdProcess.hpp>
#include <Process/GenericProcessFactory.hpp>
#include <Process/WidgetLayer/WidgetProcessFactory.hpp>

#include <QProcess>
#include <QFileInfo>
#include <z_libpd.h>
namespace Pd
{
/*
struct UiWrapper : public QWidget
{
  QPointer<const ProcessModel> m_model;
  QProcess m_process;
  UiWrapper(
      const ProcessModel& proc,
      const score::DocumentContext& ctx,
      QWidget* parent)
      : m_model{&proc}
  {
    setGeometry(0, 0, 0, 0);

    connect(
        &m_process,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        &QWidget::deleteLater);
    connect(
        &proc,
        &IdentifiedObjectAbstract::identified_object_destroying,
        this,
        &QWidget::deleteLater);
    const auto& bin = locatePdBinary();
    if (!bin.isEmpty())
    {
      m_process.start(bin, {proc.script()});
    }
  }

  void closeEvent(QCloseEvent* event) override
  {
    QPointer<UiWrapper> p(this);
    if (m_model)
    {
      const_cast<QWidget*&>(m_model->externalUI) = nullptr;
      m_model->externalUIVisible(false);
    }
    if (p)
      QWidget::closeEvent(event);
  }

  ~UiWrapper()
  {
    if (m_model)
    {
      const_cast<QWidget*&>(m_model->externalUI) = nullptr;
      m_model->externalUIVisible(false);
    }
    m_process.terminate();
  }
};
*/
struct UiWrapper : public QWidget
{
  std::shared_ptr<Pd::Instance> m_instance;
  QPointer<const ProcessModel> m_model;
  UiWrapper(
      const ProcessModel& proc,
      const score::DocumentContext& ctx,
      QWidget* parent)
      : m_model{&proc}
      , m_instance{proc.m_instance}
  {
    setGeometry(0, 0, 0, 0);

    connect(
        &proc,
        &IdentifiedObjectAbstract::identified_object_destroying,
        this,
        &QWidget::deleteLater);
    const auto& bin = locatePdBinary();
    if (!bin.isEmpty())
    {
      libpd_set_instance(m_instance->instance);
      libpd_start_gui(locatePdResourceFolder().toUtf8().constData());
    }
    startTimer(8);
  }


  QString locatePdResourceFolder() noexcept {
#if defined(__linux__)
    return "/usr/lib/pd";
#else
    return QFileInfo{locatePdBinary()}.absolutePath();
#endif
  }

  void closeEvent(QCloseEvent* event) override
  {
    QPointer<UiWrapper> p(this);
    if (m_model)
    {
      const_cast<QWidget*&>(m_model->externalUI) = nullptr;
      //libpd_set_instance(m_model->m_instance.instance);
      //libpd_stop_gui();
      m_model->externalUIVisible(false);
    }
    if (p)
    {
      QWidget::closeEvent(event);
    }
  }

  ~UiWrapper()
  {
    if (m_model)
    {
      const_cast<QWidget*&>(m_model->externalUI) = nullptr;
      libpd_set_instance(m_instance->instance);
      libpd_stop_gui();
      m_model->externalUIVisible(false);
    }
  }

  void timerEvent(QTimerEvent* event) override
  {
    libpd_set_instance(m_instance->instance);
    libpd_poll_gui();
  }
};


using LayerFactory = Process::EffectLayerFactory_T<
    Pd::ProcessModel,
    Process::DefaultEffectItem,
    UiWrapper>;
}
