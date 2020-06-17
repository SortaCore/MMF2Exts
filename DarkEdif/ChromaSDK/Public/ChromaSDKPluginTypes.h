#pragma once

#include <Windows.h>
#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "RzErrors.h"
#include <vector>

namespace ChromaSDK
{

	typedef RZRESULT(*CHROMA_SDK_INIT)(void);
	typedef RZRESULT(*CHROMA_SDK_UNINIT)(void);
	typedef RZRESULT(*CHROMA_SDK_CREATE_EFFECT)(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_CREATE_CHROMA_LINK_EFFECT)(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_CREATE_KEYBOARD_EFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_CREATE_KEYPAD_EFFECT)(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_CREATE_HEADSET_EFFECT)(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_CREATE_MOUSE_EFFECT)(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_CREATE_MOUSEPAD_EFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
	typedef RZRESULT(*CHROMA_SDK_SET_EFFECT)(RZEFFECTID EffectId);
	typedef RZRESULT(*CHROMA_SDK_DELETE_EFFECT)(RZEFFECTID EffectId);
	typedef RZRESULT(*CHROMA_SDK_REGISTER_EVENT_NOTIFICATION)(HWND hWnd);
	typedef RZRESULT(*CHROMA_SDK_UNREGISTER_EVENT_NOTIFICATION)(void);
	typedef RZRESULT(*CHROMA_SDK_QUERY_DEVICE)(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE &DeviceInfo);

	enum EChromaSDKDeviceTypeEnum
	{
		DE_1D = 0,
		DE_2D,
	};

	enum class EChromaSDKDevice1DEnum
	{
		DE_ChromaLink = 0,
		DE_Headset,
		DE_Mousepad,
	};

	enum class EChromaSDKDevice2DEnum
	{
		DE_Keyboard = 0,
		DE_Keypad,
		DE_Mouse,
	};

	enum class EChromaSDKDeviceEnum
	{
		DE_ChromaLink = 0,
		DE_Headset,
		DE_Keyboard,
		DE_Keypad,
		DE_Mouse,
		DE_Mousepad,
	};

	enum EChromaSDKKeyboardKey
	{
		KK_ESC,
		KK_F1,
		KK_F2,
		KK_F3,
		KK_F4,
		KK_F5,
		KK_F6,
		KK_F7,
		KK_F8,
		KK_F9,
		KK_F10,
		KK_F11,
		KK_F12,
		KK_1,
		KK_2,
		KK_3,
		KK_4,
		KK_5,
		KK_6,
		KK_7,
		KK_8,
		KK_9,
		KK_0,
		KK_A,
		KK_B,
		KK_C,
		KK_D,
		KK_E,
		KK_F,
		KK_G,
		KK_H,
		KK_I,
		KK_J,
		KK_K,
		KK_L,
		KK_M,
		KK_N,
		KK_O,
		KK_P,
		KK_Q,
		KK_R,
		KK_S,
		KK_T,
		KK_U,
		KK_V,
		KK_W,
		KK_X,
		KK_Y,
		KK_Z,
		KK_NUMLOCK,
		KK_NUMPAD0,
		KK_NUMPAD1,
		KK_NUMPAD2,
		KK_NUMPAD3,
		KK_NUMPAD4,
		KK_NUMPAD5,
		KK_NUMPAD6,
		KK_NUMPAD7,
		KK_NUMPAD8,
		KK_NUMPAD9,
		KK_NUMPAD_DIVIDE,
		KK_NUMPAD_MULTIPLY,
		KK_NUMPAD_SUBTRACT,
		KK_NUMPAD_ADD,
		KK_NUMPAD_ENTER,
		KK_NUMPAD_DECIMAL,
		KK_PRINTSCREEN,
		KK_SCROLL,
		KK_PAUSE,
		KK_INSERT,
		KK_HOME,
		KK_PAGEUP,
		KK_DELETE,
		KK_END,
		KK_PAGEDOWN,
		KK_UP,
		KK_LEFT,
		KK_DOWN,
		KK_RIGHT,
		KK_TAB,
		KK_CAPSLOCK,
		KK_BACKSPACE,
		KK_ENTER,
		KK_LCTRL,
		KK_LWIN,
		KK_LALT,
		KK_SPACE,
		KK_RALT,
		KK_FN,
		KK_RMENU,
		KK_RCTRL,
		KK_LSHIFT,
		KK_RSHIFT,
		KK_MACRO1,
		KK_MACRO2,
		KK_MACRO3,
		KK_MACRO4,
		KK_MACRO5,
		KK_OEM_1,
		KK_OEM_2,
		KK_OEM_3,
		KK_OEM_4,
		KK_OEM_5,
		KK_OEM_6,
		KK_OEM_7,
		KK_OEM_8,
		KK_OEM_9,
		KK_OEM_10,
		KK_OEM_11,
		KK_EUR_1,
		KK_EUR_2,
		KK_JPN_1,
		KK_JPN_2,
		KK_JPN_3,
		KK_JPN_4,
		KK_JPN_5,
		KK_KOR_1,
		KK_KOR_2,
		KK_KOR_3,
		KK_KOR_4,
		KK_KOR_5,
		KK_KOR_6,
		KK_KOR_7,
		KK_LOGO,
		KK_INVALID,
	};

	enum EChromaSDKMouseLED
	{
		ML_SCROLLWHEEL,
		ML_LOGO,
		ML_BACKLIGHT,
		ML_LEFT_SIDE1,
		ML_LEFT_SIDE2,
		ML_LEFT_SIDE3,
		ML_LEFT_SIDE4,
		ML_LEFT_SIDE5,
		ML_LEFT_SIDE6,
		ML_LEFT_SIDE7,
		ML_BOTTOM1,
		ML_BOTTOM2,
		ML_BOTTOM3,
		ML_BOTTOM4,
		ML_BOTTOM5,
		ML_RIGHT_SIDE1,
		ML_RIGHT_SIDE2,
		ML_RIGHT_SIDE3,
		ML_RIGHT_SIDE4,
		ML_RIGHT_SIDE5,
		ML_RIGHT_SIDE6,
		ML_RIGHT_SIDE7,
	};

	struct FChromaSDKGuid
	{
		RZEFFECTID Data;

		//Constructor
		FChromaSDKGuid()
		{
			Data = RZEFFECTID();
			Data.Data1 = 0;
			Data.Data2 = 0;
			Data.Data3 = 0;
			for (int i = 0; i < 8; ++i)
			{
				Data.Data4[i] = 0;
			}
		}
	};


	struct FChromaSDKEffectResult
	{
		RZRESULT Result;

		FChromaSDKGuid EffectId;

		//Constructor
		FChromaSDKEffectResult()
		{
			Result = 0;
			EffectId = FChromaSDKGuid();
		}
	};

	struct FChromaSDKColors
	{
		std::vector<COLORREF> Colors;
	};

	struct FChromaSDKColorFrame1D
	{
		std::vector<COLORREF> Colors;

		float Duration;

		FChromaSDKColorFrame1D();
	};

	struct FChromaSDKColorFrame2D
	{
		std::vector<FChromaSDKColors> Colors;

		float Duration;

		FChromaSDKColorFrame2D();
	};
}
