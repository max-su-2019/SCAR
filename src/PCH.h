#pragma once

#include "effolkronium/random.hpp"
using Random = effolkronium::random_static;

// clib
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <ShlObj_core.h>

using namespace std::literals;
using namespace REL::literals;

// Version
#include "Version.h"

// DKUtil
#include "DKUtil/Logger.hpp"

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)