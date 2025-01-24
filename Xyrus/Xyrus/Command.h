#pragma once

#include <string>

class Command
{
private:
	std::string _name{ "NONE" };
	std::string _type{ "NONE" };

public:
	Command();
	Command(const std::string& name, const std::string& type);

	const std::string& name() const;
	const std::string& type() const;

	std::string toString() const;
};

