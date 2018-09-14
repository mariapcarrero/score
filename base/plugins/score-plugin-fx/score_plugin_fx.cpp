// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "score_plugin_fx.hpp"

#include <Fx/AngleNode.hpp>
#include <Fx/Arpeggiator.hpp>
#include <Fx/Chord.hpp>
#include <Fx/ClassicalBeat.hpp>
#include <Fx/EmptyMapping.hpp>
#include <Fx/Envelope.hpp>
#include <Fx/FactorOracle.hpp>
#include <Fx/Gain.hpp>
#include <Fx/LFO.hpp>
#include <Fx/MathGenerator.hpp>
#include <Fx/MathMapping.hpp>
#include <Fx/Metro.hpp>
#include <Fx/MidiUtil.hpp>
#include <Fx/Quantifier.hpp>
#include <Fx/TestNode.hpp>
#include <Fx/VelToNote.hpp>

#include <score/plugins/customfactory/FactorySetup.hpp>

#include <score_plugin_engine.hpp>

namespace Control
{
}
score_plugin_fx::score_plugin_fx() = default;
score_plugin_fx::~score_plugin_fx() = default;

std::vector<std::unique_ptr<score::InterfaceBase>> score_plugin_fx::factories(
    const score::ApplicationContext& ctx, const score::InterfaceKey& key) const
{
  return Control::instantiate_fx<
      Nodes::Arpeggiator::Node, Nodes::PulseToNote::Node,
      Nodes::ClassicalBeat::Node, Nodes::LFO::Node, Nodes::Chord::Node,
      Nodes::MidiUtil::Node, Nodes::Gain::Node, Nodes::Metro::Node,
      Nodes::Envelope::Node, Nodes::Quantifier::Node,
      Nodes::MathGenerator::Node, Nodes::MathAudioGenerator::Node,
      Nodes::MathMapping::Node, Nodes::EmptyValueMapping::Node,
      Nodes::EmptyMidiMapping::Node, Nodes::EmptyAudioMapping::Node,
      Nodes::FactorOracle::Node>(ctx, key);
}

auto score_plugin_fx::required() const -> std::vector<score::PluginKey>
{
  return {score_plugin_engine::static_key()};
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_plugin_fx)
