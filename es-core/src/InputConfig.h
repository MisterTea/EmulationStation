#ifndef _INPUTCONFIG_H_
#define _INPUTCONFIG_H_

#include <map>
#include <vector>
#include <string>
#include <SDL.h>
#include <sstream>
#include "pugixml/pugixml.hpp"

#define DEVICE_KEYBOARD -1

enum InputType
{
	TYPE_AXIS,
	TYPE_BUTTON,
	TYPE_HAT,
	TYPE_KEY,
	TYPE_COUNT
};

enum InputCategory {
  INPUT_UP,
  INPUT_DOWN,
  INPUT_LEFT,
  INPUT_RIGHT,
  INPUT_JOYSTICK1_Y,
  INPUT_JOYSTICK1_X,
  INPUT_JOYSTICK2_Y,
  INPUT_JOYSTICK2_X,
  INPUT_4B_LEFT,
  INPUT_4B_DOWN,
  INPUT_4B_RIGHT,
  INPUT_4B_UP,
  INPUT_6B_BOTTOM_LEFT,
  INPUT_6B_BOTTOM_CENTER,
  INPUT_6B_BOTTOM_RIGHT,
  INPUT_6B_TOP_LEFT,
  INPUT_6B_TOP_CENTER,
  INPUT_6B_TOP_RIGHT,
  INPUT_START,
  INPUT_SELECT,
  INPUT_L1,
  INPUT_R1,
  INPUT_L2,
  INPUT_R2,
  INPUT_L3,
  INPUT_R3,
  INPUT_HOTKEY,
  INPUT_END
};

std::string inputCategoryToString(InputCategory category);

struct Input
{
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

	Input(int dev, InputType t, int i, int val, bool conf) : device(dev), type(t), id(i), value(val), configured(conf)
	{
	}

	InputCategory getHatDir(int val)
	{
		if(val & SDL_HAT_UP)
			return INPUT_UP;
		else if(val & SDL_HAT_DOWN)
			return INPUT_DOWN;
		else if(val & SDL_HAT_LEFT)
			return INPUT_LEFT;
		else if(val & SDL_HAT_RIGHT)
			return INPUT_RIGHT;
		return INPUT_END;
	}

	std::string string()
	{
		std::stringstream stream;
		switch(type)
		{
			case TYPE_BUTTON:
				stream << "Button " << id;
				break;
			case TYPE_AXIS:
				stream << "Axis " << id << (value > 0 ? "+" : "-");
				break;
			case TYPE_HAT:
				stream << "Hat " << id << " " << getHatDir(value);
				break;
			case TYPE_KEY:
				stream << "Key " << SDL_GetKeyName((SDL_Keycode)id);
				break;
			default:
				stream << "Input to string error";
				break;
		}

		return stream.str();
	}
};

class InputConfig
{
public:
	InputConfig(int deviceId, int deviceIndex, const std::string& deviceName, const std::string& deviceGUID);

	void clear();
	void mapInput(InputCategory name, Input input);
	void unmapInput(InputCategory name); // unmap all Inputs mapped to this name

	inline int getDeviceId() const { return mDeviceId; };

	inline int getDeviceIndex() const { return mDeviceIndex; };
	inline const std::string& getDeviceName() { return mDeviceName; }
	inline const std::string& getDeviceGUIDString() { return mDeviceGUID; }

	//Returns true if Input is mapped to this name, false otherwise.
	bool isMappedTo(InputCategory name, Input input);

	//Returns a list of names this input is mapped to.
	std::vector<InputCategory> getMappedTo(Input input);

	void loadFromXML(pugi::xml_node root);
	void writeToXML(pugi::xml_node parent);

	bool isConfigured();

private:
	// Returns true if there is an Input mapped to this name, false otherwise.
	// Writes Input mapped to this name to result if true.
	bool getInputByName(InputCategory name, Input* result);

	std::map<InputCategory, Input> mNameMap;
	const int mDeviceId;
	const int mDeviceIndex;
	const std::string mDeviceName;
	const std::string mDeviceGUID;
};

#endif
