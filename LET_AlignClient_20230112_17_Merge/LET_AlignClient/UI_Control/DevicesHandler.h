#pragma once
#include <vector>
#include <string>

class DevicesHandler
{
	public:
		enum Action
		{
			Disable,
			Enable,
			Toggle
		};

		bool enumerateNics(std::vector<std::string> &listadapter);
		bool toggleNic(const unsigned long &index, const Action &action);

		DevicesHandler();
		~DevicesHandler();
	private:
		void *handle;
};
