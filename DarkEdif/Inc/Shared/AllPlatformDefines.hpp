#pragma once
// Define std::intXX_t types
#include <stdint.h>

// Include TCHAR, allowing both ANSI and Unicode functions
#include <tchar.h>

// Enables bitwise operations on an enum class. Basically just casts to numeric type, does the op, then casts back to enum class.
#define enum_class_is_a_bitmask(EnumClassType) \
	EnumClassType constexpr static operator|(EnumClassType lhs, EnumClassType rhs) { \
		return static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) | static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
	} \
	EnumClassType constexpr static operator&(EnumClassType lhs, EnumClassType rhs) { \
		return static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) & static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
	} \
	EnumClassType static operator|=(EnumClassType &lhs, EnumClassType rhs) { \
		lhs = static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) | static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
		return lhs; \
	} \
	EnumClassType static operator&=(EnumClassType &lhs, EnumClassType rhs) { \
		lhs = static_cast<EnumClassType>(static_cast<std::underlying_type<EnumClassType>::type>(lhs) & static_cast<std::underlying_type<EnumClassType>::type>(rhs)); \
		return lhs; \
	}

// Generic class for reading actions, conditions and expression parameters
struct ACEParamReader {
	virtual float GetFloat(int i) = 0;
	virtual const TCHAR * GetString(int i) = 0;
	virtual int GetInteger(int i) = 0;
};
