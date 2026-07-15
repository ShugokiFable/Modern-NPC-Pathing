#pragma once

#include "SKSE/Impl/PCH.h"
#include "SKSE/SKSE.h"

// TESFaction must be included before TESForm for LookupByID<TESFaction> template instantiation
#include "RE/T/TESFaction.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <numbers>
#include <unordered_map>

using namespace std::literals;
