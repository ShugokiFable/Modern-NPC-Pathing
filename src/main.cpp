#include "evg_traversal.h"
#include "pathing.h"
#include "settings.h"

#include "SKSE/SKSE.h"

#include "RE/B/BSTEvent.h"
#include "RE/M/MenuOpenCloseEvent.h"
#include "RE/U/UI.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace
{
    void InitializeLogging()
    {
        auto logPath = SKSE::log::log_directory();
        if (!logPath) {
            return;
        }
        auto logFile = logPath.value() / "NPCPathingNG.log";
        auto logger = spdlog::basic_logger_mt("NPCPathingNG", logFile.string(), true);
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(logger));
    }

    // Re-read the fallback INI whenever the journal/system menu closes, so
    // non-MCM users can tweak settings without restarting. MCM users don't
    // need this — their changes flow through the ESP globals instantly.
    class MenuListener : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuListener* GetSingleton()
        {
            static MenuListener instance;
            return &instance;
        }

        static void Register()
        {
            if (auto* ui = RE::UI::GetSingleton()) {
                ui->AddEventSink(GetSingleton());
            }
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                              RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (a_event && !a_event->opening && a_event->menuName == "Journal Menu") {
                Settings::GetSingleton()->Load();
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
    {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            // Forms exist now — safe to cache keywords, bind MCM globals,
            // and install the frame hook.
            Settings::GetSingleton()->Load();
            Settings::GetSingleton()->BindGlobals();
            EvgTraversal::CacheForms();
            PathingManager::Install();
            MenuListener::Register();
            break;
        case SKSE::MessagingInterface::kPreLoadGame:
        case SKSE::MessagingInterface::kNewGame:
            PathingManager::GetSingleton()->Reset();
            break;
        default:
            break;
        }
    }
}

// AE (1.6.340+) loads plugins via this export. Version-independent through Address Library.
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
    SKSE::PluginVersionData v;
    v.PluginName("NPCPathingNG");
    v.PluginVersion(REL::Version{ 2, 4, 0 });
    v.AuthorName("karlo");
    v.UsesAddressLibrary(true);
    v.UsesStructsPost629(true);
    return v;
}();

// SE (1.5.97) loads plugins via Query/Load.
extern "C" __declspec(dllexport) bool SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "NPCPathingNG";
    a_info->version = 2;
    return true;
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    InitializeLogging();

    spdlog::info("NPCPathingNG v2.4.0 loading (runtime {})", REL::Module::get().version().string());

    if (auto* messaging = SKSE::GetMessagingInterface()) {
        messaging->RegisterListener(MessageHandler);
    } else {
        spdlog::error("NPCPathingNG: no messaging interface");
        return false;
    }

    spdlog::info("NPCPathingNG loaded");
    return true;
}
