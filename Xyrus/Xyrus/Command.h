#pragma once

#include <string>
#include <SFML/System/Vector2.hpp>

class Command
{
private:
	std::string _name{ "NONE" };
	std::string _type{ "NONE" };


public:
	Command();
	sf::Vector2i _mPos{ 0, 0 };
	Command(const std::string& name, const std::string& type, int x = 0, int y = 0);

	const std::string& name() const;
	const std::string& type() const;

	std::string toString() const;
};

