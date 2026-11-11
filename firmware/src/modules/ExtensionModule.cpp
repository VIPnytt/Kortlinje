#include "modules/ExtensionModule.h"

void ExtensionModule::begin() {}
void ExtensionModule::configure() {}
void ExtensionModule::handle() {}
void ExtensionModule::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique) {}
void ExtensionModule::onReceive(JsonObjectConst payload, std::string_view source) {}
void ExtensionModule::onTransmit(JsonObjectConst payload, std::string_view source) {}
