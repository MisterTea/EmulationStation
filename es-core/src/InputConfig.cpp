//  SPDX-License-Identifier: MIT
//
//  ES-DE Frontend
//  InputConfig.cpp
//
//  Input device configuration functions.
//

#include "InputConfig.h"

#include "Log.h"
#include "Settings.h"

#include <algorithm>
#include <pugixml.hpp>

InputConfig::InputConfig(int deviceId, const std::string& deviceName, const std::string& deviceGUID)
    : mDeviceId {deviceId}
    , mDeviceName {deviceName}
    , mDeviceGUID {deviceGUID}
{
}

std::string InputConfig::inputTypeToString(InputType type)
{
    switch (type) {
        case TYPE_AXIS:
            return "axis";
        case TYPE_BUTTON:
            return "button";
        case TYPE_KEY:
            return "key";
        case TYPE_TOUCH:
            return "touch-button";
        default:
            return "error";
    }
}

InputType InputConfig::stringToInputType(const std::string& type)
{
    if (type == "axis")
        return TYPE_AXIS;
    if (type == "button")
        return TYPE_BUTTON;
    if (type == "key")
        return TYPE_KEY;
    if (type == "touch-button")
        return TYPE_TOUCH;
    return TYPE_COUNT;
}

std::string InputConfig::toLower(std::string str)
{
    for (unsigned int i {0}; i < str.length(); ++i)
        str[i] = static_cast<char>(tolower(str[i]));

    return str;
}

void InputConfig::mapInput(const std::string& name, Input input)
{
    mNameMap[toLower(name)] = input;
}

void InputConfig::unmapInput(const std::string& name)
{
    auto it = mNameMap.find(toLower(name));
    if (it != mNameMap.cend())
        mNameMap.erase(it);
}

bool InputConfig::isMappedTo(const std::string& name, Input input)
{
    Input comp;
    if (!getInputByName(name, &comp))
        return false;

    if (comp.configured && comp.type == input.type && comp.id == input.id) {
        if (comp.type == TYPE_AXIS)
            return input.value == 0 || comp.value == input.value;
        else
            return true;
    }
    return false;
}

bool InputConfig::isMappedLike(const std::string& name, Input input)
{
    if (name == "left") {
        return isMappedTo("left", input) || isMappedTo("leftthumbstickleft", input) ||
               isMappedTo("rightthumbstickleft", input);
    }
    else if (name == "right") {
        return isMappedTo("right", input) || isMappedTo("leftthumbstickright", input) ||
               isMappedTo("rightthumbstickright", input);
    }
    else if (name == "up") {
        return isMappedTo("up", input) || isMappedTo("leftthumbstickup", input) ||
               isMappedTo("rightthumbstickup", input);
    }
    else if (name == "down") {
        return isMappedTo("down", input) || isMappedTo("leftthumbstickdown", input) ||
               isMappedTo("rightthumbstickdown", input);
    }
    else if (name == "leftshoulder") {
        return isMappedTo("leftshoulder", input) || isMappedTo("pageup", input);
    }
    else if (name == "rightshoulder") {
        return isMappedTo("rightshoulder", input) || isMappedTo("pagedown", input);
    }
    else if (name == "lefttrigger") {
        return isMappedTo("lefttrigger", input) || isMappedTo("home", input);
    }
    else if (name == "righttrigger") {
        return isMappedTo("righttrigger", input) || isMappedTo("end", input);
    }
    return isMappedTo(name, input);
}

std::vector<std::string> InputConfig::getMappedTo(Input input)
{
    std::vector<std::string> maps;

    for (auto it = mNameMap.cbegin(); it != mNameMap.cend(); ++it) {
        Input chk {it->second};

        if (!chk.configured)
            continue;

        if (chk.device == input.device && chk.type == input.type && chk.id == input.id) {
            if (input.type == TYPE_AXIS) {
                if (input.value == 0 || chk.value == input.value)
                    maps.push_back(it->first);
            }
            else {
                maps.push_back(it->first);
            }
        }
    }
    return maps;
}

bool InputConfig::getInputByName(const std::string& name, Input* result)
{
    std::string nameInput {name};

    if (Settings::getInstance()->getBool("InputSwapButtons") && mDeviceId != DEVICE_KEYBOARD) {
        if (name == "a")
            nameInput = "b";
        else if (name == "b")
            nameInput = "a";
        else if (name == "x")
            nameInput = "y";
        else if (name == "y")
            nameInput = "x";
    }

    auto it = mNameMap.find(toLower(nameInput));
    if (it != mNameMap.cend()) {
        *result = it->second;
        return true;
    }
    return false;
}

int InputConfig::getInputIDByName(const std::string& name)
{
    auto it = mNameMap.find(toLower(name));
    if (it != mNameMap.cend()) {
        return it->second.id;
    }
    return -1;
}

void InputConfig::loadFromXML(pugi::xml_node& node)
{
    clear();

    for (pugi::xml_node input {node.child("input")}; input; input = input.next_sibling("input")) {
        std::string name {input.attribute("name").as_string()};
        std::string type {input.attribute("type").as_string()};
        InputType typeEnum {stringToInputType(type)};

        if (typeEnum == TYPE_COUNT) {
            LOG(LogError) << "InputConfig load error - input of type \"" << type
                          << "\" is invalid! Skipping input \"" << name << "\".\n";
            continue;
        }

        int id {input.attribute("id").as_int()};
        int value {input.attribute("value").as_int()};

        if (value == 0) {
            LOG(LogWarning) << "InputConfig value is 0 for " << type << " " << id << "!\n";
        }

        mNameMap[toLower(name)] = Input(mDeviceId, typeEnum, id, value, true);
    }
}

void InputConfig::writeToXML(pugi::xml_node& parent)
{
    pugi::xml_node cfg {parent.append_child("inputConfig")};

    if (mDeviceId == DEVICE_KEYBOARD) {
        cfg.append_attribute("type") = "keyboard";
        cfg.append_attribute("deviceName") = "Keyboard";
    }
    else {
        cfg.append_attribute("type") = "controller";
        cfg.append_attribute("deviceName") = mDeviceName.c_str();
    }

    cfg.append_attribute("deviceGUID") = mDeviceGUID.c_str();

    for (auto it = mNameMap.cbegin(); it != mNameMap.cend(); ++it) {
        if (!it->second.configured)
            continue;

        pugi::xml_node input {cfg.append_child("input")};
        input.append_attribute("name") = it->first.c_str();
        input.append_attribute("type") = inputTypeToString(it->second.type).c_str();
        input.append_attribute("id").set_value(it->second.id);
        input.append_attribute("value").set_value(it->second.value);
    }
}

bool mamePortIsAnalog(const std::string& mamePort)
{
    return mamePort.find("AD_STICK") != std::string::npos ||
           mamePort.find("PADDLE") != std::string::npos;
}

std::string reverseInputName(const std::string& inputName)
{
    if (inputName == "up") return "down";
    if (inputName == "down") return "up";
    if (inputName == "left") return "right";
    if (inputName == "right") return "left";
    if (inputName == "leftanalogup" || inputName == "leftthumbstickup") return "leftthumbstickdown";
    if (inputName == "leftanalogdown" || inputName == "leftthumbstickdown") return "leftthumbstickup";
    if (inputName == "leftanalogleft" || inputName == "leftthumbstickleft") return "leftthumbstickright";
    if (inputName == "leftanalogright" || inputName == "leftthumbstickright") return "leftthumbstickleft";
    if (inputName == "rightanalogup" || inputName == "rightthumbstickup") return "rightthumbstickdown";
    if (inputName == "rightanalogdown" || inputName == "rightthumbstickdown") return "rightthumbstickup";
    if (inputName == "rightanalogleft" || inputName == "rightthumbstickleft") return "rightthumbstickright";
    if (inputName == "rightanalogright" || inputName == "rightthumbstickright") return "rightthumbstickleft";
    return "";
}

static std::string axisIndexToMameName(int index, bool positive, bool analog)
{
    if (analog) {
        switch (index) {
            case 0: return "XAXIS";
            case 1: return "YAXIS";
            case 2: return "ZAXIS";
            case 3: return "RXAXIS";
            case 4: return "RYAXIS";
            default: return "XAXIS";
        }
    }
    else {
        switch (index) {
            case 0: return positive ? "XAXIS_RIGHT_SWITCH" : "XAXIS_LEFT_SWITCH";
            case 1: return positive ? "YAXIS_DOWN_SWITCH" : "YAXIS_UP_SWITCH";
            case 2: return positive ? "ZAXIS_POS_SWITCH" : "ZAXIS_NEG_SWITCH";
            case 3: return positive ? "RXAXIS_POS_SWITCH" : "RXAXIS_NEG_SWITCH";
            case 4: return positive ? "RYAXIS_POS_SWITCH" : "RYAXIS_NEG_SWITCH";
            default: return positive ? "XAXIS_RIGHT_SWITCH" : "XAXIS_LEFT_SWITCH";
        }
    }
}

static std::string getMameKeyName(SDL_Keycode key)
{
    switch (key) {
        case SDLK_ESCAPE: return "ESC";
        case SDLK_MINUS: return "MINUS";
        case SDLK_EQUALS: return "EQUALS";
        case SDLK_BACKSPACE: return "BACKSPACE";
        case SDLK_TAB: return "TAB";
        case SDLK_LEFTBRACKET: return "OPENBRACE";
        case SDLK_RIGHTBRACKET: return "CLOSEBRACE";
        case SDLK_RETURN: return "ENTER";
        case SDLK_LCTRL: return "LCONTROL";
        case SDLK_RCTRL: return "RCONTROL";
        case SDLK_LSHIFT: return "LSHIFT";
        case SDLK_RSHIFT: return "RSHIFT";
        case SDLK_LALT: return "LALT";
        case SDLK_RALT: return "RALT";
        case SDLK_UP: return "UP";
        case SDLK_DOWN: return "DOWN";
        case SDLK_LEFT: return "LEFT";
        case SDLK_RIGHT: return "RIGHT";
        case SDLK_SPACE: return "SPACE";
        default: {
            std::string s = SDL_GetKeyName(key);
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            return s;
        }
    }
}

std::string Input::mameString(int deviceIndex, bool analog) const
{
    switch (type) {
        case TYPE_BUTTON:
            return "JOYCODE_" + std::to_string(deviceIndex + 1) + "_BUTTON" + std::to_string(id + 1);
        case TYPE_AXIS:
            return "JOYCODE_" + std::to_string(deviceIndex + 1) + "_" +
                   axisIndexToMameName(id, value > 0, analog);
        case TYPE_KEY:
            return "KEYCODE_" + getMameKeyName(static_cast<SDL_Keycode>(id));
        default:
            return "";
    }
}

std::vector<std::string> inputNameToMameStrings(const std::string& inputName, const std::string& machine, int player)
{
    std::string prefix = "P" + std::to_string(player + 1) + "_";

    if (inputName == "up")
        return {prefix + "HAT_UP", prefix + "JOYSTICK_UP"};
    if (inputName == "down")
        return {prefix + "HAT_DOWN", prefix + "JOYSTICK_DOWN"};
    if (inputName == "left")
        return {prefix + "HAT_LEFT", prefix + "JOYSTICK_LEFT"};
    if (inputName == "right")
        return {prefix + "HAT_RIGHT", prefix + "JOYSTICK_RIGHT"};

    if (inputName == "leftanalogup" || inputName == "leftthumbstickup")
        return {prefix + "JOYSTICK_UP", prefix + "JOYSTICKLEFT_UP", prefix + "AD_STICK_Y"};
    if (inputName == "leftanalogdown" || inputName == "leftthumbstickdown")
        return {prefix + "JOYSTICK_DOWN", prefix + "JOYSTICKLEFT_DOWN"};
    if (inputName == "leftanalogleft" || inputName == "leftthumbstickleft")
        return {prefix + "JOYSTICK_LEFT", prefix + "JOYSTICKLEFT_LEFT", prefix + "AD_STICK_X"};
    if (inputName == "leftanalogright" || inputName == "leftthumbstickright")
        return {prefix + "JOYSTICK_RIGHT", prefix + "JOYSTICKLEFT_RIGHT"};

    if (inputName == "rightanalogup" || inputName == "rightthumbstickup")
        return {prefix + "JOYSTICKRIGHT_UP", prefix + "PADDLE"};
    if (inputName == "rightanalogdown" || inputName == "rightthumbstickdown")
        return {prefix + "JOYSTICKRIGHT_DOWN"};
    if (inputName == "rightanalogleft" || inputName == "rightthumbstickleft")
        return {prefix + "JOYSTICKRIGHT_LEFT", prefix + "AD_STICK_Z"};
    if (inputName == "rightanalogright" || inputName == "rightthumbstickright")
        return {prefix + "JOYSTICKRIGHT_RIGHT"};

    if (inputName == "y") { // West button
        if (machine == "Street Fighter" || machine == "Mortal Kombat")
            return {prefix + "BUTTON1"}; // Jab / High Punch
        return {prefix + "BUTTON1"};
    }
    if (inputName == "b") { // South button
        if (machine == "Street Fighter")
            return {prefix + "BUTTON4"}; // Short / Low Kick
        if (machine == "Mortal Kombat")
            return {prefix + "BUTTON4"}; // Low Kick
        return {prefix + "BUTTON2"};
    }
    if (inputName == "a") { // East button
        if (machine == "Street Fighter" || machine == "Mortal Kombat")
            return {prefix + "BUTTON5"}; // Forward / High Kick
        return {prefix + "BUTTON3"};
    }
    if (inputName == "x") { // North button
        if (machine == "Street Fighter")
            return {prefix + "BUTTON2"}; // Strong / Medium Punch
        if (machine == "Mortal Kombat")
            return {prefix + "BUTTON3"}; // Block
        return {prefix + "BUTTON4"};
    }

    if (inputName == "rightshoulder") { // R1
        if (machine == "Street Fighter")
            return {prefix + "BUTTON3"}; // Fierce / High Punch
        if (machine == "Mortal Kombat")
            return {prefix + "BUTTON6"}; // Run
        if (machine == "snes")
            return {prefix + "BUTTON5"};
        if (machine == "gba")
            return {prefix + "BUTTON4"};
        if (machine == "psx")
            return {prefix + "BUTTON5"};
        return {prefix + "BUTTON5"};
    }
    if (inputName == "righttrigger") { // R2
        if (machine == "Street Fighter")
            return {prefix + "BUTTON6"}; // Roundhouse / High Kick
        if (machine == "Mortal Kombat")
            return {prefix + "BUTTON2"}; // Low Punch
        if (machine == "psx")
            return {prefix + "BUTTON7"};
        return {prefix + "BUTTON6"};
    }

    if (inputName == "leftshoulder") { // L1
        if (machine == "snes")
            return {prefix + "BUTTON6"};
        if (machine == "gba")
            return {prefix + "BUTTON3"};
        if (machine == "psx")
            return {prefix + "BUTTON6"};
        return {};
    }
    if (inputName == "lefttrigger") { // L2
        if (machine == "psx")
            return {prefix + "BUTTON8"};
        return {};
    }
    if (inputName == "leftthumb" || inputName == "leftthumbstickclick") { // L3
        if (machine == "psx")
            return {prefix + "BUTTON10"};
        return {};
    }
    if (inputName == "rightthumb" || inputName == "rightthumbstickclick") { // R3
        if (machine == "psx")
            return {prefix + "BUTTON9"};
        return {};
    }

    if (inputName == "start") {
        return {prefix + "START", "START" + std::to_string(player + 1)};
    }
    if (inputName == "back" || inputName == "select") {
        return {prefix + "SELECT", "COIN" + std::to_string(player + 1)};
    }

    return {};
}

std::string InputConfig::getMameNameForCategory(const std::string& inputName, const std::string& mamePort, const std::string& sequence, int deviceIndex)
{
    Input input;
    if (!getInputByName(inputName, &input))
        return "";

    bool analog = mamePortIsAnalog(mamePort);
    if (!analog) {
        return input.mameString(deviceIndex, false);
    }
    else {
        if (sequence == "standard") {
            if (input.type != TYPE_AXIS)
                return "";
            return input.mameString(deviceIndex, true);
        }
        else {
            if (input.type == TYPE_AXIS)
                return "";
            return input.mameString(deviceIndex, false);
        }
    }
}
