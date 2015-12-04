#pragma once
#include <Process/ProcessFactoryKey.hpp>
#include <QString>
#include <iscore_plugin_automation_export.h>
struct ISCORE_PLUGIN_AUTOMATION_EXPORT AutomationProcessMetadata
{
        static const ProcessFactoryKey& factoryKey();

        static QString processObjectName();

        static QString factoryPrettyName();
};
