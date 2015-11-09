#include "InputConfig.h"
#include <string>
#include <algorithm>
#include <SDL.h>
#include <iostream>
#include "Log.h"
#include "InputManager.h"

bool mamePortIsAnalog(const std::string &mamePort) {
  return
    mamePort == "P1_AD_STICK_X" ||
    mamePort == "P1_AD_STICK_Y" ||
    mamePort == "P1_AD_STICK_Z" ||
    mamePort == "P1_PADDLE" ||
    mamePort == "P2_AD_STICK_X" ||
    mamePort == "P2_AD_STICK_Y" ||
    mamePort == "P2_AD_STICK_Z" ||
    mamePort == "P2_PADDLE";
}

InputCategory reverseInputCategory(InputCategory ic) {
  switch (ic) {
  case INPUT_UP:
    return INPUT_DOWN;
  case INPUT_DOWN:
    return INPUT_UP;
  case INPUT_LEFT:
    return INPUT_RIGHT;
  case INPUT_RIGHT:
    return INPUT_LEFT;
  case INPUT_JOYSTICK1_UP:
    return INPUT_JOYSTICK1_DOWN;
  case INPUT_JOYSTICK1_DOWN:
    return INPUT_JOYSTICK1_UP;
  case INPUT_JOYSTICK1_LEFT:
    return INPUT_JOYSTICK1_RIGHT;
  case INPUT_JOYSTICK1_RIGHT:
    return INPUT_JOYSTICK1_LEFT;
  case INPUT_JOYSTICK2_UP:
    return INPUT_JOYSTICK2_DOWN;
  case INPUT_JOYSTICK2_DOWN:
    return INPUT_JOYSTICK2_UP;
  case INPUT_JOYSTICK2_LEFT:
    return INPUT_JOYSTICK2_RIGHT;
  case INPUT_JOYSTICK2_RIGHT:
    return INPUT_JOYSTICK2_LEFT;
  default:
    return INPUT_END;
  }
}

std::string axisIndexToMameName(int index, bool positive, bool analog) {
  if (analog) {
    switch (index) {
    case 0:
      return "XAXIS";
    case 1:
      return "YAXIS";
    case 2:
      return "ZAXIS";
    case 3:
      return "RXAXIS";
    case 4:
      return "RYAXIS";
    default:
      return "OOPS";
    }
  } else {
    switch (index) {
    case 0:
      return positive?"XAXIS_RIGHT_SWITCH":"XAXIS_LEFT_SWITCH";
    case 1:
      return positive?"YAXIS_UP_SWITCH":"YAXIS_DOWN_SWITCH";
    case 2:
      return positive?"ZAXIS_POS_SWITCH":"ZAXIS_NEG_SWITCH";
    case 3:
      return positive?"RXAXIS_POS_SWITCH":"RXAXIS_NEG_SWITCH";
    case 4:
      return positive?"RYAXIS_POS_SWITCH":"RYAXIS_NEG_SWITCH";
    default:
      return "OOPS";
    }
  }
}

std::string getMameKeyName(SDL_Keycode key) {
  switch (key) {
  case SDLK_ESCAPE:
    return "ESC";
  case SDLK_MINUS:
    return "MINUS";
  case SDLK_EQUALS:
    return "EQUALS";
  case SDLK_BACKSPACE:
    return "BACKSPACE";
  case SDLK_TAB:
    return "TAB";
  case SDLK_LEFTBRACKET:
    return "OPENBRACE";
  case SDLK_RIGHTBRACKET:
    return "CLOSEBRACE";
  case SDLK_RETURN:
    return "RETURN";
  case SDLK_LCTRL:
    return "LCONTROL";
  case SDLK_RCTRL:
    return "RCONTROL";
  case SDLK_LSHIFT:
    return "LSHIFT";
  case SDLK_RSHIFT:
    return "RSHIFT";
    //TODO: There are more, but I covered the major ones.
  default:
    std::string s = SDL_GetKeyName(key);
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
  }
}

std::string Input::mameString(int deviceIndex, bool analog) {
  switch(type)
  {
  case TYPE_BUTTON:
    return std::string("JOYCODE_") + std::to_string(deviceIndex+1) + std::string("_BUTTON") + std::to_string(id+1);
  case TYPE_AXIS:
    return std::string("JOYCODE_") + std::to_string(deviceIndex+1) + std::string("_") + axisIndexToMameName(id,value>0, analog);
  case TYPE_HAT:
    return std::string("JOYCODE_") + std::to_string(deviceIndex+1) + std::string("_HAT") + std::to_string(id+1) + getHatDirString();
  case TYPE_KEY:
    return std::string("KEYCODE_") + getMameKeyName((SDL_Keycode)id);
  case TYPE_COUNT:
    return std::string("OOPS");
  }
}

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
    case INPUT_JOYSTICK1_UP:
      return "j1up";
    case INPUT_JOYSTICK1_DOWN:
      return "j1down";
    case INPUT_JOYSTICK1_LEFT:
      return "j1left";
    case INPUT_JOYSTICK1_RIGHT:
      return "j1right";
    case INPUT_JOYSTICK2_UP:
      return "j2up";
    case INPUT_JOYSTICK2_DOWN:
      return "j2down";
    case INPUT_JOYSTICK2_LEFT:
      return "j2left";
    case INPUT_JOYSTICK2_RIGHT:
      return "j2right";
    case INPUT_4B_LEFT:
      return "4bl";
    case INPUT_4B_DOWN:
      return "4bd";
    case INPUT_4B_RIGHT:
      return "4br";
    case INPUT_4B_UP:
      return "4bu";
    case INPUT_6B_TOP_RIGHT:
      return "6btr";
    case INPUT_6B_BOTTOM_RIGHT:
      return "6bbr";
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

std::vector<std::string> inputCategoryToMameStrings(InputCategory category, int player) {
  std::string retval = std::string("P") + std::to_string(player+1) + std::string("_");
  switch (category) {
    case INPUT_UP:
      return {retval + std::string("HAT_UP")};
    case INPUT_DOWN:
      return {retval + std::string("HAT_DOWN")};
    case INPUT_LEFT:
      return {retval + std::string("HAT_LEFT")};
    case INPUT_RIGHT:
      return {retval + std::string("HAT_RIGHT")};
    case INPUT_JOYSTICK1_UP:
      return {retval + std::string("JOYSTICK_UP"),
          retval + std::string("JOYSTICKLEFT_UP"),
          retval + std::string("AD_STICK_Y")};
    case INPUT_JOYSTICK1_DOWN:
      return {retval + std::string("JOYSTICK_DOWN"),
          retval + std::string("JOYSTICKLEFT_DOWN")};
    case INPUT_JOYSTICK1_LEFT:
      return {retval + std::string("JOYSTICK_LEFT"),
          retval + std::string("JOYSTICKLEFT_LEFT"),
          retval + std::string("AD_STICK_X")};
    case INPUT_JOYSTICK1_RIGHT:
      return {retval + std::string("JOYSTICK_RIGHT"),
          retval + std::string("JOYSTICKLEFT_RIGHT")};
    case INPUT_JOYSTICK2_UP:
      return {retval + std::string("JOYSTICKRIGHT_UP"),
          retval + std::string("PADDLE")};
    case INPUT_JOYSTICK2_DOWN:
      return {retval + std::string("JOYSTICKRIGHT_DOWN")};
    case INPUT_JOYSTICK2_LEFT:
      return {retval + std::string("JOYSTICKRIGHT_LEFT"),
          retval + std::string("AD_STICK_Z")};
    case INPUT_JOYSTICK2_RIGHT:
      return {retval + std::string("JOYSTICKRIGHT_RIGHT")};
    case INPUT_4B_LEFT:
      return {retval + std::string("BUTTON1")};
    case INPUT_4B_DOWN:
      return {retval + std::string("BUTTON2")};
    case INPUT_4B_RIGHT:
      return {retval + std::string("BUTTON3")};
    case INPUT_4B_UP:
      return {retval + std::string("BUTTON4")};
    case INPUT_6B_TOP_RIGHT:
      return {retval + std::string("BUTTON5")};
    case INPUT_6B_BOTTOM_RIGHT:
      return {retval + std::string("BUTTON6")};
    case INPUT_START:
      return {retval + std::string("START")};
    case INPUT_SELECT:
      return {retval + std::string("SELECT")};
    case INPUT_L1:
      return {retval + std::string("SHOULDER_BUTTON1")};
    case INPUT_R1:
      return {retval + std::string("SHOULDER_BUTTON2")};
    case INPUT_L2:
      return {retval + std::string("SHOULDER_BUTTON3")};
    case INPUT_R2:
      return {retval + std::string("SHOULDER_BUTTON4")};
    case INPUT_L3:
      return {retval + std::string("JOYSTICK_BUTTON1")};
    case INPUT_R3:
      return {retval + std::string("JOYSTICK_BUTTON2")};
    case INPUT_HOTKEY:
      return {};
    case INPUT_END:
      return {};
  }
}

InputCategory stringToInputCategory(const std::string &name) {
  if (name == "up") { return INPUT_UP; }
  if (name == "down") { return INPUT_DOWN; }
  if (name == "left") { return INPUT_LEFT; }
  if (name == "right") { return INPUT_RIGHT; }
  if (name == "j1up") { return INPUT_JOYSTICK1_UP; }
  if (name == "j1down") { return INPUT_JOYSTICK1_DOWN; }
  if (name == "j1left") { return INPUT_JOYSTICK1_LEFT; }
  if (name == "j1right") { return INPUT_JOYSTICK1_RIGHT; }
  if (name == "j2up") { return INPUT_JOYSTICK2_UP; }
  if (name == "j2down") { return INPUT_JOYSTICK2_DOWN; }
  if (name == "j2left") { return INPUT_JOYSTICK2_LEFT; }
  if (name == "j2right") { return INPUT_JOYSTICK2_RIGHT; }
  if (name == "4bl") { return INPUT_4B_LEFT; }
  if (name == "4bd") { return INPUT_4B_DOWN; }
  if (name == "4br") { return INPUT_4B_RIGHT; }
  if (name == "4bu") { return INPUT_4B_UP; }
  if (name == "6btr") { return INPUT_6B_TOP_RIGHT; }
  if (name == "6bbr") { return INPUT_6B_BOTTOM_RIGHT; }
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

std::string InputConfig::getMameNameForCategory(InputCategory ic, const std::string &mamePort, const std::string &sequence, int deviceIndex) {
  if (mNameMap.find(ic) == mNameMap.end()) {
    return "";
  }
  const Input &input = mNameMap[ic];
  bool analog = mamePortIsAnalog(mamePort);
  if (!analog) {
    return mNameMap[ic].mameString(deviceIndex, false);
  } else {
    // If analog, it depends on the sequence
    if (sequence == "standard") {
      // Only accept analog inputs
      if (input.type != TYPE_AXIS) {
        return "";
      }
      return mNameMap[ic].mameString(deviceIndex, analog);
    } else {
      // Only accept non-analog inputs
      if (input.type == TYPE_AXIS) {
        return "";
      }
      return mNameMap[ic].mameString(deviceIndex, analog);
    }
  }
}
