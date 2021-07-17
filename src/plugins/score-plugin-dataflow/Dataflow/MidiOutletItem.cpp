#include "MidiOutletItem.hpp"

#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Inspector/InspectorLayout.hpp>
#include <Process/Dataflow/AudioPortComboBox.hpp>

#if defined(SCORE_PLUGIN_PROTOCOLS)
#include <Protocols/MIDI/MIDIProtocolFactory.hpp>
#include <Protocols/MIDI/MIDISpecificSettings.hpp>
#endif

#include <score/document/DocumentContext.hpp>

namespace Dataflow
{
void MidiOutletFactory::setupOutletInspector(
    const Process::Outlet& port,
    const score::DocumentContext& ctx,
    QWidget* parent,
    Inspector::Layout& lay,
    QObject* context)
{
#if defined(SCORE_PLUGIN_PROTOCOLS)
  static const MSVC_BUGGY_CONSTEXPR auto midi_uuid
      = Protocols::MIDIOutputProtocolFactory::static_concreteKey();

  auto& device = *ctx.findPlugin<Explorer::DeviceDocumentPlugin>();
  QStringList midiDevices;
  midiDevices.push_back("");

  device.list().apply([&](Device::DeviceInterface& dev) {
    auto& set = dev.settings();
    if (set.protocol == midi_uuid)
    {
      const auto& midi_set = set.deviceSpecificSettings
                                 .value<Protocols::MIDISpecificSettings>();
      if (midi_set.io == Protocols::MIDISpecificSettings::IO::Out)
        midiDevices.push_back(set.name);
    }
  });

  lay.addRow(Process::makeDeviceCombo(midiDevices, port, ctx, parent));
#endif
}
}
