#include "modules/ModeModule.h"

#include <string_view>

void ModeModule::begin() {}
void ModeModule::handle() {}
void ModeModule::onHomeAssistant(JsonDocument &discovery, std::string_view unique) {}
void ModeModule::onReceive(JsonObjectConst payload, std::string_view source) {}
