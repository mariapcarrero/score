#pragma once
#include <Process/ExecutionAction.hpp>

#include <score/application/ApplicationContext.hpp>

#include <ossia/audio/audio_engine.hpp>
#include <ossia/audio/audio_tick.hpp>

#include <score_plugin_audio_export.h>

namespace Audio
{
using tick_fun = ossia::audio_engine::fun_type;

SCORE_PLUGIN_AUDIO_EXPORT
tick_fun makePauseTick(const score::ApplicationContext& app);

SCORE_PLUGIN_AUDIO_EXPORT
extern std::atomic<ossia::transport_status> execution_status;
}
