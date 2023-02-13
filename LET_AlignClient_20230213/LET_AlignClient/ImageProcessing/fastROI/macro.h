#pragma once


#include <stdexcept>
#include <string>

#ifndef FAST_MACRO
#define FAST_MACRO

#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#pragma warning(disable : 4996)

#define HVERROR(name, message) \
std::string name;name += "error:";name += __func__;name += ":";name += std::to_string(__LINE__);name += ":";name += message;throw std::runtime_error(name);\

#endif