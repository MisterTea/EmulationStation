//  SPDX-License-Identifier: MIT
//
//  ES-DE Frontend
//  InputConfig.h
//
//  Input device configuration functions.
//

#ifndef ES_CORE_INPUT_CONFIG_H
#define ES_CORE_INPUT_CONFIG_H

#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_keyboard.h>
#include <map>
#include <sstream>
#include <vector>

#define DEVICE_KEYBOARD -1
#define DEVICE_TOUCH -2

enum InputType {
    TYPE_AXIS,
    TYPE_BUTTON,
    TYPE_KEY,
    TYPE_TOUCH,
    TYPE_COUNT
};

namespace pugi
{
    class xml_node;
}

struct Input {
public:
    int device;
    InputType type;
    int id;
    int value;
    bool configured;

    Input()
    {
        device = DEVICE_KEYBOARD;
        configured = false;
        id = -1;
        value = -999;
        type = TYPE_COUNT;
    }

    Input(int dev, InputType t, int i, int val, bool conf)
        : device(dev)
        , type(t)
        , id(i)
        , value(val)
        , configured(conf)
    {
    }

    std::string string()
    {
        std::stringstream stream;
        switch (type) {
            case TYPE_AXIS: {
                char signChar = ' ';
                if (value > 0)
                    signChar = '+';
                else if (value < 0)
                    signChar = '-';
                stream << "Axis " << id << signChar;
                break;
            }
            case TYPE_BUTTON: {
                stream << "Button " << id;
                break;
            }
            case TYPE_KEY: {
                stream << "Key " << SDL_GetKeyName((SDL_Keycode)id);
                break;
            }
            case TYPE_TOUCH: {
                stream << "Button " << id;
                break;
            }
            default: {
                stream << "Input to string error";
                break;
            }
        }

        return stream.str();
    }

    std::string mameString(int deviceIndex, bool analog) const;
};

bool mamePortIsAnalog(const std::string& mamePort);
std::vector<std::string> inputNameToMameStrings(const std::string& inputName, const std::string& machine, int player);
std::string reverseInputName(const std::string& inputName);

class InputConfig
{
public:
    InputConfig(int deviceId, const std::string& deviceName, const std::string& deviceGUID);

    // Utility functions.
    std::string inputTypeToString(InputType type);
    InputType stringToInputType(const std::string& type);
    std::string toLower(std::string str);

    void clear() { mNameMap.clear(); }
    bool isConfigured() { return mNameMap.size() > 0; }

    void mapInput(const std::string& name, Input input);
    void unmapInput(const std::string& name); // Unmap all Inputs mapped to this name.

    // Returns true if Input is mapped to this name, false otherwise.
    bool isMappedTo(const std::string& name, Input input);
    bool isMappedLike(const std::string& name, Input input);

    // Returns a list of names this input is mapped to.
    std::vector<std::string> getMappedTo(Input input);

    // Returns true if there is an Input mapped to this name, false otherwise.
    // Writes Input mapped to this name to result if true.
    bool getInputByName(const std::string& name, Input* result);
    int getInputIDByName(const std::string& name);

    void loadFromXML(pugi::xml_node& root);
    void writeToXML(pugi::xml_node& parent);

    int getDeviceId() const { return mDeviceId; }
    const std::string& getDeviceName() { return mDeviceName; }
    const std::string& getDeviceGUIDString() { return mDeviceGUID; }

    std::string getMameNameForCategory(const std::string& inputName, const std::string& mamePort, const std::string& sequence, int deviceIndex);

private:
    std::map<std::string, Input> mNameMap;
    const int mDeviceId;
    const std::string mDeviceName;
    const std::string mDeviceGUID;
};

bool mamePortIsAnalog(const std::string& mamePort);
std::string reverseInputName(const std::string& inputName);
std::vector<std::string> inputNameToMameStrings(const std::string& inputName, const std::string& machine, int player);

#endif // ES_CORE_INPUT_CONFIG_H
