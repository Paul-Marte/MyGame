#include "Command.h"

Command::Command()
{
}

Command::Command(const std::string& name, const std::string& type)
	: _name(name)
	, _type(type)
{
}

const std::string& Command::name() const
{
	return _name;
}

const std::string& Command::type() const
{
	return _type;
}

std::string Command::toString() const
{
	return _name + ":" + _type;
}
