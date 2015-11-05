#include "InputConfig.h"
#include <string>
#include <algorithm>
#include <SDL.h>
#include <iostream>
#include "Log.h"
#include "InputManager.h"

std::string inputCategoryToString(InputCategory category) {
  switch (category) {
    case INPUT_UP:
      return "up";
    case INPUT_DOWN:
      return "down";
    case INPUT_LEFT:
      return "left";
    case INPUT_RIGHT:
      return "right";
    case INPUT_JOYSTICK1_Y:
      return "j1y";
    case INPUT_JOYSTICK1_X:
      return "j1x";
    case INPUT_JOYSTICK2_Y:
      return "j2y";
    case INPUT_JOYSTICK2_X:
      return "j2x";
    case INPUT_4B_LEFT:
      return "4bl";
    case INPUT_4B_DOWN:
      return "4bd";
    case INPUT_4B_RIGHT:
      return "4br";
    case INPUT_4B_UP:
      return "4bu";
    case INPUT_6B_BOTTOM_LEFT:
      return "6bbl";
    case INPUT_6B_BOTTOM_CENTER:
      return "6bbc";
    case INPUT_6B_BOTTOM_RIGHT:
      return "6bbr";
    case INPUT_6B_TOP_LEFT:
      return "6btl";
    case INPUT_6B_TOP_CENTER:
      return "6btc";
    case INPUT_6B_TOP_RIGHT:
      return "6btr";
    case INPUT_START:
      return "start";
    case INPUT_SELECT:
      return "select";
    case INPUT_L1:
      return "l1";
    case INPUT_R1:
      return "r1";
    case INPUT_L2:
      return "l2";
    case INPUT_R2:
      return "r2";
    case INPUT_L3:
      return "l3";
    case INPUT_R3:
      return "r3";
    case INPUT_HOTKEY:
      return "hotkey";
    case INPUT_END:
      return "end";
  }
}

InputCategory stringToInputCategory(const std::string &name) {
  if (name == "up") { return INPUT_UP; }
  if (name == "down") { return INPUT_DOWN; }
  if (name == "left") { return INPUT_LEFT; }
  if (name == "right") { return INPUT_RIGHT; }
  if (name == "j1y") { return INPUT_JOYSTICK1_Y; }
  if (name == "j1x") { return INPUT_JOYSTICK1_X; }
  if (name == "j2y") { return INPUT_JOYSTICK2_Y; }
  if (name == "j2x") { return INPUT_JOYSTICK2_X; }
  if (name == "4bl") { return INPUT_4B_LEFT; }
  if (name == "4bd") { return INPUT_4B_DOWN; }
  if (name == "4br") { return INPUT_4B_RIGHT; }
  if (name == "4bu") { return INPUT_4B_UP; }
  if (name == "6bbl") { return INPUT_6B_BOTTOM_LEFT; }
  if (name == "6bbc") { return INPUT_6B_BOTTOM_CENTER; }
  if (name == "6bbr") { return INPUT_6B_BOTTOM_RIGHT; }
  if (name == "6btl") { return INPUT_6B_TOP_LEFT; }
  if (name == "6btc") { return INPUT_6B_TOP_CENTER; }
  if (name == "6btr") { return INPUT_6B_TOP_RIGHT; }
  if (name == "start") { return INPUT_START; }
  if (name == "select") { return INPUT_SELECT; }
  if (name == "l1") { return INPUT_L1; }
  if (name == "r1") { return INPUT_R1; }
  if (name == "l2") { return INPUT_L2; }
  if (name == "r2") { return INPUT_R2; }
  if (name == "l3") { return INPUT_L3; }
  if (name == "r3") { return INPUT_R3; }
  if (name == "hotkey") { return INPUT_HOTKEY; }
  return INPUT_END;
}

//some util functions
std::string inputTypeToString(InputType type)
{
	switch(type)
	{
	case TYPE_AXIS:
		return "axis";
	case TYPE_BUTTON:
		return "button";
	case TYPE_HAT:
		return "hat";
	case TYPE_KEY:
		return "key";
	default:
		return "error";
	}
}

InputType stringToInputType(const std::string& type)
{
	if(type == "axis")
		return TYPE_AXIS;
	if(type == "button")
		return TYPE_BUTTON;
	if(type == "hat")
		return TYPE_HAT;
	if(type == "key")
		return TYPE_KEY;
	return TYPE_COUNT;
}


std::string toLower(std::string str)
{
	for(unsigned int i = 0; i < str.length(); i++)
	{
		str[i] = tolower(str[i]);
	}

	return str;
}
//end util functions

InputConfig::InputConfig(int deviceId, int deviceIndex, const std::string& deviceName, const std::string& deviceGUID) : mDeviceId(deviceId), mDeviceIndex(deviceIndex), mDeviceName(deviceName), mDeviceGUID(deviceGUID)
{
}

void InputConfig::clear()
{
	mNameMap.clear();
}

bool InputConfig::isConfigured()
{
	return mNameMap.size() > 0;
}

void InputConfig::mapInput(InputCategory name, Input input)
{
	mNameMap[name] = input;
}

void InputConfig::unmapInput(InputCategory name)
{
	auto it = mNameMap.find(name);
	if(it != mNameMap.end())
		mNameMap.erase(it);
}

bool InputConfig::getInputByName(InputCategory name, Input* result)
{
	auto it = mNameMap.find(name);
	if(it != mNameMap.end())
	{
		*result = it->second;
		return true;
	}

	return false;
}

bool InputConfig::isMappedTo(InputCategory name, Input input)
{
	Input comp;
	if(!getInputByName(name, &comp))
		return false;

	if(comp.configured && comp.type == input.type && comp.id == input.id)
	{
		if(comp.type == TYPE_HAT)
		{
			return (input.value == 0 || input.value & comp.value);
		}

		if(comp.type == TYPE_AXIS)
		{
			return input.value == 0 || comp.value == input.value;
		}else{
			return true;
		}
	}
	return false;
}

std::vector<InputCategory> InputConfig::getMappedTo(Input input)
{
	std::vector<InputCategory> maps;

	typedef std::map<InputCategory, Input>::iterator it_type;
	for(it_type iterator = mNameMap.begin(); iterator != mNameMap.end(); iterator++)
	{
		Input chk = iterator->second;

		if(!chk.configured)
			continue;

		if(chk.device == input.device && chk.type == input.type && chk.id == input.id)
		{
			if(chk.type == TYPE_HAT)
			{
				if(input.value == 0 || input.value & chk.value)
				{
					maps.push_back(iterator->first);
				}
				continue;
			}

			if(input.type == TYPE_AXIS)
			{
				if(input.value == 0 || chk.value == input.value)
					maps.push_back(iterator->first);
			}else{
				maps.push_back(iterator->first);
			}
		}
	}

	return maps;
}

void InputConfig::loadFromXML(pugi::xml_node node)
{
	clear();

	for(pugi::xml_node input = node.child("input"); input; input = input.next_sibling("input"))
	{
		InputCategory name = stringToInputCategory(input.attribute("name").as_string());
    if (name == INPUT_END) {
			LOG(LogError) << "Invalid input category name " << input.attribute("name").as_string();
      continue;
    }
		std::string type = input.attribute("type").as_string();
		InputType typeEnum = stringToInputType(type);

		if(typeEnum == TYPE_COUNT)
		{
			LOG(LogError) << "InputConfig load error - input of type \"" << type << "\" is invalid! Skipping input \"" << name << "\".\n";
			continue;
		}

		int id = input.attribute("id").as_int();
		int value = input.attribute("value").as_int();

        if(value == 0) {
			LOG(LogWarning) << "WARNING: InputConfig value is 0 for " << type << " " << id << "!\n";
        }

		mNameMap[name] = Input(mDeviceId, typeEnum, id, value, true);
	}
}

void InputConfig::writeToXML(pugi::xml_node parent)
{
	pugi::xml_node cfg = parent.append_child("inputConfig");

	if(mDeviceId == DEVICE_KEYBOARD)
	{
		cfg.append_attribute("type") = "keyboard";
		cfg.append_attribute("deviceName") = "Keyboard";
	}else{
		cfg.append_attribute("type") = "joystick";
		cfg.append_attribute("deviceName") = mDeviceName.c_str();
	}

	cfg.append_attribute("deviceGUID") = mDeviceGUID.c_str();

	typedef std::map<InputCategory, Input>::iterator it_type;
	for(it_type iterator = mNameMap.begin(); iterator != mNameMap.end(); iterator++)
	{
		if(!iterator->second.configured)
			continue;

		pugi::xml_node input = cfg.append_child("input");
		input.append_attribute("name") = inputCategoryToString(iterator->first).c_str();
		input.append_attribute("type") = inputTypeToString(iterator->second.type).c_str();
		input.append_attribute("id").set_value(iterator->second.id);
		input.append_attribute("value").set_value(iterator->second.value);
	}
}
