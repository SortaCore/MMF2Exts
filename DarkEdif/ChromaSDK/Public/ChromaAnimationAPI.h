#pragma once

#include "ChromaSDKPluginTypes.h"

# ifdef _WIN64
#define CHROMA_EDITOR_DLL	_T("CChromaEditorLibrary64.dll")
#else
#define CHROMA_EDITOR_DLL	_T("CChromaEditorLibrary.dll")
#endif

/* Setup log mechanism */
typedef void(*DebugLogPtr)(const char*);
void LogDebug(const char* text, ...);
void LogError(const char* text, ...);
/* End of setup log mechanism */
                
#pragma region API typedefs
/*
	Adds a frame to the `Chroma` animation and sets the `duration` (in seconds). 
	The `color` is expected to be an array of the dimensions for the `deviceType/device`. 
	The `length` parameter is the size of the `color` array. For `EChromaSDKDevice1DEnum` 
	the array size should be `MAX LEDS`. For `EChromaSDKDevice2DEnum` the array 
	size should be `MAX ROW` * `MAX COLUMN`. Returns the animation id upon 
	success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_ADD_FRAME)(int animationId, float duration, int* colors, int length);
/*
	Add source color to target where color is not black for all frames, reference 
	source and target by id.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Add source color to target where color is not black for all frames, reference 
	source and target by name.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Add source color to target where color is not black for all frames starting 
	at offset for the length of the source, reference source and target by 
	id.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Add source color to target where color is not black for all frames starting 
	at offset for the length of the source, reference source and target by 
	name.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Add source color to target where color is not black for the source frame 
	and target offset frame, reference source and target by id.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_OFFSET)(int sourceAnimationId, int targetAnimationId, int frameId, int offset);
/*
	Add source color to target where color is not black for the source frame 
	and target offset frame, reference source and target by name.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_ADD_NON_ZERO_ALL_KEYS_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double offset);
/*
	Add source color to target where the target color is not black for all frames, 
	reference source and target by id.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Add source color to target where the target color is not black for all frames, 
	reference source and target by name.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Add source color to target where the target color is not black for all frames 
	starting at offset for the length of the source, reference source and target 
	by id.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Add source color to target where the target color is not black for all frames 
	starting at offset for the length of the source, reference source and target 
	by name.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Add source color to target where target color is not blank from the source 
	frame to the target offset frame, reference source and target by id.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_OFFSET)(int sourceAnimationId, int targetAnimationId, int frameId, int offset);
/*
	Add source color to target where target color is not blank from the source 
	frame to the target offset frame, reference source and target by name.
*/
typedef void		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double offset);
/*
	Append all source frames to the target animation, reference source and target 
	by id.
*/
typedef void		(*PLUGIN_APPEND_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Append all source frames to the target animation, reference source and target 
	by name.
*/
typedef void		(*PLUGIN_APPEND_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_APPEND_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	`PluginClearAll` will issue a `CLEAR` effect for all devices.
*/
typedef void		(*PLUGIN_CLEAR_ALL)();
/*
	`PluginClearAnimationType` will issue a `CLEAR` effect for the given device.
*/
typedef void		(*PLUGIN_CLEAR_ANIMATION_TYPE)(int deviceType, int device);
/*
	`PluginCloseAll` closes all open animations so they can be reloaded from 
	disk. The set of animations will be stopped if playing.
*/
typedef void		(*PLUGIN_CLOSE_ALL)();
/*
	Closes the `Chroma` animation to free up resources referenced by id. Returns 
	the animation id upon success. Returns -1 upon failure. This might be used 
	while authoring effects if there was a change necessitating re-opening 
	the animation. The animation id can no longer be used once closed.
*/
typedef int			(*PLUGIN_CLOSE_ANIMATION)(int animationId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_CLOSE_ANIMATION_D)(double animationId);
/*
	Closes the `Chroma` animation referenced by name so that the animation can 
	be reloaded from disk.
*/
typedef void		(*PLUGIN_CLOSE_ANIMATION_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_CLOSE_ANIMATION_NAME_D)(const char* path);
/*
	`PluginCloseComposite` closes a set of animations so they can be reloaded 
	from disk. The set of animations will be stopped if playing.
*/
typedef void		(*PLUGIN_CLOSE_COMPOSITE)(const char* name);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_CLOSE_COMPOSITE_D)(const char* name);
/*
	Copy animation to named target animation in memory. If target animation 
	exists, close first. Source is referenced by id.
*/
typedef int			(*PLUGIN_COPY_ANIMATION)(int sourceAnimationId, const char* targetAnimation);
/*
	Copy animation to named target animation in memory. If target animation 
	exists, close first. Source is referenced by name.
*/
typedef void		(*PLUGIN_COPY_ANIMATION_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_ANIMATION_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Copy blue channel to other channels for all frames. Intensity range is 0.0 
	to 1.0. Reference the animation by id.
*/
typedef void		(*PLUGIN_COPY_BLUE_CHANNEL_ALL_FRAMES)(int animationId, float redIntensity, float greenIntensity);
/*
	Copy blue channel to other channels for all frames. Intensity range is 0.0 
	to 1.0. Reference the animation by name.
*/
typedef void		(*PLUGIN_COPY_BLUE_CHANNEL_ALL_FRAMES_NAME)(const char* path, float redIntensity, float greenIntensity);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_BLUE_CHANNEL_ALL_FRAMES_NAME_D)(const char* path, double redIntensity, double greenIntensity);
/*
	Copy green channel to other channels for all frames. Intensity range is 
	0.0 to 1.0. Reference the animation by id.
*/
typedef void		(*PLUGIN_COPY_GREEN_CHANNEL_ALL_FRAMES)(int animationId, float redIntensity, float blueIntensity);
/*
	Copy green channel to other channels for all frames. Intensity range is 
	0.0 to 1.0. Reference the animation by name.
*/
typedef void		(*PLUGIN_COPY_GREEN_CHANNEL_ALL_FRAMES_NAME)(const char* path, float redIntensity, float blueIntensity);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_GREEN_CHANNEL_ALL_FRAMES_NAME_D)(const char* path, double redIntensity, double blueIntensity);
/*
	Copy animation key color from the source animation to the target animation 
	for the given frame. Reference the source and target by id.
*/
typedef void		(*PLUGIN_COPY_KEY_COLOR)(int sourceAnimationId, int targetAnimationId, int frameId, int rzkey);
/*
	Copy animation key color from the source animation to the target animation 
	for all frames. Reference the source and target by id.
*/
typedef void		(*PLUGIN_COPY_KEY_COLOR_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId, int rzkey);
/*
	Copy animation key color from the source animation to the target animation 
	for all frames. Reference the source and target by name.
*/
typedef void		(*PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation, int rzkey);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double rzkey);
/*
	Copy animation key color from the source animation to the target animation 
	for all frames, starting at the offset for the length of the source animation. 
	Source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int rzkey, int offset);
/*
	Copy animation key color from the source animation to the target animation 
	for all frames, starting at the offset for the length of the source animation. 
	Source and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int rzkey, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double rzkey, double offset);
/*
	Copy animation key color from the source animation to the target animation 
	for the given frame.
*/
typedef void		(*PLUGIN_COPY_KEY_COLOR_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int rzkey);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_KEY_COLOR_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double rzkey);
/*
	Copy animation color for a set of keys from the source animation to the 
	target animation for the given frame. Reference the source and target by 
	id.
*/
typedef void		(*PLUGIN_COPY_KEYS_COLOR)(int sourceAnimationId, int targetAnimationId, int frameId, const int* keys, int size);
/*
	Copy animation color for a set of keys from the source animation to the 
	target animation for all frames. Reference the source and target by id.
*/
typedef void		(*PLUGIN_COPY_KEYS_COLOR_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId, const int* keys, int size);
/*
	Copy animation color for a set of keys from the source animation to the 
	target animation for all frames. Reference the source and target by name.
*/
typedef void		(*PLUGIN_COPY_KEYS_COLOR_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation, const int* keys, int size);
/*
	Copy animation color for a set of keys from the source animation to the 
	target animation for the given frame. Reference the source and target by 
	name.
*/
typedef void		(*PLUGIN_COPY_KEYS_COLOR_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, const int* keys, int size);
/*
	Copy animation color for a set of keys from the source animation to the 
	target animation from the source frame to the target frame. Reference the 
	source and target by id.
*/
typedef void		(*PLUGIN_COPY_KEYS_COLOR_OFFSET)(int sourceAnimationId, int targetAnimationId, int sourceFrameId, int targetFrameId, const int* keys, int size);
/*
	Copy animation color for a set of keys from the source animation to the 
	target animation from the source frame to the target frame. Reference the 
	source and target by name.
*/
typedef void		(*PLUGIN_COPY_KEYS_COLOR_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int sourceFrameId, int targetFrameId, const int* keys, int size);
/*
	Copy source animation to target animation for the given frame. Source and 
	target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS)(int sourceAnimationId, int targetAnimationId, int frameId);
/*
	Copy nonzero colors from a source animation to a target animation for all 
	frames. Reference source and target by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Copy nonzero colors from a source animation to a target animation for all 
	frames. Reference source and target by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Copy nonzero colors from a source animation to a target animation for all 
	frames starting at the offset for the length of the source animation. The 
	source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Copy nonzero colors from a source animation to a target animation for all 
	frames starting at the offset for the length of the source animation. The 
	source and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Copy nonzero colors from source animation to target animation for the specified 
	frame. Source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId);
/*
	Copy nonzero colors from the source animation to the target animation from 
	the source frame to the target offset frame. Source and target are referenced 
	by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_OFFSET)(int sourceAnimationId, int targetAnimationId, int frameId, int offset);
/*
	Copy nonzero colors from the source animation to the target animation from 
	the source frame to the target offset frame. Source and target are referenced 
	by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_ALL_KEYS_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double offset);
/*
	Copy animation key color from the source animation to the target animation 
	for the given frame where color is not zero.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_KEY_COLOR)(int sourceAnimationId, int targetAnimationId, int frameId, int rzkey);
/*
	Copy animation key color from the source animation to the target animation 
	for the given frame where color is not zero.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_KEY_COLOR_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int rzkey);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_KEY_COLOR_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double rzkey);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for the specified frame. Source and target 
	are referenced by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS)(int sourceAnimationId, int targetAnimationId, int frameId);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for all frames. Source and target are referenced 
	by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for all frames. Source and target are referenced 
	by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for all frames. Source and target are referenced 
	by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for all frames starting at the target offset 
	for the length of the source animation. Source and target animations are 
	referenced by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for the specified frame. The source and target 
	are referenced by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for the specified source frame and target offset 
	frame. The source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_OFFSET)(int sourceAnimationId, int targetAnimationId, int frameId, int offset);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is nonzero for the specified source frame and target offset 
	frame. The source and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double offset);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is zero for all frames. Source and target are referenced 
	by id.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ZERO_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Copy nonzero colors from the source animation to the target animation where 
	the target color is zero for all frames. Source and target are referenced 
	by name.
*/
typedef void		(*PLUGIN_COPY_NON_ZERO_TARGET_ZERO_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_NON_ZERO_TARGET_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Copy red channel to other channels for all frames. Intensity range is 0.0 
	to 1.0. Reference the animation by id.
*/
typedef void		(*PLUGIN_COPY_RED_CHANNEL_ALL_FRAMES)(int animationId, float greenIntensity, float blueIntensity);
/*
	Copy green channel to other channels for all frames. Intensity range is 
	0.0 to 1.0. Reference the animation by name.
*/
typedef void		(*PLUGIN_COPY_RED_CHANNEL_ALL_FRAMES_NAME)(const char* path, float greenIntensity, float blueIntensity);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_RED_CHANNEL_ALL_FRAMES_NAME_D)(const char* path, double greenIntensity, double blueIntensity);
/*
	Copy zero colors from source animation to target animation for all frames. 
	Source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Copy zero colors from source animation to target animation for all frames. 
	Source and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Copy zero colors from source animation to target animation for all frames 
	starting at the target offset for the length of the source animation. Source 
	and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Copy zero colors from source animation to target animation for all frames 
	starting at the target offset for the length of the source animation. Source 
	and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Copy zero key color from source animation to target animation for the specified 
	frame. Source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_ZERO_KEY_COLOR)(int sourceAnimationId, int targetAnimationId, int frameId, int rzkey);
/*
	Copy zero key color from source animation to target animation for the specified 
	frame. Source and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_ZERO_KEY_COLOR_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int rzkey);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_ZERO_KEY_COLOR_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double rzkey);
/*
	Copy nonzero color from source animation to target animation where target 
	is zero for all frames. Source and target are referenced by id.
*/
typedef void		(*PLUGIN_COPY_ZERO_TARGET_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Copy nonzero color from source animation to target animation where target 
	is zero for all frames. Source and target are referenced by name.
*/
typedef void		(*PLUGIN_COPY_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_COPY_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_CHROMA_LINK_EFFECT)(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_EFFECT)(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_HEADSET_EFFECT)(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_KEYBOARD_EFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_KEYPAD_EFFECT)(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_MOUSE_EFFECT)(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_CREATE_MOUSEPAD_EFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_DELETE_EFFECT)(RZEFFECTID EffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_INIT)();
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_INIT_SDK)(ChromaSDK::APPINFOTYPE* AppInfo);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_QUERY_DEVICE)(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE& DeviceInfo);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_SET_EFFECT)(RZEFFECTID EffectId);
/*
	Direct access to low level API.
*/
typedef RZRESULT	(*PLUGIN_CORE_UNINIT)();
/*
	Creates a `Chroma` animation at the given path. The `deviceType` parameter 
	uses `EChromaSDKDeviceTypeEnum` as an integer. The `device` parameter uses 
	`EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` as an integer, respective 
	to the `deviceType`. Returns the animation id upon success. Returns -1 
	upon failure. Saves a `Chroma` animation file with the `.chroma` extension 
	at the given path. Returns the animation id upon success. Returns -1 upon 
	failure.
*/
typedef int			(*PLUGIN_CREATE_ANIMATION)(const char* path, int deviceType, int device);
/*
	Creates a `Chroma` animation in memory without creating a file. The `deviceType` 
	parameter uses `EChromaSDKDeviceTypeEnum` as an integer. The `device` parameter 
	uses `EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` as an integer, 
	respective to the `deviceType`. Returns the animation id upon success. 
	Returns -1 upon failure. Returns the animation id upon success. Returns 
	-1 upon failure.
*/
typedef int			(*PLUGIN_CREATE_ANIMATION_IN_MEMORY)(int deviceType, int device);
/*
	Create a device specific effect.
*/
typedef RZRESULT	(*PLUGIN_CREATE_EFFECT)(RZDEVICEID deviceId, ChromaSDK::EFFECT_TYPE effect, int* colors, int size, ChromaSDK::FChromaSDKGuid* effectId);
/*
	Delete an effect given the effect id.
*/
typedef RZRESULT	(*PLUGIN_DELETE_EFFECT)(const ChromaSDK::FChromaSDKGuid& effectId);
/*
	Duplicate the first animation frame so that the animation length matches 
	the frame count. Animation is referenced by id.
*/
typedef void		(*PLUGIN_DUPLICATE_FIRST_FRAME)(int animationId, int frameCount);
/*
	Duplicate the first animation frame so that the animation length matches 
	the frame count. Animation is referenced by name.
*/
typedef void		(*PLUGIN_DUPLICATE_FIRST_FRAME_NAME)(const char* path, int frameCount);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_DUPLICATE_FIRST_FRAME_NAME_D)(const char* path, double frameCount);
/*
	Duplicate all the frames of the animation to double the animation length. 
	Frame 1 becomes frame 1 and 2. Frame 2 becomes frame 3 and 4. And so on. 
	The animation is referenced by id.
*/
typedef void		(*PLUGIN_DUPLICATE_FRAMES)(int animationId);
/*
	Duplicate all the frames of the animation to double the animation length. 
	Frame 1 becomes frame 1 and 2. Frame 2 becomes frame 3 and 4. And so on. 
	The animation is referenced by name.
*/
typedef void		(*PLUGIN_DUPLICATE_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_DUPLICATE_FRAMES_NAME_D)(const char* path);
/*
	Duplicate all the animation frames in reverse so that the animation plays 
	forwards and backwards. Animation is referenced by id.
*/
typedef void		(*PLUGIN_DUPLICATE_MIRROR_FRAMES)(int animationId);
/*
	Duplicate all the animation frames in reverse so that the animation plays 
	forwards and backwards. Animation is referenced by name.
*/
typedef void		(*PLUGIN_DUPLICATE_MIRROR_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_DUPLICATE_MIRROR_FRAMES_NAME_D)(const char* path);
/*
	Fade the animation to black starting at the fade frame index to the end 
	of the animation. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FADE_END_FRAMES)(int animationId, int fade);
/*
	Fade the animation to black starting at the fade frame index to the end 
	of the animation. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FADE_END_FRAMES_NAME)(const char* path, int fade);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FADE_END_FRAMES_NAME_D)(const char* path, double fade);
/*
	Fade the animation from black to full color starting at 0 to the fade frame 
	index. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FADE_START_FRAMES)(int animationId, int fade);
/*
	Fade the animation from black to full color starting at 0 to the fade frame 
	index. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FADE_START_FRAMES_NAME)(const char* path, int fade);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FADE_START_FRAMES_NAME_D)(const char* path, double fade);
/*
	Set the RGB value for all colors in the specified frame. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_FILL_COLOR)(int animationId, int frameId, int color);
/*
	Set the RGB value for all colors for all frames. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_FILL_COLOR_ALL_FRAMES)(int animationId, int color);
/*
	Set the RGB value for all colors for all frames. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_FILL_COLOR_ALL_FRAMES_NAME)(const char* path, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_COLOR_ALL_FRAMES_NAME_D)(const char* path, double color);
/*
	Set the RGB value for all colors for all frames. Use the range of 0 to 255 
	for red, green, and blue parameters. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_COLOR_ALL_FRAMES_RGB)(int animationId, int red, int green, int blue);
/*
	Set the RGB value for all colors for all frames. Use the range of 0 to 255 
	for red, green, and blue parameters. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_COLOR_ALL_FRAMES_RGB_NAME)(const char* path, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_COLOR_ALL_FRAMES_RGB_NAME_D)(const char* path, double red, double green, double blue);
/*
	Set the RGB value for all colors in the specified frame. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_FILL_COLOR_NAME)(const char* path, int frameId, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_COLOR_NAME_D)(const char* path, double frameId, double color);
/*
	Set the RGB value for all colors in the specified frame. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_FILL_COLOR_RGB)(int animationId, int frameId, int red, int green, int blue);
/*
	Set the RGB value for all colors in the specified frame. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_FILL_COLOR_RGB_NAME)(const char* path, int frameId, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_COLOR_RGB_NAME_D)(const char* path, double frameId, double red, double green, double blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors in the specified 
	frame. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR)(int animationId, int frameId, int color);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors for all frames. 
	Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES)(int animationId, int color);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors for all frames. 
	Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_NAME)(const char* path, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_NAME_D)(const char* path, double color);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors for all frames. 
	Use the range of 0 to 255 for red, green, and blue parameters. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_RGB)(int animationId, int red, int green, int blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors for all frames. 
	Use the range of 0 to 255 for red, green, and blue parameters. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_RGB_NAME)(const char* path, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_RGB_NAME_D)(const char* path, double red, double green, double blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors in the specified 
	frame. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_NAME)(const char* path, int frameId, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_NON_ZERO_COLOR_NAME_D)(const char* path, double frameId, double color);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors in the specified 
	frame. Use the range of 0 to 255 for red, green, and blue parameters. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_RGB)(int animationId, int frameId, int red, int green, int blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Set the RGB value for a subset of colors in the specified 
	frame. Use the range of 0 to 255 for red, green, and blue parameters. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_FILL_NON_ZERO_COLOR_RGB_NAME)(const char* path, int frameId, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_NON_ZERO_COLOR_RGB_NAME_D)(const char* path, double frameId, double red, double green, double blue);
/*
	Fill the frame with random RGB values for the given frame. Animation is 
	referenced by id.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS)(int animationId, int frameId);
/*
	Fill the frame with random RGB values for all frames. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_ALL_FRAMES)(int animationId);
/*
	Fill the frame with random RGB values for all frames. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_ALL_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_RANDOM_COLORS_ALL_FRAMES_NAME_D)(const char* path);
/*
	Fill the frame with random black and white values for the specified frame. 
	Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE)(int animationId, int frameId);
/*
	Fill the frame with random black and white values for all frames. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_ALL_FRAMES)(int animationId);
/*
	Fill the frame with random black and white values for all frames. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_ALL_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_ALL_FRAMES_NAME_D)(const char* path);
/*
	Fill the frame with random black and white values for the specified frame. 
	Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_NAME)(const char* path, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_NAME_D)(const char* path, double frameId);
/*
	Fill the frame with random RGB values for the given frame. Animation is 
	referenced by name.
*/
typedef void		(*PLUGIN_FILL_RANDOM_COLORS_NAME)(const char* path, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_RANDOM_COLORS_NAME_D)(const char* path, double frameId);
/*
	Fill the specified frame with RGB color where the animation color is less 
	than the RGB threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS)(int animationId, int frameId, int threshold, int color);
/*
	Fill all frames with RGB color where the animation color is less than the 
	RGB threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES)(int animationId, int threshold, int color);
/*
	Fill all frames with RGB color where the animation color is less than the 
	RGB threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_NAME)(const char* path, int threshold, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_NAME_D)(const char* path, double threshold, double color);
/*
	Fill all frames with RGB color where the animation color is less than the 
	threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_RGB)(int animationId, int threshold, int red, int green, int blue);
/*
	Fill all frames with RGB color where the animation color is less than the 
	threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_RGB_NAME)(const char* path, int threshold, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_RGB_NAME_D)(const char* path, double threshold, double red, double green, double blue);
/*
	Fill all frames with the min RGB color where the animation color is less 
	than the min threshold AND with the max RGB color where the animation is 
	more than the max threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_ALL_FRAMES_RGB)(int animationId, int minThreshold, int minRed, int minGreen, int minBlue, int maxThreshold, int maxRed, int maxGreen, int maxBlue);
/*
	Fill all frames with the min RGB color where the animation color is less 
	than the min threshold AND with the max RGB color where the animation is 
	more than the max threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_ALL_FRAMES_RGB_NAME)(const char* path, int minThreshold, int minRed, int minGreen, int minBlue, int maxThreshold, int maxRed, int maxGreen, int maxBlue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_ALL_FRAMES_RGB_NAME_D)(const char* path, double minThreshold, double minRed, double minGreen, double minBlue, double maxThreshold, double maxRed, double maxGreen, double maxBlue);
/*
	Fill the specified frame with the min RGB color where the animation color 
	is less than the min threshold AND with the max RGB color where the animation 
	is more than the max threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_RGB)(int animationId, int frameId, int minThreshold, int minRed, int minGreen, int minBlue, int maxThreshold, int maxRed, int maxGreen, int maxBlue);
/*
	Fill the specified frame with the min RGB color where the animation color 
	is less than the min threshold AND with the max RGB color where the animation 
	is more than the max threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_RGB_NAME)(const char* path, int frameId, int minThreshold, int minRed, int minGreen, int minBlue, int maxThreshold, int maxRed, int maxGreen, int maxBlue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_RGB_NAME_D)(const char* path, double frameId, double minThreshold, double minRed, double minGreen, double minBlue, double maxThreshold, double maxRed, double maxGreen, double maxBlue);
/*
	Fill the specified frame with RGB color where the animation color is less 
	than the RGB threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_NAME)(const char* path, int frameId, int threshold, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_COLORS_NAME_D)(const char* path, double frameId, double threshold, double color);
/*
	Fill the specified frame with RGB color where the animation color is less 
	than the RGB threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_RGB)(int animationId, int frameId, int threshold, int red, int green, int blue);
/*
	Fill the specified frame with RGB color where the animation color is less 
	than the RGB threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_COLORS_RGB_NAME)(const char* path, int frameId, int threshold, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_COLORS_RGB_NAME_D)(const char* path, double frameId, double threshold, double red, double green, double blue);
/*
	Fill all frames with RGB color where the animation color is less than the 
	RGB threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_RGB_COLORS_ALL_FRAMES_RGB)(int animationId, int redThreshold, int greenThreshold, int blueThreshold, int red, int green, int blue);
/*
	Fill all frames with RGB color where the animation color is less than the 
	RGB threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_RGB_COLORS_ALL_FRAMES_RGB_NAME)(const char* path, int redThreshold, int greenThreshold, int blueThreshold, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_RGB_COLORS_ALL_FRAMES_RGB_NAME_D)(const char* path, double redThreshold, double greenThreshold, double blueThreshold, double red, double green, double blue);
/*
	Fill the specified frame with RGB color where the animation color is less 
	than the RGB threshold. Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_RGB_COLORS_RGB)(int animationId, int frameId, int redThreshold, int greenThreshold, int blueThreshold, int red, int green, int blue);
/*
	Fill the specified frame with RGB color where the animation color is less 
	than the RGB threshold. Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_THRESHOLD_RGB_COLORS_RGB_NAME)(const char* path, int frameId, int redThreshold, int greenThreshold, int blueThreshold, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_THRESHOLD_RGB_COLORS_RGB_NAME_D)(const char* path, double frameId, double redThreshold, double greenThreshold, double blueThreshold, double red, double green, double blue);
/*
	Fill the specified frame with RGB color where the animation color is zero. 
	Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR)(int animationId, int frameId, int color);
/*
	Fill all frames with RGB color where the animation color is zero. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES)(int animationId, int color);
/*
	Fill all frames with RGB color where the animation color is zero. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_NAME)(const char* path, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_NAME_D)(const char* path, double color);
/*
	Fill all frames with RGB color where the animation color is zero. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_RGB)(int animationId, int red, int green, int blue);
/*
	Fill all frames with RGB color where the animation color is zero. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_RGB_NAME)(const char* path, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_RGB_NAME_D)(const char* path, double red, double green, double blue);
/*
	Fill the specified frame with RGB color where the animation color is zero. 
	Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_NAME)(const char* path, int frameId, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_ZERO_COLOR_NAME_D)(const char* path, double frameId, double color);
/*
	Fill the specified frame with RGB color where the animation color is zero. 
	Animation is referenced by id.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_RGB)(int animationId, int frameId, int red, int green, int blue);
/*
	Fill the specified frame with RGB color where the animation color is zero. 
	Animation is referenced by name.
*/
typedef void		(*PLUGIN_FILL_ZERO_COLOR_RGB_NAME)(const char* path, int frameId, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_FILL_ZERO_COLOR_RGB_NAME_D)(const char* path, double frameId, double red, double green, double blue);
/*
	Get the animation color for a frame given the `1D` `led`. The `led` should 
	be greater than or equal to 0 and less than the `MaxLeds`. Animation is 
	referenced by id.
*/
typedef int			(*PLUGIN_GET_1D_COLOR)(int animationId, int frameId, int led);
/*
	Get the animation color for a frame given the `1D` `led`. The `led` should 
	be greater than or equal to 0 and less than the `MaxLeds`. Animation is 
	referenced by name.
*/
typedef int			(*PLUGIN_GET_1D_COLOR_NAME)(const char* path, int frameId, int led);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_1D_COLOR_NAME_D)(const char* path, double frameId, double led);
/*
	Get the animation color for a frame given the `2D` `row` and `column`. The 
	`row` should be greater than or equal to 0 and less than the `MaxRow`. 
	The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
	Animation is referenced by id.
*/
typedef int			(*PLUGIN_GET_2D_COLOR)(int animationId, int frameId, int row, int column);
/*
	Get the animation color for a frame given the `2D` `row` and `column`. The 
	`row` should be greater than or equal to 0 and less than the `MaxRow`. 
	The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
	Animation is referenced by name.
*/
typedef int			(*PLUGIN_GET_2D_COLOR_NAME)(const char* path, int frameId, int row, int column);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_2D_COLOR_NAME_D)(const char* path, double frameId, double row, double column);
/*
	Get the animation id for the named animation.
*/
typedef int			(*PLUGIN_GET_ANIMATION)(const char* name);
/*
	`PluginGetAnimationCount` will return the number of loaded animations.
*/
typedef int			(*PLUGIN_GET_ANIMATION_COUNT)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_ANIMATION_D)(const char* name);
/*
	`PluginGetAnimationId` will return the `animationId` given the `index` of 
	the loaded animation. The `index` is zero-based and less than the number 
	returned by `PluginGetAnimationCount`. Use `PluginGetAnimationName` to 
	get the name of the animation.
*/
typedef int			(*PLUGIN_GET_ANIMATION_ID)(int index);
/*
	`PluginGetAnimationName` takes an `animationId` and returns the name of 
	the animation of the `.chroma` animation file. If a name is not available 
	then an empty string will be returned.
*/
typedef const char*	(*PLUGIN_GET_ANIMATION_NAME)(int animationId);
/*
	Get the current frame of the animation referenced by id.
*/
typedef int			(*PLUGIN_GET_CURRENT_FRAME)(int animationId);
/*
	Get the current frame of the animation referenced by name.
*/
typedef int			(*PLUGIN_GET_CURRENT_FRAME_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_CURRENT_FRAME_NAME_D)(const char* path);
/*
	Returns the `EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` of a `Chroma` 
	animation respective to the `deviceType`, as an integer upon success. Returns 
	-1 upon failure.
*/
typedef int			(*PLUGIN_GET_DEVICE)(int animationId);
/*
	Returns the `EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` of a `Chroma` 
	animation respective to the `deviceType`, as an integer upon success. Returns 
	-1 upon failure.
*/
typedef int			(*PLUGIN_GET_DEVICE_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_DEVICE_NAME_D)(const char* path);
/*
	Returns the `EChromaSDKDeviceTypeEnum` of a `Chroma` animation as an integer 
	upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_GET_DEVICE_TYPE)(int animationId);
/*
	Returns the `EChromaSDKDeviceTypeEnum` of a `Chroma` animation as an integer 
	upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_GET_DEVICE_TYPE_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_DEVICE_TYPE_NAME_D)(const char* path);
/*
	Gets the frame colors and duration (in seconds) for a `Chroma` animation. 
	The `color` is expected to be an array of the expected dimensions for the 
	`deviceType/device`. The `length` parameter is the size of the `color` 
	array. For `EChromaSDKDevice1DEnum` the array size should be `MAX LEDS`. 
	For `EChromaSDKDevice2DEnum` the array size should be `MAX ROW` * `MAX 
	COLUMN`. Returns the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_GET_FRAME)(int animationId, int frameIndex, float* duration, int* colors, int length);
/*
	Returns the frame count of a `Chroma` animation upon success. Returns -1 
	upon failure.
*/
typedef int			(*PLUGIN_GET_FRAME_COUNT)(int animationId);
/*
	Returns the frame count of a `Chroma` animation upon success. Returns -1 
	upon failure.
*/
typedef int			(*PLUGIN_GET_FRAME_COUNT_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_FRAME_COUNT_NAME_D)(const char* path);
/*
	Get the color of an animation key for the given frame referenced by id.
*/
typedef int			(*PLUGIN_GET_KEY_COLOR)(int animationId, int frameId, int rzkey);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_KEY_COLOR_D)(const char* path, double frameId, double rzkey);
/*
	Get the color of an animation key for the given frame referenced by name.
*/
typedef int			(*PLUGIN_GET_KEY_COLOR_NAME)(const char* path, int frameId, int rzkey);
/*
	Returns `RZRESULT_SUCCESS` if the plugin has been initialized successfully. 
	Returns `RZRESULT_DLL_NOT_FOUND` if core Chroma library is not found. Returns 
	`RZRESULT_DLL_INVALID_SIGNATURE` if core Chroma library has an invalid 
	signature.
*/
typedef RZRESULT	(*PLUGIN_GET_LIBRARY_LOADED_STATE)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_LIBRARY_LOADED_STATE_D)();
/*
	Returns the `MAX COLUMN` given the `EChromaSDKDevice2DEnum` device as an 
	integer upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_GET_MAX_COLUMN)(int device);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_MAX_COLUMN_D)(double device);
/*
	Returns the MAX LEDS given the `EChromaSDKDevice1DEnum` device as an integer 
	upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_GET_MAX_LEDS)(int device);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_MAX_LEDS_D)(double device);
/*
	Returns the `MAX ROW` given the `EChromaSDKDevice2DEnum` device as an integer 
	upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_GET_MAX_ROW)(int device);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_MAX_ROW_D)(double device);
/*
	`PluginGetPlayingAnimationCount` will return the number of playing animations.
*/
typedef int			(*PLUGIN_GET_PLAYING_ANIMATION_COUNT)();
/*
	`PluginGetPlayingAnimationId` will return the `animationId` given the `index` 
	of the playing animation. The `index` is zero-based and less than the number 
	returned by `PluginGetPlayingAnimationCount`. Use `PluginGetAnimationName` 
	to get the name of the animation.
*/
typedef int			(*PLUGIN_GET_PLAYING_ANIMATION_ID)(int index);
/*
	Get the RGB color given red, green, and blue.
*/
typedef int			(*PLUGIN_GET_RGB)(int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_GET_RGBD)(double red, double green, double blue);
/*
	Check if the animation has loop enabled referenced by id.
*/
typedef bool		(*PLUGIN_HAS_ANIMATION_LOOP)(int animationId);
/*
	Check if the animation has loop enabled referenced by name.
*/
typedef bool		(*PLUGIN_HAS_ANIMATION_LOOP_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_HAS_ANIMATION_LOOP_NAME_D)(const char* path);
/*
	Initialize the ChromaSDK. Zero indicates  success, otherwise failure. Many 
	API methods auto initialize the ChromaSDK if not already initialized.
*/
typedef RZRESULT	(*PLUGIN_INIT)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_INIT_D)();
/*
	Initialize the ChromaSDK. AppInfo populates the details in Synapse. Zero 
	indicates  success, otherwise failure. Many API methods auto initialize 
	the ChromaSDK if not already initialized.
*/
typedef RZRESULT	(*PLUGIN_INIT_SDK)(ChromaSDK::APPINFOTYPE* AppInfo);
/*
	Insert an animation delay by duplicating the frame by the delay number of 
	times. Animation is referenced by id.
*/
typedef void		(*PLUGIN_INSERT_DELAY)(int animationId, int frameId, int delay);
/*
	Insert an animation delay by duplicating the frame by the delay number of 
	times. Animation is referenced by name.
*/
typedef void		(*PLUGIN_INSERT_DELAY_NAME)(const char* path, int frameId, int delay);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_INSERT_DELAY_NAME_D)(const char* path, double frameId, double delay);
/*
	Duplicate the source frame index at the target frame index. Animation is 
	referenced by id.
*/
typedef void		(*PLUGIN_INSERT_FRAME)(int animationId, int sourceFrame, int targetFrame);
/*
	Duplicate the source frame index at the target frame index. Animation is 
	referenced by name.
*/
typedef void		(*PLUGIN_INSERT_FRAME_NAME)(const char* path, int sourceFrame, int targetFrame);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_INSERT_FRAME_NAME_D)(const char* path, double sourceFrame, double targetFrame);
/*
	Invert all the colors at the specified frame. Animation is referenced by 
	id.
*/
typedef void		(*PLUGIN_INVERT_COLORS)(int animationId, int frameId);
/*
	Invert all the colors for all frames. Animation is referenced by id.
*/
typedef void		(*PLUGIN_INVERT_COLORS_ALL_FRAMES)(int animationId);
/*
	Invert all the colors for all frames. Animation is referenced by name.
*/
typedef void		(*PLUGIN_INVERT_COLORS_ALL_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_INVERT_COLORS_ALL_FRAMES_NAME_D)(const char* path);
/*
	Invert all the colors at the specified frame. Animation is referenced by 
	name.
*/
typedef void		(*PLUGIN_INVERT_COLORS_NAME)(const char* path, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_INVERT_COLORS_NAME_D)(const char* path, double frameId);
/*
	Check if the animation is paused referenced by id.
*/
typedef bool		(*PLUGIN_IS_ANIMATION_PAUSED)(int animationId);
/*
	Check if the animation is paused referenced by name.
*/
typedef bool		(*PLUGIN_IS_ANIMATION_PAUSED_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_ANIMATION_PAUSED_NAME_D)(const char* path);
/*
	The editor dialog is a non-blocking modal window, this method returns true 
	if the modal window is open, otherwise false.
*/
typedef bool		(*PLUGIN_IS_DIALOG_OPEN)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_DIALOG_OPEN_D)();
/*
	Returns true if the plugin has been initialized. Returns false if the plugin 
	is uninitialized.
*/
typedef bool		(*PLUGIN_IS_INITIALIZED)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_INITIALIZED_D)();
/*
	If the method can be invoked the method returns true.
*/
typedef bool		(*PLUGIN_IS_PLATFORM_SUPPORTED)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_PLATFORM_SUPPORTED_D)();
/*
	`PluginIsPlayingName` automatically handles initializing the `ChromaSDK`. 
	The named `.chroma` animation file will be automatically opened. The method 
	will return whether the animation is playing or not. Animation is referenced 
	by id.
*/
typedef bool		(*PLUGIN_IS_PLAYING)(int animationId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_PLAYING_D)(double animationId);
/*
	`PluginIsPlayingName` automatically handles initializing the `ChromaSDK`. 
	The named `.chroma` animation file will be automatically opened. The method 
	will return whether the animation is playing or not. Animation is referenced 
	by name.
*/
typedef bool		(*PLUGIN_IS_PLAYING_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_PLAYING_NAME_D)(const char* path);
/*
	`PluginIsPlayingType` automatically handles initializing the `ChromaSDK`. 
	If any animation is playing for the `deviceType` and `device` combination, 
	the method will return true, otherwise false.
*/
typedef bool		(*PLUGIN_IS_PLAYING_TYPE)(int deviceType, int device);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_IS_PLAYING_TYPE_D)(double deviceType, double device);
/*
	Do a lerp math operation on a float.
*/
typedef float		(*PLUGIN_LERP)(float start, float end, float amt);
/*
	Lerp from one color to another given t in the range 0.0 to 1.0.
*/
typedef int			(*PLUGIN_LERP_COLOR)(int from, int to, float t);
/*
	Loads `Chroma` effects so that the animation can be played immediately. 
	Returns the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_LOAD_ANIMATION)(int animationId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_LOAD_ANIMATION_D)(double animationId);
/*
	Load the named animation.
*/
typedef void		(*PLUGIN_LOAD_ANIMATION_NAME)(const char* path);
/*
	Load a composite set of animations.
*/
typedef void		(*PLUGIN_LOAD_COMPOSITE)(const char* name);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color defaults to color. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES)(int animationId, int frameCount, float duration, int color);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color defaults to color. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_NAME)(const char* path, int frameCount, float duration, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MAKE_BLANK_FRAMES_NAME_D)(const char* path, double frameCount, double duration, double color);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color is random. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_RANDOM)(int animationId, int frameCount, float duration);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color is random black and white. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_RANDOM_BLACK_AND_WHITE)(int animationId, int frameCount, float duration);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color is random black and white. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_RANDOM_BLACK_AND_WHITE_NAME)(const char* path, int frameCount, float duration);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MAKE_BLANK_FRAMES_RANDOM_BLACK_AND_WHITE_NAME_D)(const char* path, double frameCount, double duration);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color is random. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_RANDOM_NAME)(const char* path, int frameCount, float duration);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MAKE_BLANK_FRAMES_RANDOM_NAME_D)(const char* path, double frameCount, double duration);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color defaults to color. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_RGB)(int animationId, int frameCount, float duration, int red, int green, int blue);
/*
	Make a blank animation for the length of the frame count. Frame duration 
	defaults to the duration. The frame color defaults to color. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_MAKE_BLANK_FRAMES_RGB_NAME)(const char* path, int frameCount, float duration, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MAKE_BLANK_FRAMES_RGB_NAME_D)(const char* path, double frameCount, double duration, double red, double green, double blue);
/*
	Flips the color grid horizontally for all `Chroma` animation frames. Returns 
	the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_MIRROR_HORIZONTALLY)(int animationId);
/*
	Flips the color grid vertically for all `Chroma` animation frames. This 
	method has no effect for `EChromaSDKDevice1DEnum` devices. Returns the 
	animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_MIRROR_VERTICALLY)(int animationId);
/*
	Multiply the color intensity with the lerp result from color 1 to color 
	2 using the frame index divided by the frame count for the `t` parameter. 
	Animation is referenced in id.
*/
typedef void		(*PLUGIN_MULTIPLY_COLOR_LERP_ALL_FRAMES)(int animationId, int color1, int color2);
/*
	Multiply the color intensity with the lerp result from color 1 to color 
	2 using the frame index divided by the frame count for the `t` parameter. 
	Animation is referenced in name.
*/
typedef void		(*PLUGIN_MULTIPLY_COLOR_LERP_ALL_FRAMES_NAME)(const char* path, int color1, int color2);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_COLOR_LERP_ALL_FRAMES_NAME_D)(const char* path, double color1, double color2);
/*
	Multiply all the colors in the frame by the intensity value. The valid the 
	intensity range is from 0.0 to 255.0. RGB components are multiplied equally. 
	An intensity of 0.5 would half the color value. Black colors in the frame 
	will not be affected by this method.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY)(int animationId, int frameId, float intensity);
/*
	Multiply all the colors for all frames by the intensity value. The valid 
	the intensity range is from 0.0 to 255.0. RGB components are multiplied 
	equally. An intensity of 0.5 would half the color value. Black colors in 
	the frame will not be affected by this method.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES)(int animationId, float intensity);
/*
	Multiply all the colors for all frames by the intensity value. The valid 
	the intensity range is from 0.0 to 255.0. RGB components are multiplied 
	equally. An intensity of 0.5 would half the color value. Black colors in 
	the frame will not be affected by this method.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_NAME)(const char* path, float intensity);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_NAME_D)(const char* path, double intensity);
/*
	Multiply all frames by the RBG color intensity. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_RGB)(int animationId, int red, int green, int blue);
/*
	Multiply all frames by the RBG color intensity. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_RGB_NAME)(const char* path, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_RGB_NAME_D)(const char* path, double red, double green, double blue);
/*
	Multiply the specific frame by the RBG color intensity. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_COLOR)(int animationId, int frameId, int color);
/*
	Multiply all frames by the RBG color intensity. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_COLOR_ALL_FRAMES)(int animationId, int color);
/*
	Multiply all frames by the RBG color intensity. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_COLOR_ALL_FRAMES_NAME)(const char* path, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_INTENSITY_COLOR_ALL_FRAMES_NAME_D)(const char* path, double color);
/*
	Multiply the specific frame by the RBG color intensity. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_COLOR_NAME)(const char* path, int frameId, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_INTENSITY_COLOR_NAME_D)(const char* path, double frameId, double color);
/*
	Multiply all the colors in the frame by the intensity value. The valid the 
	intensity range is from 0.0 to 255.0. RGB components are multiplied equally. 
	An intensity of 0.5 would half the color value. Black colors in the frame 
	will not be affected by this method.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_NAME)(const char* path, int frameId, float intensity);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_INTENSITY_NAME_D)(const char* path, double frameId, double intensity);
/*
	Multiply the specific frame by the RBG color intensity. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_RGB)(int animationId, int frameId, int red, int green, int blue);
/*
	Multiply the specific frame by the RBG color intensity. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_MULTIPLY_INTENSITY_RGB_NAME)(const char* path, int frameId, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_INTENSITY_RGB_NAME_D)(const char* path, double frameId, double red, double green, double blue);
/*
	Multiply the specific frame by the color lerp result between color 1 and 
	2 using the frame color value as the `t` value. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP)(int animationId, int frameId, int color1, int color2);
/*
	Multiply all frames by the color lerp result between color 1 and 2 using 
	the frame color value as the `t` value. Animation is referenced by id.
*/
typedef void		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES)(int animationId, int color1, int color2);
/*
	Multiply all frames by the color lerp result between color 1 and 2 using 
	the frame color value as the `t` value. Animation is referenced by name.
*/
typedef void		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_NAME)(const char* path, int color1, int color2);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_NAME_D)(const char* path, double color1, double color2);
/*
	Multiply the specific frame by the color lerp result between RGB 1 and 2 
	using the frame color value as the `t` value. Animation is referenced by 
	id.
*/
typedef void		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_RGB)(int animationId, int red1, int green1, int blue1, int red2, int green2, int blue2);
/*
	Multiply the specific frame by the color lerp result between RGB 1 and 2 
	using the frame color value as the `t` value. Animation is referenced by 
	name.
*/
typedef void		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME)(const char* path, int red1, int green1, int blue1, int red2, int green2, int blue2);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME_D)(const char* path, double red1, double green1, double blue1, double red2, double green2, double blue2);
/*
	Multiply the specific frame by the color lerp result between color 1 and 
	2 using the frame color value as the `t` value. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP)(int animationId, int frameId, int color1, int color2);
/*
	Multiply all frames by the color lerp result between color 1 and 2 using 
	the frame color value as the `t` value. Animation is referenced by id.
*/
typedef void		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES)(int animationId, int color1, int color2);
/*
	Multiply all frames by the color lerp result between color 1 and 2 using 
	the frame color value as the `t` value. Animation is referenced by name.
*/
typedef void		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_NAME)(const char* path, int color1, int color2);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_NAME_D)(const char* path, double color1, double color2);
/*
	Multiply all frames by the color lerp result between RGB 1 and 2 using the 
	frame color value as the `t` value. Animation is referenced by id.
*/
typedef void		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_RGB)(int animationId, int red1, int green1, int blue1, int red2, int green2, int blue2);
/*
	Multiply all frames by the color lerp result between RGB 1 and 2 using the 
	frame color value as the `t` value. Animation is referenced by name.
*/
typedef void		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME)(const char* path, int red1, int green1, int blue1, int red2, int green2, int blue2);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME_D)(const char* path, double red1, double green1, double blue1, double red2, double green2, double blue2);
/*
	Offset all colors in the frame using the RGB offset. Use the range of -255 
	to 255 for red, green, and blue parameters. Negative values remove color. 
	Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_COLORS)(int animationId, int frameId, int red, int green, int blue);
/*
	Offset all colors for all frames using the RGB offset. Use the range of 
	-255 to 255 for red, green, and blue parameters. Negative values remove 
	color. Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_COLORS_ALL_FRAMES)(int animationId, int red, int green, int blue);
/*
	Offset all colors for all frames using the RGB offset. Use the range of 
	-255 to 255 for red, green, and blue parameters. Negative values remove 
	color. Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_COLORS_ALL_FRAMES_NAME)(const char* path, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OFFSET_COLORS_ALL_FRAMES_NAME_D)(const char* path, double red, double green, double blue);
/*
	Offset all colors in the frame using the RGB offset. Use the range of -255 
	to 255 for red, green, and blue parameters. Negative values remove color. 
	Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_COLORS_NAME)(const char* path, int frameId, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OFFSET_COLORS_NAME_D)(const char* path, double frameId, double red, double green, double blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Offset a subset of colors in the frame using the RGB offset. 
	Use the range of -255 to 255 for red, green, and blue parameters. Negative 
	values remove color. Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_NON_ZERO_COLORS)(int animationId, int frameId, int red, int green, int blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Offset a subset of colors for all frames using the RGB offset. 
	Use the range of -255 to 255 for red, green, and blue parameters. Negative 
	values remove color. Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_NON_ZERO_COLORS_ALL_FRAMES)(int animationId, int red, int green, int blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Offset a subset of colors for all frames using the RGB offset. 
	Use the range of -255 to 255 for red, green, and blue parameters. Negative 
	values remove color. Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_NON_ZERO_COLORS_ALL_FRAMES_NAME)(const char* path, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OFFSET_NON_ZERO_COLORS_ALL_FRAMES_NAME_D)(const char* path, double red, double green, double blue);
/*
	This method will only update colors in the animation that are not already 
	set to black. Offset a subset of colors in the frame using the RGB offset. 
	Use the range of -255 to 255 for red, green, and blue parameters. Negative 
	values remove color. Positive values add color.
*/
typedef void		(*PLUGIN_OFFSET_NON_ZERO_COLORS_NAME)(const char* path, int frameId, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OFFSET_NON_ZERO_COLORS_NAME_D)(const char* path, double frameId, double red, double green, double blue);
/*
	Opens a `Chroma` animation file so that it can be played. Returns an animation 
	id >= 0 upon success. Returns -1 if there was a failure. The animation 
	id is used in most of the API methods.
*/
typedef int			(*PLUGIN_OPEN_ANIMATION)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OPEN_ANIMATION_D)(const char* path);
/*
	Opens a `Chroma` animation data from memory so that it can be played. `Data` 
	is a pointer to byte array of the loaded animation in memory. `Name` will 
	be assigned to the animation when loaded. Returns an animation id >= 0 
	upon success. Returns -1 if there was a failure. The animation id is used 
	in most of the API methods.
*/
typedef int			(*PLUGIN_OPEN_ANIMATION_FROM_MEMORY)(const byte* data, const char* name);
/*
	Opens a `Chroma` animation file with the `.chroma` extension. Returns zero 
	upon success. Returns -1 if there was a failure.
*/
typedef int			(*PLUGIN_OPEN_EDITOR_DIALOG)(const char* path);
/*
	Open the named animation in the editor dialog and play the animation at 
	start.
*/
typedef int			(*PLUGIN_OPEN_EDITOR_DIALOG_AND_PLAY)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OPEN_EDITOR_DIALOG_AND_PLAY_D)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OPEN_EDITOR_DIALOG_D)(const char* path);
/*
	Sets the `duration` for all grames in the `Chroma` animation to the `duration` 
	parameter. Returns the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_OVERRIDE_FRAME_DURATION)(int animationId, float duration);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_OVERRIDE_FRAME_DURATION_D)(double animationId, double duration);
/*
	Override the duration of all frames with the `duration` value. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_OVERRIDE_FRAME_DURATION_NAME)(const char* path, float duration);
/*
	Pause the current animation referenced by id.
*/
typedef void		(*PLUGIN_PAUSE_ANIMATION)(int animationId);
/*
	Pause the current animation referenced by name.
*/
typedef void		(*PLUGIN_PAUSE_ANIMATION_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_PAUSE_ANIMATION_NAME_D)(const char* path);
/*
	Plays the `Chroma` animation. This will load the animation, if not loaded 
	previously. Returns the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_PLAY_ANIMATION)(int animationId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_PLAY_ANIMATION_D)(double animationId);
/*
	`PluginPlayAnimationFrame` automatically handles initializing the `ChromaSDK`. 
	The method will play the animation given the `animationId` with looping 
	`on` or `off` starting at the `frameId`.
*/
typedef void		(*PLUGIN_PLAY_ANIMATION_FRAME)(int animationId, int frameId, bool loop);
/*
	`PluginPlayAnimationFrameName` automatically handles initializing the `ChromaSDK`. 
	The named `.chroma` animation file will be automatically opened. The animation 
	will play with looping `on` or `off` starting at the `frameId`.
*/
typedef void		(*PLUGIN_PLAY_ANIMATION_FRAME_NAME)(const char* path, int frameId, bool loop);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_PLAY_ANIMATION_FRAME_NAME_D)(const char* path, double frameId, double loop);
/*
	`PluginPlayAnimationLoop` automatically handles initializing the `ChromaSDK`. 
	The method will play the animation given the `animationId` with looping 
	`on` or `off`.
*/
typedef void		(*PLUGIN_PLAY_ANIMATION_LOOP)(int animationId, bool loop);
/*
	`PluginPlayAnimationName` automatically handles initializing the `ChromaSDK`. 
	The named `.chroma` animation file will be automatically opened. The animation 
	will play with looping `on` or `off`.
*/
typedef void		(*PLUGIN_PLAY_ANIMATION_NAME)(const char* path, bool loop);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_PLAY_ANIMATION_NAME_D)(const char* path, double loop);
/*
	`PluginPlayComposite` automatically handles initializing the `ChromaSDK`. 
	The named animation files for the `.chroma` set will be automatically opened. 
	The set of animations will play with looping `on` or `off`.
*/
typedef void		(*PLUGIN_PLAY_COMPOSITE)(const char* name, bool loop);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_PLAY_COMPOSITE_D)(const char* name, double loop);
/*
	Displays the `Chroma` animation frame on `Chroma` hardware given the `frameIndex`. 
	Returns the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_PREVIEW_FRAME)(int animationId, int frameIndex);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_PREVIEW_FRAME_D)(double animationId, double frameIndex);
/*
	Displays the `Chroma` animation frame on `Chroma` hardware given the `frameIndex`. 
	Animaton is referenced by name.
*/
typedef void		(*PLUGIN_PREVIEW_FRAME_NAME)(const char* path, int frameIndex);
/*
	Reduce the frames of the animation by removing every nth element. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_REDUCE_FRAMES)(int animationId, int n);
/*
	Reduce the frames of the animation by removing every nth element. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_REDUCE_FRAMES_NAME)(const char* path, int n);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_REDUCE_FRAMES_NAME_D)(const char* path, double n);
/*
	Resets the `Chroma` animation to 1 blank frame. Returns the animation id 
	upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_RESET_ANIMATION)(int animationId);
/*
	Resume the animation with loop `ON` or `OFF` referenced by id.
*/
typedef void		(*PLUGIN_RESUME_ANIMATION)(int animationId, bool loop);
/*
	Resume the animation with loop `ON` or `OFF` referenced by name.
*/
typedef void		(*PLUGIN_RESUME_ANIMATION_NAME)(const char* path, bool loop);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_RESUME_ANIMATION_NAME_D)(const char* path, double loop);
/*
	Reverse the animation frame order of the `Chroma` animation. Returns the 
	animation id upon success. Returns -1 upon failure. Animation is referenced 
	by id.
*/
typedef int			(*PLUGIN_REVERSE)(int animationId);
/*
	Reverse the animation frame order of the `Chroma` animation. Animation is 
	referenced by id.
*/
typedef void		(*PLUGIN_REVERSE_ALL_FRAMES)(int animationId);
/*
	Reverse the animation frame order of the `Chroma` animation. Animation is 
	referenced by name.
*/
typedef void		(*PLUGIN_REVERSE_ALL_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_REVERSE_ALL_FRAMES_NAME_D)(const char* path);
/*
	Save the animation referenced by id to the path specified.
*/
typedef int			(*PLUGIN_SAVE_ANIMATION)(int animationId, const char* path);
/*
	Save the named animation to the target path specified.
*/
typedef int			(*PLUGIN_SAVE_ANIMATION_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	Set the animation color for a frame given the `1D` `led`. The `led` should 
	be greater than or equal to 0 and less than the `MaxLeds`. The animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_1D_COLOR)(int animationId, int frameId, int led, int color);
/*
	Set the animation color for a frame given the `1D` `led`. The `led` should 
	be greater than or equal to 0 and less than the `MaxLeds`. The animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_1D_COLOR_NAME)(const char* path, int frameId, int led, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_1D_COLOR_NAME_D)(const char* path, double frameId, double led, double color);
/*
	Set the animation color for a frame given the `2D` `row` and `column`. The 
	`row` should be greater than or equal to 0 and less than the `MaxRow`. 
	The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
	The animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_2D_COLOR)(int animationId, int frameId, int row, int column, int color);
/*
	Set the animation color for a frame given the `2D` `row` and `column`. The 
	`row` should be greater than or equal to 0 and less than the `MaxRow`. 
	The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
	The animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_2D_COLOR_NAME)(const char* path, int frameId, int row, int column, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_2D_COLOR_NAME_D)(const char* path, double frameId, double rowColumnIndex, double color);
/*
	When custom color is set, the custom key mode will be used. The animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_CHROMA_CUSTOM_COLOR_ALL_FRAMES)(int animationId);
/*
	When custom color is set, the custom key mode will be used. The animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_CHROMA_CUSTOM_COLOR_ALL_FRAMES_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_CHROMA_CUSTOM_COLOR_ALL_FRAMES_NAME_D)(const char* path);
/*
	Set the Chroma custom key color flag on all frames. `True` changes the layout 
	from grid to key. `True` changes the layout from key to grid. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_CHROMA_CUSTOM_FLAG)(int animationId, bool flag);
/*
	Set the Chroma custom key color flag on all frames. `True` changes the layout 
	from grid to key. `True` changes the layout from key to grid. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_CHROMA_CUSTOM_FLAG_NAME)(const char* path, bool flag);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_CHROMA_CUSTOM_FLAG_NAME_D)(const char* path, double flag);
/*
	Set the current frame of the animation referenced by id.
*/
typedef void		(*PLUGIN_SET_CURRENT_FRAME)(int animationId, int frameId);
/*
	Set the current frame of the animation referenced by name.
*/
typedef void		(*PLUGIN_SET_CURRENT_FRAME_NAME)(const char* path, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_CURRENT_FRAME_NAME_D)(const char* path, double frameId);
/*
	Changes the `deviceType` and `device` of a `Chroma` animation. If the device 
	is changed, the `Chroma` animation will be reset with 1 blank frame. Returns 
	the animation id upon success. Returns -1 upon failure.
*/
typedef int			(*PLUGIN_SET_DEVICE)(int animationId, int deviceType, int device);
/*
	SetEffect will display the referenced effect id.
*/
typedef RZRESULT	(*PLUGIN_SET_EFFECT)(const ChromaSDK::FChromaSDKGuid& effectId);
/*
	When the idle animation is used, the named animation will play when no other 
	animations are playing. Reference the animation by id.
*/
typedef void		(*PLUGIN_SET_IDLE_ANIMATION)(int animationId);
/*
	When the idle animation is used, the named animation will play when no other 
	animations are playing. Reference the animation by name.
*/
typedef void		(*PLUGIN_SET_IDLE_ANIMATION_NAME)(const char* path);
/*
	Set animation key to a static color for the given frame.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR)(int animationId, int frameId, int rzkey, int color);
/*
	Set the key to the specified key color for all frames. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_ALL_FRAMES)(int animationId, int rzkey, int color);
/*
	Set the key to the specified key color for all frames. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_ALL_FRAMES_NAME)(const char* path, int rzkey, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_COLOR_ALL_FRAMES_NAME_D)(const char* path, double rzkey, double color);
/*
	Set the key to the specified key color for all frames. Animation is referenced 
	by id.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_ALL_FRAMES_RGB)(int animationId, int rzkey, int red, int green, int blue);
/*
	Set the key to the specified key color for all frames. Animation is referenced 
	by name.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_ALL_FRAMES_RGB_NAME)(const char* path, int rzkey, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_COLOR_ALL_FRAMES_RGB_NAME_D)(const char* path, double rzkey, double red, double green, double blue);
/*
	Set animation key to a static color for the given frame.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_NAME)(const char* path, int frameId, int rzkey, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_COLOR_NAME_D)(const char* path, double frameId, double rzkey, double color);
/*
	Set the key to the specified key color for the specified frame. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_RGB)(int animationId, int frameId, int rzkey, int red, int green, int blue);
/*
	Set the key to the specified key color for the specified frame. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEY_COLOR_RGB_NAME)(const char* path, int frameId, int rzkey, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_COLOR_RGB_NAME_D)(const char* path, double frameId, double rzkey, double red, double green, double blue);
/*
	Set animation key to a static color for the given frame if the existing 
	color is not already black.
*/
typedef void		(*PLUGIN_SET_KEY_NON_ZERO_COLOR)(int animationId, int frameId, int rzkey, int color);
/*
	Set animation key to a static color for the given frame if the existing 
	color is not already black.
*/
typedef void		(*PLUGIN_SET_KEY_NON_ZERO_COLOR_NAME)(const char* path, int frameId, int rzkey, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_NON_ZERO_COLOR_NAME_D)(const char* path, double frameId, double rzkey, double color);
/*
	Set the key to the specified key color for the specified frame where color 
	is not black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEY_NON_ZERO_COLOR_RGB)(int animationId, int frameId, int rzkey, int red, int green, int blue);
/*
	Set the key to the specified key color for the specified frame where color 
	is not black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEY_NON_ZERO_COLOR_RGB_NAME)(const char* path, int frameId, int rzkey, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_NON_ZERO_COLOR_RGB_NAME_D)(const char* path, double frameId, double rzkey, double red, double green, double blue);
/*
	Set an array of animation keys to a static color for the given frame. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR)(int animationId, int frameId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_ALL_FRAMES)(int animationId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_ALL_FRAMES_NAME)(const char* path, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_ALL_FRAMES_RGB)(int animationId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for all frames. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_ALL_FRAMES_RGB_NAME)(const char* path, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_NAME)(const char* path, int frameId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for the given frame. Animation 
	is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_RGB)(int animationId, int frameId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame. Animation 
	is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_COLOR_RGB_NAME)(const char* path, int frameId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame if 
	the existing color is not already black.
*/
typedef void		(*PLUGIN_SET_KEYS_NON_ZERO_COLOR)(int animationId, int frameId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is not black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_NON_ZERO_COLOR_ALL_FRAMES)(int animationId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames if the existing 
	color is not already black. Reference animation by name.
*/
typedef void		(*PLUGIN_SET_KEYS_NON_ZERO_COLOR_ALL_FRAMES_NAME)(const char* path, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for the given frame if 
	the existing color is not already black. Reference animation by name.
*/
typedef void		(*PLUGIN_SET_KEYS_NON_ZERO_COLOR_NAME)(const char* path, int frameId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is not black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_NON_ZERO_COLOR_RGB)(int animationId, int frameId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is not black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_NON_ZERO_COLOR_RGB_NAME)(const char* path, int frameId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR)(int animationId, int frameId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames where the 
	color is black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES)(int animationId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames where the 
	color is black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES_NAME)(const char* path, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for all frames where the 
	color is black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES_RGB)(int animationId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for all frames where the 
	color is black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES_RGB_NAME)(const char* path, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_NAME)(const char* path, int frameId, const int* rzkeys, int keyCount, int color);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_RGB)(int animationId, int frameId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set an array of animation keys to a static color for the given frame where 
	the color is black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEYS_ZERO_COLOR_RGB_NAME)(const char* path, int frameId, const int* rzkeys, int keyCount, int red, int green, int blue);
/*
	Set animation key to a static color for the given frame where the color 
	is black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEY_ZERO_COLOR)(int animationId, int frameId, int rzkey, int color);
/*
	Set animation key to a static color for the given frame where the color 
	is black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEY_ZERO_COLOR_NAME)(const char* path, int frameId, int rzkey, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_ZERO_COLOR_NAME_D)(const char* path, double frameId, double rzkey, double color);
/*
	Set animation key to a static color for the given frame where the color 
	is black. Animation is referenced by id.
*/
typedef void		(*PLUGIN_SET_KEY_ZERO_COLOR_RGB)(int animationId, int frameId, int rzkey, int red, int green, int blue);
/*
	Set animation key to a static color for the given frame where the color 
	is black. Animation is referenced by name.
*/
typedef void		(*PLUGIN_SET_KEY_ZERO_COLOR_RGB_NAME)(const char* path, int frameId, int rzkey, int red, int green, int blue);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SET_KEY_ZERO_COLOR_RGB_NAME_D)(const char* path, double frameId, double rzkey, double red, double green, double blue);
/*
	Invokes the setup for a debug logging callback so that `stdout` is redirected 
	to the callback. This is used by `Unity` so that debug messages can appear 
	in the console window.
*/
typedef void		(*PLUGIN_SET_LOG_DELEGATE)(DebugLogPtr fp);
/*
	`PluginStaticColor` sets the target device to the static color.
*/
typedef void		(*PLUGIN_STATIC_COLOR)(int deviceType, int device, int color);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_STATIC_COLOR_D)(double deviceType, double device, double color);
/*
	`PluginStopAll` will automatically stop all animations that are playing.
*/
typedef void		(*PLUGIN_STOP_ALL)();
/*
	Stops animation playback if in progress. Returns the animation id upon success. 
	Returns -1 upon failure.
*/
typedef int			(*PLUGIN_STOP_ANIMATION)(int animationId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_STOP_ANIMATION_D)(double animationId);
/*
	`PluginStopAnimationName` automatically handles initializing the `ChromaSDK`. 
	The named `.chroma` animation file will be automatically opened. The animation 
	will stop if playing.
*/
typedef void		(*PLUGIN_STOP_ANIMATION_NAME)(const char* path);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_STOP_ANIMATION_NAME_D)(const char* path);
/*
	`PluginStopAnimationType` automatically handles initializing the `ChromaSDK`. 
	If any animation is playing for the `deviceType` and `device` combination, 
	it will be stopped.
*/
typedef void		(*PLUGIN_STOP_ANIMATION_TYPE)(int deviceType, int device);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_STOP_ANIMATION_TYPE_D)(double deviceType, double device);
/*
	`PluginStopComposite` automatically handles initializing the `ChromaSDK`. 
	The named animation files for the `.chroma` set will be automatically opened. 
	The set of animations will be stopped if playing.
*/
typedef void		(*PLUGIN_STOP_COMPOSITE)(const char* name);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_STOP_COMPOSITE_D)(const char* name);
/*
	Subtract the source color from the target color for all frames where the 
	target color is not black. Source and target are referenced by id.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Subtract the source color from the target color for all frames where the 
	target color is not black. Source and target are referenced by name.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Subtract the source color from the target color for all frames where the 
	target color is not black starting at offset for the length of the source. 
	Source and target are referenced by id.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Subtract the source color from the target color for all frames where the 
	target color is not black starting at offset for the length of the source. 
	Source and target are referenced by name.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Subtract the source color from the target where color is not black for the 
	source frame and target offset frame, reference source and target by id.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_OFFSET)(int sourceAnimationId, int targetAnimationId, int frameId, int offset);
/*
	Subtract the source color from the target where color is not black for the 
	source frame and target offset frame, reference source and target by name.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double offset);
/*
	Subtract the source color from the target color where the target color is 
	not black for all frames. Reference source and target by id.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES)(int sourceAnimationId, int targetAnimationId);
/*
	Subtract the source color from the target color where the target color is 
	not black for all frames. Reference source and target by name.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME)(const char* sourceAnimation, const char* targetAnimation);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D)(const char* sourceAnimation, const char* targetAnimation);
/*
	Subtract the source color from the target color where the target color is 
	not black for all frames starting at the target offset for the length of 
	the source. Reference source and target by id.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET)(int sourceAnimationId, int targetAnimationId, int offset);
/*
	Subtract the source color from the target color where the target color is 
	not black for all frames starting at the target offset for the length of 
	the source. Reference source and target by name.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double offset);
/*
	Subtract the source color from the target color where the target color is 
	not black from the source frame to the target offset frame. Reference source 
	and target by id.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_OFFSET)(int sourceAnimationId, int targetAnimationId, int frameId, int offset);
/*
	Subtract the source color from the target color where the target color is 
	not black from the source frame to the target offset frame. Reference source 
	and target by name.
*/
typedef void		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME)(const char* sourceAnimation, const char* targetAnimation, int frameId, int offset);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME_D)(const char* sourceAnimation, const char* targetAnimation, double frameId, double offset);
/*
	Trim the end of the animation. The length of the animation will be the lastFrameId 
	+ 1. Reference the animation by id.
*/
typedef void		(*PLUGIN_TRIM_END_FRAMES)(int animationId, int lastFrameId);
/*
	Trim the end of the animation. The length of the animation will be the lastFrameId 
	+ 1. Reference the animation by name.
*/
typedef void		(*PLUGIN_TRIM_END_FRAMES_NAME)(const char* path, int lastFrameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_TRIM_END_FRAMES_NAME_D)(const char* path, double lastFrameId);
/*
	Remove the frame from the animation. Reference animation by id.
*/
typedef void		(*PLUGIN_TRIM_FRAME)(int animationId, int frameId);
/*
	Remove the frame from the animation. Reference animation by name.
*/
typedef void		(*PLUGIN_TRIM_FRAME_NAME)(const char* path, int frameId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_TRIM_FRAME_NAME_D)(const char* path, double frameId);
/*
	Trim the start of the animation starting at frame 0 for the number of frames. 
	Reference the animation by id.
*/
typedef void		(*PLUGIN_TRIM_START_FRAMES)(int animationId, int numberOfFrames);
/*
	Trim the start of the animation starting at frame 0 for the number of frames. 
	Reference the animation by name.
*/
typedef void		(*PLUGIN_TRIM_START_FRAMES_NAME)(const char* path, int numberOfFrames);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_TRIM_START_FRAMES_NAME_D)(const char* path, double numberOfFrames);
/*
	Uninitializes the `ChromaSDK`. Returns 0 upon success. Returns -1 upon failure.
*/
typedef RZRESULT	(*PLUGIN_UNINIT)();
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_UNINIT_D)();
/*
	Unloads `Chroma` effects to free up resources. Returns the animation id 
	upon success. Returns -1 upon failure. Reference the animation by id.
*/
typedef int			(*PLUGIN_UNLOAD_ANIMATION)(int animationId);
/*
	D suffix for limited data types.
*/
typedef double		(*PLUGIN_UNLOAD_ANIMATION_D)(double animationId);
/*
	Unload the animation effects. Reference the animation by name.
*/
typedef void		(*PLUGIN_UNLOAD_ANIMATION_NAME)(const char* path);
/*
	Unload the the composite set of animation effects. Reference the animation 
	by name.
*/
typedef void		(*PLUGIN_UNLOAD_COMPOSITE)(const char* name);
/*
	Updates the `frameIndex` of the `Chroma` animation and sets the `duration` 
	(in seconds). The `color` is expected to be an array of the dimensions 
	for the `deviceType/device`. The `length` parameter is the size of the 
	`color` array. For `EChromaSDKDevice1DEnum` the array size should be `MAX 
	LEDS`. For `EChromaSDKDevice2DEnum` the array size should be `MAX ROW` 
	* `MAX COLUMN`. Returns the animation id upon success. Returns -1 upon 
	failure.
*/
typedef int			(*PLUGIN_UPDATE_FRAME)(int animationId, int frameIndex, float duration, int* colors, int length);
/*
	When the idle animation flag is true, when no other animations are playing, 
	the idle animation will be used. The idle animation will not be affected 
	by the API calls to PluginIsPlaying, PluginStopAnimationType, PluginGetPlayingAnimationId, 
	and PluginGetPlayingAnimationCount. Then the idle animation flag is false, 
	the idle animation is disabled. `Device` uses `EChromaSDKDeviceEnum` enums.
*/
typedef void		(*PLUGIN_USE_IDLE_ANIMATION)(int device, bool flag);
/*
	Set idle animation flag for all devices.
*/
typedef void		(*PLUGIN_USE_IDLE_ANIMATIONS)(bool flag);
/*
	Set preloading animation flag, which is set to true by default. Reference 
	animation by id.
*/
typedef void		(*PLUGIN_USE_PRELOADING)(int animationId, bool flag);
/*
	Set preloading animation flag, which is set to true by default. Reference 
	animation by name.
*/
typedef void		(*PLUGIN_USE_PRELOADING_NAME)(const char* path, bool flag);
#pragma endregion

#define CHROMASDK_DECLARE_METHOD(Signature, FieldName) static Signature FieldName;

namespace ChromaSDK
{
	class ChromaAnimationAPI
	{
	private:
		static bool _sIsInitializedAPI;

	public:

#pragma region API declare prototypes
		/*
			Adds a frame to the `Chroma` animation and sets the `duration` (in seconds). 
			The `color` is expected to be an array of the dimensions for the `deviceType/device`. 
			The `length` parameter is the size of the `color` array. For `EChromaSDKDevice1DEnum` 
			the array size should be `MAX LEDS`. For `EChromaSDKDevice2DEnum` the array 
			size should be `MAX ROW` * `MAX COLUMN`. Returns the animation id upon 
			success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_FRAME, AddFrame);
		/*
			Add source color to target where color is not black for all frames, reference 
			source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES, AddNonZeroAllKeysAllFrames);
		/*
			Add source color to target where color is not black for all frames, reference 
			source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME, AddNonZeroAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D, AddNonZeroAllKeysAllFramesNameD);
		/*
			Add source color to target where color is not black for all frames starting 
			at offset for the length of the source, reference source and target by 
			id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET, AddNonZeroAllKeysAllFramesOffset);
		/*
			Add source color to target where color is not black for all frames starting 
			at offset for the length of the source, reference source and target by 
			name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, AddNonZeroAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, AddNonZeroAllKeysAllFramesOffsetNameD);
		/*
			Add source color to target where color is not black for the source frame 
			and target offset frame, reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_OFFSET, AddNonZeroAllKeysOffset);
		/*
			Add source color to target where color is not black for the source frame 
			and target offset frame, reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_OFFSET_NAME, AddNonZeroAllKeysOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_ALL_KEYS_OFFSET_NAME_D, AddNonZeroAllKeysOffsetNameD);
		/*
			Add source color to target where the target color is not black for all frames, 
			reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES, AddNonZeroTargetAllKeysAllFrames);
		/*
			Add source color to target where the target color is not black for all frames, 
			reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME, AddNonZeroTargetAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D, AddNonZeroTargetAllKeysAllFramesNameD);
		/*
			Add source color to target where the target color is not black for all frames 
			starting at offset for the length of the source, reference source and target 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET, AddNonZeroTargetAllKeysAllFramesOffset);
		/*
			Add source color to target where the target color is not black for all frames 
			starting at offset for the length of the source, reference source and target 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, AddNonZeroTargetAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, AddNonZeroTargetAllKeysAllFramesOffsetNameD);
		/*
			Add source color to target where target color is not blank from the source 
			frame to the target offset frame, reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_OFFSET, AddNonZeroTargetAllKeysOffset);
		/*
			Add source color to target where target color is not blank from the source 
			frame to the target offset frame, reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME, AddNonZeroTargetAllKeysOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_ADD_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME_D, AddNonZeroTargetAllKeysOffsetNameD);
		/*
			Append all source frames to the target animation, reference source and target 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_APPEND_ALL_FRAMES, AppendAllFrames);
		/*
			Append all source frames to the target animation, reference source and target 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_APPEND_ALL_FRAMES_NAME, AppendAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_APPEND_ALL_FRAMES_NAME_D, AppendAllFramesNameD);
		/*
			`PluginClearAll` will issue a `CLEAR` effect for all devices.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLEAR_ALL, ClearAll);
		/*
			`PluginClearAnimationType` will issue a `CLEAR` effect for the given device.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLEAR_ANIMATION_TYPE, ClearAnimationType);
		/*
			`PluginCloseAll` closes all open animations so they can be reloaded from 
			disk. The set of animations will be stopped if playing.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_ALL, CloseAll);
		/*
			Closes the `Chroma` animation to free up resources referenced by id. Returns 
			the animation id upon success. Returns -1 upon failure. This might be used 
			while authoring effects if there was a change necessitating re-opening 
			the animation. The animation id can no longer be used once closed.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_ANIMATION, CloseAnimation);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_ANIMATION_D, CloseAnimationD);
		/*
			Closes the `Chroma` animation referenced by name so that the animation can 
			be reloaded from disk.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_ANIMATION_NAME, CloseAnimationName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_ANIMATION_NAME_D, CloseAnimationNameD);
		/*
			`PluginCloseComposite` closes a set of animations so they can be reloaded 
			from disk. The set of animations will be stopped if playing.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_COMPOSITE, CloseComposite);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CLOSE_COMPOSITE_D, CloseCompositeD);
		/*
			Copy animation to named target animation in memory. If target animation 
			exists, close first. Source is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ANIMATION, CopyAnimation);
		/*
			Copy animation to named target animation in memory. If target animation 
			exists, close first. Source is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ANIMATION_NAME, CopyAnimationName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ANIMATION_NAME_D, CopyAnimationNameD);
		/*
			Copy blue channel to other channels for all frames. Intensity range is 0.0 
			to 1.0. Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_BLUE_CHANNEL_ALL_FRAMES, CopyBlueChannelAllFrames);
		/*
			Copy blue channel to other channels for all frames. Intensity range is 0.0 
			to 1.0. Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_BLUE_CHANNEL_ALL_FRAMES_NAME, CopyBlueChannelAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_BLUE_CHANNEL_ALL_FRAMES_NAME_D, CopyBlueChannelAllFramesNameD);
		/*
			Copy green channel to other channels for all frames. Intensity range is 
			0.0 to 1.0. Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_GREEN_CHANNEL_ALL_FRAMES, CopyGreenChannelAllFrames);
		/*
			Copy green channel to other channels for all frames. Intensity range is 
			0.0 to 1.0. Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_GREEN_CHANNEL_ALL_FRAMES_NAME, CopyGreenChannelAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_GREEN_CHANNEL_ALL_FRAMES_NAME_D, CopyGreenChannelAllFramesNameD);
		/*
			Copy animation key color from the source animation to the target animation 
			for the given frame. Reference the source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR, CopyKeyColor);
		/*
			Copy animation key color from the source animation to the target animation 
			for all frames. Reference the source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_ALL_FRAMES, CopyKeyColorAllFrames);
		/*
			Copy animation key color from the source animation to the target animation 
			for all frames. Reference the source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_NAME, CopyKeyColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_NAME_D, CopyKeyColorAllFramesNameD);
		/*
			Copy animation key color from the source animation to the target animation 
			for all frames, starting at the offset for the length of the source animation. 
			Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_OFFSET, CopyKeyColorAllFramesOffset);
		/*
			Copy animation key color from the source animation to the target animation 
			for all frames, starting at the offset for the length of the source animation. 
			Source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_OFFSET_NAME, CopyKeyColorAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_ALL_FRAMES_OFFSET_NAME_D, CopyKeyColorAllFramesOffsetNameD);
		/*
			Copy animation key color from the source animation to the target animation 
			for the given frame.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_NAME, CopyKeyColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEY_COLOR_NAME_D, CopyKeyColorNameD);
		/*
			Copy animation color for a set of keys from the source animation to the 
			target animation for the given frame. Reference the source and target by 
			id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEYS_COLOR, CopyKeysColor);
		/*
			Copy animation color for a set of keys from the source animation to the 
			target animation for all frames. Reference the source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEYS_COLOR_ALL_FRAMES, CopyKeysColorAllFrames);
		/*
			Copy animation color for a set of keys from the source animation to the 
			target animation for all frames. Reference the source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEYS_COLOR_ALL_FRAMES_NAME, CopyKeysColorAllFramesName);
		/*
			Copy animation color for a set of keys from the source animation to the 
			target animation for the given frame. Reference the source and target by 
			name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEYS_COLOR_NAME, CopyKeysColorName);
		/*
			Copy animation color for a set of keys from the source animation to the 
			target animation from the source frame to the target frame. Reference the 
			source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEYS_COLOR_OFFSET, CopyKeysColorOffset);
		/*
			Copy animation color for a set of keys from the source animation to the 
			target animation from the source frame to the target frame. Reference the 
			source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_KEYS_COLOR_OFFSET_NAME, CopyKeysColorOffsetName);
		/*
			Copy source animation to target animation for the given frame. Source and 
			target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS, CopyNonZeroAllKeys);
		/*
			Copy nonzero colors from a source animation to a target animation for all 
			frames. Reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES, CopyNonZeroAllKeysAllFrames);
		/*
			Copy nonzero colors from a source animation to a target animation for all 
			frames. Reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME, CopyNonZeroAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D, CopyNonZeroAllKeysAllFramesNameD);
		/*
			Copy nonzero colors from a source animation to a target animation for all 
			frames starting at the offset for the length of the source animation. The 
			source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET, CopyNonZeroAllKeysAllFramesOffset);
		/*
			Copy nonzero colors from a source animation to a target animation for all 
			frames starting at the offset for the length of the source animation. The 
			source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, CopyNonZeroAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, CopyNonZeroAllKeysAllFramesOffsetNameD);
		/*
			Copy nonzero colors from source animation to target animation for the specified 
			frame. Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_NAME, CopyNonZeroAllKeysName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_NAME_D, CopyNonZeroAllKeysNameD);
		/*
			Copy nonzero colors from the source animation to the target animation from 
			the source frame to the target offset frame. Source and target are referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_OFFSET, CopyNonZeroAllKeysOffset);
		/*
			Copy nonzero colors from the source animation to the target animation from 
			the source frame to the target offset frame. Source and target are referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_OFFSET_NAME, CopyNonZeroAllKeysOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_ALL_KEYS_OFFSET_NAME_D, CopyNonZeroAllKeysOffsetNameD);
		/*
			Copy animation key color from the source animation to the target animation 
			for the given frame where color is not zero.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_KEY_COLOR, CopyNonZeroKeyColor);
		/*
			Copy animation key color from the source animation to the target animation 
			for the given frame where color is not zero.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_KEY_COLOR_NAME, CopyNonZeroKeyColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_KEY_COLOR_NAME_D, CopyNonZeroKeyColorNameD);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for the specified frame. Source and target 
			are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS, CopyNonZeroTargetAllKeys);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for all frames. Source and target are referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES, CopyNonZeroTargetAllKeysAllFrames);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for all frames. Source and target are referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME, CopyNonZeroTargetAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D, CopyNonZeroTargetAllKeysAllFramesNameD);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for all frames. Source and target are referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET, CopyNonZeroTargetAllKeysAllFramesOffset);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for all frames starting at the target offset 
			for the length of the source animation. Source and target animations are 
			referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, CopyNonZeroTargetAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, CopyNonZeroTargetAllKeysAllFramesOffsetNameD);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for the specified frame. The source and target 
			are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_NAME, CopyNonZeroTargetAllKeysName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_NAME_D, CopyNonZeroTargetAllKeysNameD);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for the specified source frame and target offset 
			frame. The source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_OFFSET, CopyNonZeroTargetAllKeysOffset);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is nonzero for the specified source frame and target offset 
			frame. The source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME, CopyNonZeroTargetAllKeysOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME_D, CopyNonZeroTargetAllKeysOffsetNameD);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is zero for all frames. Source and target are referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ZERO_ALL_KEYS_ALL_FRAMES, CopyNonZeroTargetZeroAllKeysAllFrames);
		/*
			Copy nonzero colors from the source animation to the target animation where 
			the target color is zero for all frames. Source and target are referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ZERO_ALL_KEYS_ALL_FRAMES_NAME, CopyNonZeroTargetZeroAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_NON_ZERO_TARGET_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D, CopyNonZeroTargetZeroAllKeysAllFramesNameD);
		/*
			Copy red channel to other channels for all frames. Intensity range is 0.0 
			to 1.0. Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_RED_CHANNEL_ALL_FRAMES, CopyRedChannelAllFrames);
		/*
			Copy green channel to other channels for all frames. Intensity range is 
			0.0 to 1.0. Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_RED_CHANNEL_ALL_FRAMES_NAME, CopyRedChannelAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_RED_CHANNEL_ALL_FRAMES_NAME_D, CopyRedChannelAllFramesNameD);
		/*
			Copy zero colors from source animation to target animation for all frames. 
			Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES, CopyZeroAllKeysAllFrames);
		/*
			Copy zero colors from source animation to target animation for all frames. 
			Source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_NAME, CopyZeroAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D, CopyZeroAllKeysAllFramesNameD);
		/*
			Copy zero colors from source animation to target animation for all frames 
			starting at the target offset for the length of the source animation. Source 
			and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET, CopyZeroAllKeysAllFramesOffset);
		/*
			Copy zero colors from source animation to target animation for all frames 
			starting at the target offset for the length of the source animation. Source 
			and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, CopyZeroAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, CopyZeroAllKeysAllFramesOffsetNameD);
		/*
			Copy zero key color from source animation to target animation for the specified 
			frame. Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_KEY_COLOR, CopyZeroKeyColor);
		/*
			Copy zero key color from source animation to target animation for the specified 
			frame. Source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_KEY_COLOR_NAME, CopyZeroKeyColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_KEY_COLOR_NAME_D, CopyZeroKeyColorNameD);
		/*
			Copy nonzero color from source animation to target animation where target 
			is zero for all frames. Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_TARGET_ALL_KEYS_ALL_FRAMES, CopyZeroTargetAllKeysAllFrames);
		/*
			Copy nonzero color from source animation to target animation where target 
			is zero for all frames. Source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME, CopyZeroTargetAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_COPY_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D, CopyZeroTargetAllKeysAllFramesNameD);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_CHROMA_LINK_EFFECT, CoreCreateChromaLinkEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_EFFECT, CoreCreateEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_HEADSET_EFFECT, CoreCreateHeadsetEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_KEYBOARD_EFFECT, CoreCreateKeyboardEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_KEYPAD_EFFECT, CoreCreateKeypadEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_MOUSE_EFFECT, CoreCreateMouseEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_CREATE_MOUSEPAD_EFFECT, CoreCreateMousepadEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_DELETE_EFFECT, CoreDeleteEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_INIT, CoreInit);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_INIT_SDK, CoreInitSDK);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_QUERY_DEVICE, CoreQueryDevice);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_SET_EFFECT, CoreSetEffect);
		/*
			Direct access to low level API.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CORE_UNINIT, CoreUnInit);
		/*
			Creates a `Chroma` animation at the given path. The `deviceType` parameter 
			uses `EChromaSDKDeviceTypeEnum` as an integer. The `device` parameter uses 
			`EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` as an integer, respective 
			to the `deviceType`. Returns the animation id upon success. Returns -1 
			upon failure. Saves a `Chroma` animation file with the `.chroma` extension 
			at the given path. Returns the animation id upon success. Returns -1 upon 
			failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CREATE_ANIMATION, CreateAnimation);
		/*
			Creates a `Chroma` animation in memory without creating a file. The `deviceType` 
			parameter uses `EChromaSDKDeviceTypeEnum` as an integer. The `device` parameter 
			uses `EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` as an integer, 
			respective to the `deviceType`. Returns the animation id upon success. 
			Returns -1 upon failure. Returns the animation id upon success. Returns 
			-1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CREATE_ANIMATION_IN_MEMORY, CreateAnimationInMemory);
		/*
			Create a device specific effect.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_CREATE_EFFECT, CreateEffect);
		/*
			Delete an effect given the effect id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DELETE_EFFECT, DeleteEffect);
		/*
			Duplicate the first animation frame so that the animation length matches 
			the frame count. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_FIRST_FRAME, DuplicateFirstFrame);
		/*
			Duplicate the first animation frame so that the animation length matches 
			the frame count. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_FIRST_FRAME_NAME, DuplicateFirstFrameName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_FIRST_FRAME_NAME_D, DuplicateFirstFrameNameD);
		/*
			Duplicate all the frames of the animation to double the animation length. 
			Frame 1 becomes frame 1 and 2. Frame 2 becomes frame 3 and 4. And so on. 
			The animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_FRAMES, DuplicateFrames);
		/*
			Duplicate all the frames of the animation to double the animation length. 
			Frame 1 becomes frame 1 and 2. Frame 2 becomes frame 3 and 4. And so on. 
			The animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_FRAMES_NAME, DuplicateFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_FRAMES_NAME_D, DuplicateFramesNameD);
		/*
			Duplicate all the animation frames in reverse so that the animation plays 
			forwards and backwards. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_MIRROR_FRAMES, DuplicateMirrorFrames);
		/*
			Duplicate all the animation frames in reverse so that the animation plays 
			forwards and backwards. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_MIRROR_FRAMES_NAME, DuplicateMirrorFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_DUPLICATE_MIRROR_FRAMES_NAME_D, DuplicateMirrorFramesNameD);
		/*
			Fade the animation to black starting at the fade frame index to the end 
			of the animation. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FADE_END_FRAMES, FadeEndFrames);
		/*
			Fade the animation to black starting at the fade frame index to the end 
			of the animation. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FADE_END_FRAMES_NAME, FadeEndFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FADE_END_FRAMES_NAME_D, FadeEndFramesNameD);
		/*
			Fade the animation from black to full color starting at 0 to the fade frame 
			index. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FADE_START_FRAMES, FadeStartFrames);
		/*
			Fade the animation from black to full color starting at 0 to the fade frame 
			index. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FADE_START_FRAMES_NAME, FadeStartFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FADE_START_FRAMES_NAME_D, FadeStartFramesNameD);
		/*
			Set the RGB value for all colors in the specified frame. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR, FillColor);
		/*
			Set the RGB value for all colors for all frames. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_ALL_FRAMES, FillColorAllFrames);
		/*
			Set the RGB value for all colors for all frames. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_ALL_FRAMES_NAME, FillColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_ALL_FRAMES_NAME_D, FillColorAllFramesNameD);
		/*
			Set the RGB value for all colors for all frames. Use the range of 0 to 255 
			for red, green, and blue parameters. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_ALL_FRAMES_RGB, FillColorAllFramesRGB);
		/*
			Set the RGB value for all colors for all frames. Use the range of 0 to 255 
			for red, green, and blue parameters. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_ALL_FRAMES_RGB_NAME, FillColorAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_ALL_FRAMES_RGB_NAME_D, FillColorAllFramesRGBNameD);
		/*
			Set the RGB value for all colors in the specified frame. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_NAME, FillColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_NAME_D, FillColorNameD);
		/*
			Set the RGB value for all colors in the specified frame. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_RGB, FillColorRGB);
		/*
			Set the RGB value for all colors in the specified frame. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_RGB_NAME, FillColorRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_COLOR_RGB_NAME_D, FillColorRGBNameD);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors in the specified 
			frame. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR, FillNonZeroColor);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors for all frames. 
			Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES, FillNonZeroColorAllFrames);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors for all frames. 
			Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_NAME, FillNonZeroColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_NAME_D, FillNonZeroColorAllFramesNameD);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors for all frames. 
			Use the range of 0 to 255 for red, green, and blue parameters. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_RGB, FillNonZeroColorAllFramesRGB);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors for all frames. 
			Use the range of 0 to 255 for red, green, and blue parameters. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_RGB_NAME, FillNonZeroColorAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_ALL_FRAMES_RGB_NAME_D, FillNonZeroColorAllFramesRGBNameD);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors in the specified 
			frame. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_NAME, FillNonZeroColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_NAME_D, FillNonZeroColorNameD);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors in the specified 
			frame. Use the range of 0 to 255 for red, green, and blue parameters. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_RGB, FillNonZeroColorRGB);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Set the RGB value for a subset of colors in the specified 
			frame. Use the range of 0 to 255 for red, green, and blue parameters. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_RGB_NAME, FillNonZeroColorRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_NON_ZERO_COLOR_RGB_NAME_D, FillNonZeroColorRGBNameD);
		/*
			Fill the frame with random RGB values for the given frame. Animation is 
			referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS, FillRandomColors);
		/*
			Fill the frame with random RGB values for all frames. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_ALL_FRAMES, FillRandomColorsAllFrames);
		/*
			Fill the frame with random RGB values for all frames. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_ALL_FRAMES_NAME, FillRandomColorsAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_ALL_FRAMES_NAME_D, FillRandomColorsAllFramesNameD);
		/*
			Fill the frame with random black and white values for the specified frame. 
			Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE, FillRandomColorsBlackAndWhite);
		/*
			Fill the frame with random black and white values for all frames. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_ALL_FRAMES, FillRandomColorsBlackAndWhiteAllFrames);
		/*
			Fill the frame with random black and white values for all frames. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_ALL_FRAMES_NAME, FillRandomColorsBlackAndWhiteAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_ALL_FRAMES_NAME_D, FillRandomColorsBlackAndWhiteAllFramesNameD);
		/*
			Fill the frame with random black and white values for the specified frame. 
			Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_NAME, FillRandomColorsBlackAndWhiteName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_BLACK_AND_WHITE_NAME_D, FillRandomColorsBlackAndWhiteNameD);
		/*
			Fill the frame with random RGB values for the given frame. Animation is 
			referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_NAME, FillRandomColorsName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_RANDOM_COLORS_NAME_D, FillRandomColorsNameD);
		/*
			Fill the specified frame with RGB color where the animation color is less 
			than the RGB threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS, FillThresholdColors);
		/*
			Fill all frames with RGB color where the animation color is less than the 
			RGB threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES, FillThresholdColorsAllFrames);
		/*
			Fill all frames with RGB color where the animation color is less than the 
			RGB threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_NAME, FillThresholdColorsAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_NAME_D, FillThresholdColorsAllFramesNameD);
		/*
			Fill all frames with RGB color where the animation color is less than the 
			threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_RGB, FillThresholdColorsAllFramesRGB);
		/*
			Fill all frames with RGB color where the animation color is less than the 
			threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_RGB_NAME, FillThresholdColorsAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_ALL_FRAMES_RGB_NAME_D, FillThresholdColorsAllFramesRGBNameD);
		/*
			Fill all frames with the min RGB color where the animation color is less 
			than the min threshold AND with the max RGB color where the animation is 
			more than the max threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_ALL_FRAMES_RGB, FillThresholdColorsMinMaxAllFramesRGB);
		/*
			Fill all frames with the min RGB color where the animation color is less 
			than the min threshold AND with the max RGB color where the animation is 
			more than the max threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_ALL_FRAMES_RGB_NAME, FillThresholdColorsMinMaxAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_ALL_FRAMES_RGB_NAME_D, FillThresholdColorsMinMaxAllFramesRGBNameD);
		/*
			Fill the specified frame with the min RGB color where the animation color 
			is less than the min threshold AND with the max RGB color where the animation 
			is more than the max threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_RGB, FillThresholdColorsMinMaxRGB);
		/*
			Fill the specified frame with the min RGB color where the animation color 
			is less than the min threshold AND with the max RGB color where the animation 
			is more than the max threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_RGB_NAME, FillThresholdColorsMinMaxRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_MIN_MAX_RGB_NAME_D, FillThresholdColorsMinMaxRGBNameD);
		/*
			Fill the specified frame with RGB color where the animation color is less 
			than the RGB threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_NAME, FillThresholdColorsName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_NAME_D, FillThresholdColorsNameD);
		/*
			Fill the specified frame with RGB color where the animation color is less 
			than the RGB threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_RGB, FillThresholdColorsRGB);
		/*
			Fill the specified frame with RGB color where the animation color is less 
			than the RGB threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_RGB_NAME, FillThresholdColorsRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_COLORS_RGB_NAME_D, FillThresholdColorsRGBNameD);
		/*
			Fill all frames with RGB color where the animation color is less than the 
			RGB threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_RGB_COLORS_ALL_FRAMES_RGB, FillThresholdRGBColorsAllFramesRGB);
		/*
			Fill all frames with RGB color where the animation color is less than the 
			RGB threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_RGB_COLORS_ALL_FRAMES_RGB_NAME, FillThresholdRGBColorsAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_RGB_COLORS_ALL_FRAMES_RGB_NAME_D, FillThresholdRGBColorsAllFramesRGBNameD);
		/*
			Fill the specified frame with RGB color where the animation color is less 
			than the RGB threshold. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_RGB_COLORS_RGB, FillThresholdRGBColorsRGB);
		/*
			Fill the specified frame with RGB color where the animation color is less 
			than the RGB threshold. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_RGB_COLORS_RGB_NAME, FillThresholdRGBColorsRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_THRESHOLD_RGB_COLORS_RGB_NAME_D, FillThresholdRGBColorsRGBNameD);
		/*
			Fill the specified frame with RGB color where the animation color is zero. 
			Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR, FillZeroColor);
		/*
			Fill all frames with RGB color where the animation color is zero. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES, FillZeroColorAllFrames);
		/*
			Fill all frames with RGB color where the animation color is zero. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_NAME, FillZeroColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_NAME_D, FillZeroColorAllFramesNameD);
		/*
			Fill all frames with RGB color where the animation color is zero. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_RGB, FillZeroColorAllFramesRGB);
		/*
			Fill all frames with RGB color where the animation color is zero. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_RGB_NAME, FillZeroColorAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_ALL_FRAMES_RGB_NAME_D, FillZeroColorAllFramesRGBNameD);
		/*
			Fill the specified frame with RGB color where the animation color is zero. 
			Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_NAME, FillZeroColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_NAME_D, FillZeroColorNameD);
		/*
			Fill the specified frame with RGB color where the animation color is zero. 
			Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_RGB, FillZeroColorRGB);
		/*
			Fill the specified frame with RGB color where the animation color is zero. 
			Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_RGB_NAME, FillZeroColorRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_FILL_ZERO_COLOR_RGB_NAME_D, FillZeroColorRGBNameD);
		/*
			Get the animation color for a frame given the `1D` `led`. The `led` should 
			be greater than or equal to 0 and less than the `MaxLeds`. Animation is 
			referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_1D_COLOR, Get1DColor);
		/*
			Get the animation color for a frame given the `1D` `led`. The `led` should 
			be greater than or equal to 0 and less than the `MaxLeds`. Animation is 
			referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_1D_COLOR_NAME, Get1DColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_1D_COLOR_NAME_D, Get1DColorNameD);
		/*
			Get the animation color for a frame given the `2D` `row` and `column`. The 
			`row` should be greater than or equal to 0 and less than the `MaxRow`. 
			The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
			Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_2D_COLOR, Get2DColor);
		/*
			Get the animation color for a frame given the `2D` `row` and `column`. The 
			`row` should be greater than or equal to 0 and less than the `MaxRow`. 
			The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
			Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_2D_COLOR_NAME, Get2DColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_2D_COLOR_NAME_D, Get2DColorNameD);
		/*
			Get the animation id for the named animation.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_ANIMATION, GetAnimation);
		/*
			`PluginGetAnimationCount` will return the number of loaded animations.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_ANIMATION_COUNT, GetAnimationCount);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_ANIMATION_D, GetAnimationD);
		/*
			`PluginGetAnimationId` will return the `animationId` given the `index` of 
			the loaded animation. The `index` is zero-based and less than the number 
			returned by `PluginGetAnimationCount`. Use `PluginGetAnimationName` to 
			get the name of the animation.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_ANIMATION_ID, GetAnimationId);
		/*
			`PluginGetAnimationName` takes an `animationId` and returns the name of 
			the animation of the `.chroma` animation file. If a name is not available 
			then an empty string will be returned.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_ANIMATION_NAME, GetAnimationName);
		/*
			Get the current frame of the animation referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_CURRENT_FRAME, GetCurrentFrame);
		/*
			Get the current frame of the animation referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_CURRENT_FRAME_NAME, GetCurrentFrameName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_CURRENT_FRAME_NAME_D, GetCurrentFrameNameD);
		/*
			Returns the `EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` of a `Chroma` 
			animation respective to the `deviceType`, as an integer upon success. Returns 
			-1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_DEVICE, GetDevice);
		/*
			Returns the `EChromaSDKDevice1DEnum` or `EChromaSDKDevice2DEnum` of a `Chroma` 
			animation respective to the `deviceType`, as an integer upon success. Returns 
			-1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_DEVICE_NAME, GetDeviceName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_DEVICE_NAME_D, GetDeviceNameD);
		/*
			Returns the `EChromaSDKDeviceTypeEnum` of a `Chroma` animation as an integer 
			upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_DEVICE_TYPE, GetDeviceType);
		/*
			Returns the `EChromaSDKDeviceTypeEnum` of a `Chroma` animation as an integer 
			upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_DEVICE_TYPE_NAME, GetDeviceTypeName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_DEVICE_TYPE_NAME_D, GetDeviceTypeNameD);
		/*
			Gets the frame colors and duration (in seconds) for a `Chroma` animation. 
			The `color` is expected to be an array of the expected dimensions for the 
			`deviceType/device`. The `length` parameter is the size of the `color` 
			array. For `EChromaSDKDevice1DEnum` the array size should be `MAX LEDS`. 
			For `EChromaSDKDevice2DEnum` the array size should be `MAX ROW` * `MAX 
			COLUMN`. Returns the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_FRAME, GetFrame);
		/*
			Returns the frame count of a `Chroma` animation upon success. Returns -1 
			upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_FRAME_COUNT, GetFrameCount);
		/*
			Returns the frame count of a `Chroma` animation upon success. Returns -1 
			upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_FRAME_COUNT_NAME, GetFrameCountName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_FRAME_COUNT_NAME_D, GetFrameCountNameD);
		/*
			Get the color of an animation key for the given frame referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_KEY_COLOR, GetKeyColor);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_KEY_COLOR_D, GetKeyColorD);
		/*
			Get the color of an animation key for the given frame referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_KEY_COLOR_NAME, GetKeyColorName);
		/*
			Returns `RZRESULT_SUCCESS` if the plugin has been initialized successfully. 
			Returns `RZRESULT_DLL_NOT_FOUND` if core Chroma library is not found. Returns 
			`RZRESULT_DLL_INVALID_SIGNATURE` if core Chroma library has an invalid 
			signature.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_LIBRARY_LOADED_STATE, GetLibraryLoadedState);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_LIBRARY_LOADED_STATE_D, GetLibraryLoadedStateD);
		/*
			Returns the `MAX COLUMN` given the `EChromaSDKDevice2DEnum` device as an 
			integer upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_MAX_COLUMN, GetMaxColumn);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_MAX_COLUMN_D, GetMaxColumnD);
		/*
			Returns the MAX LEDS given the `EChromaSDKDevice1DEnum` device as an integer 
			upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_MAX_LEDS, GetMaxLeds);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_MAX_LEDS_D, GetMaxLedsD);
		/*
			Returns the `MAX ROW` given the `EChromaSDKDevice2DEnum` device as an integer 
			upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_MAX_ROW, GetMaxRow);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_MAX_ROW_D, GetMaxRowD);
		/*
			`PluginGetPlayingAnimationCount` will return the number of playing animations.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_PLAYING_ANIMATION_COUNT, GetPlayingAnimationCount);
		/*
			`PluginGetPlayingAnimationId` will return the `animationId` given the `index` 
			of the playing animation. The `index` is zero-based and less than the number 
			returned by `PluginGetPlayingAnimationCount`. Use `PluginGetAnimationName` 
			to get the name of the animation.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_PLAYING_ANIMATION_ID, GetPlayingAnimationId);
		/*
			Get the RGB color given red, green, and blue.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_RGB, GetRGB);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_GET_RGBD, GetRGBD);
		/*
			Check if the animation has loop enabled referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_HAS_ANIMATION_LOOP, HasAnimationLoop);
		/*
			Check if the animation has loop enabled referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_HAS_ANIMATION_LOOP_NAME, HasAnimationLoopName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_HAS_ANIMATION_LOOP_NAME_D, HasAnimationLoopNameD);
		/*
			Initialize the ChromaSDK. Zero indicates  success, otherwise failure. Many 
			API methods auto initialize the ChromaSDK if not already initialized.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INIT, Init);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INIT_D, InitD);
		/*
			Initialize the ChromaSDK. AppInfo populates the details in Synapse. Zero 
			indicates  success, otherwise failure. Many API methods auto initialize 
			the ChromaSDK if not already initialized.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INIT_SDK, InitSDK);
		/*
			Insert an animation delay by duplicating the frame by the delay number of 
			times. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INSERT_DELAY, InsertDelay);
		/*
			Insert an animation delay by duplicating the frame by the delay number of 
			times. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INSERT_DELAY_NAME, InsertDelayName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INSERT_DELAY_NAME_D, InsertDelayNameD);
		/*
			Duplicate the source frame index at the target frame index. Animation is 
			referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INSERT_FRAME, InsertFrame);
		/*
			Duplicate the source frame index at the target frame index. Animation is 
			referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INSERT_FRAME_NAME, InsertFrameName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INSERT_FRAME_NAME_D, InsertFrameNameD);
		/*
			Invert all the colors at the specified frame. Animation is referenced by 
			id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INVERT_COLORS, InvertColors);
		/*
			Invert all the colors for all frames. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INVERT_COLORS_ALL_FRAMES, InvertColorsAllFrames);
		/*
			Invert all the colors for all frames. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INVERT_COLORS_ALL_FRAMES_NAME, InvertColorsAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INVERT_COLORS_ALL_FRAMES_NAME_D, InvertColorsAllFramesNameD);
		/*
			Invert all the colors at the specified frame. Animation is referenced by 
			name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INVERT_COLORS_NAME, InvertColorsName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_INVERT_COLORS_NAME_D, InvertColorsNameD);
		/*
			Check if the animation is paused referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_ANIMATION_PAUSED, IsAnimationPaused);
		/*
			Check if the animation is paused referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_ANIMATION_PAUSED_NAME, IsAnimationPausedName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_ANIMATION_PAUSED_NAME_D, IsAnimationPausedNameD);
		/*
			The editor dialog is a non-blocking modal window, this method returns true 
			if the modal window is open, otherwise false.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_DIALOG_OPEN, IsDialogOpen);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_DIALOG_OPEN_D, IsDialogOpenD);
		/*
			Returns true if the plugin has been initialized. Returns false if the plugin 
			is uninitialized.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_INITIALIZED, IsInitialized);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_INITIALIZED_D, IsInitializedD);
		/*
			If the method can be invoked the method returns true.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLATFORM_SUPPORTED, IsPlatformSupported);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLATFORM_SUPPORTED_D, IsPlatformSupportedD);
		/*
			`PluginIsPlayingName` automatically handles initializing the `ChromaSDK`. 
			The named `.chroma` animation file will be automatically opened. The method 
			will return whether the animation is playing or not. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLAYING, IsPlaying);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLAYING_D, IsPlayingD);
		/*
			`PluginIsPlayingName` automatically handles initializing the `ChromaSDK`. 
			The named `.chroma` animation file will be automatically opened. The method 
			will return whether the animation is playing or not. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLAYING_NAME, IsPlayingName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLAYING_NAME_D, IsPlayingNameD);
		/*
			`PluginIsPlayingType` automatically handles initializing the `ChromaSDK`. 
			If any animation is playing for the `deviceType` and `device` combination, 
			the method will return true, otherwise false.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLAYING_TYPE, IsPlayingType);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_IS_PLAYING_TYPE_D, IsPlayingTypeD);
		/*
			Do a lerp math operation on a float.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_LERP, Lerp);
		/*
			Lerp from one color to another given t in the range 0.0 to 1.0.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_LERP_COLOR, LerpColor);
		/*
			Loads `Chroma` effects so that the animation can be played immediately. 
			Returns the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_LOAD_ANIMATION, LoadAnimation);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_LOAD_ANIMATION_D, LoadAnimationD);
		/*
			Load the named animation.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_LOAD_ANIMATION_NAME, LoadAnimationName);
		/*
			Load a composite set of animations.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_LOAD_COMPOSITE, LoadComposite);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color defaults to color. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES, MakeBlankFrames);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color defaults to color. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_NAME, MakeBlankFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_NAME_D, MakeBlankFramesNameD);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color is random. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RANDOM, MakeBlankFramesRandom);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color is random black and white. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RANDOM_BLACK_AND_WHITE, MakeBlankFramesRandomBlackAndWhite);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color is random black and white. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RANDOM_BLACK_AND_WHITE_NAME, MakeBlankFramesRandomBlackAndWhiteName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RANDOM_BLACK_AND_WHITE_NAME_D, MakeBlankFramesRandomBlackAndWhiteNameD);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color is random. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RANDOM_NAME, MakeBlankFramesRandomName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RANDOM_NAME_D, MakeBlankFramesRandomNameD);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color defaults to color. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RGB, MakeBlankFramesRGB);
		/*
			Make a blank animation for the length of the frame count. Frame duration 
			defaults to the duration. The frame color defaults to color. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RGB_NAME, MakeBlankFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MAKE_BLANK_FRAMES_RGB_NAME_D, MakeBlankFramesRGBNameD);
		/*
			Flips the color grid horizontally for all `Chroma` animation frames. Returns 
			the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MIRROR_HORIZONTALLY, MirrorHorizontally);
		/*
			Flips the color grid vertically for all `Chroma` animation frames. This 
			method has no effect for `EChromaSDKDevice1DEnum` devices. Returns the 
			animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MIRROR_VERTICALLY, MirrorVertically);
		/*
			Multiply the color intensity with the lerp result from color 1 to color 
			2 using the frame index divided by the frame count for the `t` parameter. 
			Animation is referenced in id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_COLOR_LERP_ALL_FRAMES, MultiplyColorLerpAllFrames);
		/*
			Multiply the color intensity with the lerp result from color 1 to color 
			2 using the frame index divided by the frame count for the `t` parameter. 
			Animation is referenced in name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_COLOR_LERP_ALL_FRAMES_NAME, MultiplyColorLerpAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_COLOR_LERP_ALL_FRAMES_NAME_D, MultiplyColorLerpAllFramesNameD);
		/*
			Multiply all the colors in the frame by the intensity value. The valid the 
			intensity range is from 0.0 to 255.0. RGB components are multiplied equally. 
			An intensity of 0.5 would half the color value. Black colors in the frame 
			will not be affected by this method.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY, MultiplyIntensity);
		/*
			Multiply all the colors for all frames by the intensity value. The valid 
			the intensity range is from 0.0 to 255.0. RGB components are multiplied 
			equally. An intensity of 0.5 would half the color value. Black colors in 
			the frame will not be affected by this method.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES, MultiplyIntensityAllFrames);
		/*
			Multiply all the colors for all frames by the intensity value. The valid 
			the intensity range is from 0.0 to 255.0. RGB components are multiplied 
			equally. An intensity of 0.5 would half the color value. Black colors in 
			the frame will not be affected by this method.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_NAME, MultiplyIntensityAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_NAME_D, MultiplyIntensityAllFramesNameD);
		/*
			Multiply all frames by the RBG color intensity. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_RGB, MultiplyIntensityAllFramesRGB);
		/*
			Multiply all frames by the RBG color intensity. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_RGB_NAME, MultiplyIntensityAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_ALL_FRAMES_RGB_NAME_D, MultiplyIntensityAllFramesRGBNameD);
		/*
			Multiply the specific frame by the RBG color intensity. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_COLOR, MultiplyIntensityColor);
		/*
			Multiply all frames by the RBG color intensity. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_COLOR_ALL_FRAMES, MultiplyIntensityColorAllFrames);
		/*
			Multiply all frames by the RBG color intensity. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_COLOR_ALL_FRAMES_NAME, MultiplyIntensityColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_COLOR_ALL_FRAMES_NAME_D, MultiplyIntensityColorAllFramesNameD);
		/*
			Multiply the specific frame by the RBG color intensity. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_COLOR_NAME, MultiplyIntensityColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_COLOR_NAME_D, MultiplyIntensityColorNameD);
		/*
			Multiply all the colors in the frame by the intensity value. The valid the 
			intensity range is from 0.0 to 255.0. RGB components are multiplied equally. 
			An intensity of 0.5 would half the color value. Black colors in the frame 
			will not be affected by this method.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_NAME, MultiplyIntensityName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_NAME_D, MultiplyIntensityNameD);
		/*
			Multiply the specific frame by the RBG color intensity. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_RGB, MultiplyIntensityRGB);
		/*
			Multiply the specific frame by the RBG color intensity. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_RGB_NAME, MultiplyIntensityRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_INTENSITY_RGB_NAME_D, MultiplyIntensityRGBNameD);
		/*
			Multiply the specific frame by the color lerp result between color 1 and 
			2 using the frame color value as the `t` value. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP, MultiplyNonZeroTargetColorLerp);
		/*
			Multiply all frames by the color lerp result between color 1 and 2 using 
			the frame color value as the `t` value. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES, MultiplyNonZeroTargetColorLerpAllFrames);
		/*
			Multiply all frames by the color lerp result between color 1 and 2 using 
			the frame color value as the `t` value. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_NAME, MultiplyNonZeroTargetColorLerpAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_NAME_D, MultiplyNonZeroTargetColorLerpAllFramesNameD);
		/*
			Multiply the specific frame by the color lerp result between RGB 1 and 2 
			using the frame color value as the `t` value. Animation is referenced by 
			id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_RGB, MultiplyNonZeroTargetColorLerpAllFramesRGB);
		/*
			Multiply the specific frame by the color lerp result between RGB 1 and 2 
			using the frame color value as the `t` value. Animation is referenced by 
			name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME, MultiplyNonZeroTargetColorLerpAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_NON_ZERO_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME_D, MultiplyNonZeroTargetColorLerpAllFramesRGBNameD);
		/*
			Multiply the specific frame by the color lerp result between color 1 and 
			2 using the frame color value as the `t` value. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP, MultiplyTargetColorLerp);
		/*
			Multiply all frames by the color lerp result between color 1 and 2 using 
			the frame color value as the `t` value. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES, MultiplyTargetColorLerpAllFrames);
		/*
			Multiply all frames by the color lerp result between color 1 and 2 using 
			the frame color value as the `t` value. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_NAME, MultiplyTargetColorLerpAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_NAME_D, MultiplyTargetColorLerpAllFramesNameD);
		/*
			Multiply all frames by the color lerp result between RGB 1 and 2 using the 
			frame color value as the `t` value. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_RGB, MultiplyTargetColorLerpAllFramesRGB);
		/*
			Multiply all frames by the color lerp result between RGB 1 and 2 using the 
			frame color value as the `t` value. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME, MultiplyTargetColorLerpAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_MULTIPLY_TARGET_COLOR_LERP_ALL_FRAMES_RGB_NAME_D, MultiplyTargetColorLerpAllFramesRGBNameD);
		/*
			Offset all colors in the frame using the RGB offset. Use the range of -255 
			to 255 for red, green, and blue parameters. Negative values remove color. 
			Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_COLORS, OffsetColors);
		/*
			Offset all colors for all frames using the RGB offset. Use the range of 
			-255 to 255 for red, green, and blue parameters. Negative values remove 
			color. Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_COLORS_ALL_FRAMES, OffsetColorsAllFrames);
		/*
			Offset all colors for all frames using the RGB offset. Use the range of 
			-255 to 255 for red, green, and blue parameters. Negative values remove 
			color. Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_COLORS_ALL_FRAMES_NAME, OffsetColorsAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_COLORS_ALL_FRAMES_NAME_D, OffsetColorsAllFramesNameD);
		/*
			Offset all colors in the frame using the RGB offset. Use the range of -255 
			to 255 for red, green, and blue parameters. Negative values remove color. 
			Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_COLORS_NAME, OffsetColorsName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_COLORS_NAME_D, OffsetColorsNameD);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Offset a subset of colors in the frame using the RGB offset. 
			Use the range of -255 to 255 for red, green, and blue parameters. Negative 
			values remove color. Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_NON_ZERO_COLORS, OffsetNonZeroColors);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Offset a subset of colors for all frames using the RGB offset. 
			Use the range of -255 to 255 for red, green, and blue parameters. Negative 
			values remove color. Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_NON_ZERO_COLORS_ALL_FRAMES, OffsetNonZeroColorsAllFrames);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Offset a subset of colors for all frames using the RGB offset. 
			Use the range of -255 to 255 for red, green, and blue parameters. Negative 
			values remove color. Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_NON_ZERO_COLORS_ALL_FRAMES_NAME, OffsetNonZeroColorsAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_NON_ZERO_COLORS_ALL_FRAMES_NAME_D, OffsetNonZeroColorsAllFramesNameD);
		/*
			This method will only update colors in the animation that are not already 
			set to black. Offset a subset of colors in the frame using the RGB offset. 
			Use the range of -255 to 255 for red, green, and blue parameters. Negative 
			values remove color. Positive values add color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_NON_ZERO_COLORS_NAME, OffsetNonZeroColorsName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OFFSET_NON_ZERO_COLORS_NAME_D, OffsetNonZeroColorsNameD);
		/*
			Opens a `Chroma` animation file so that it can be played. Returns an animation 
			id >= 0 upon success. Returns -1 if there was a failure. The animation 
			id is used in most of the API methods.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_ANIMATION, OpenAnimation);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_ANIMATION_D, OpenAnimationD);
		/*
			Opens a `Chroma` animation data from memory so that it can be played. `Data` 
			is a pointer to byte array of the loaded animation in memory. `Name` will 
			be assigned to the animation when loaded. Returns an animation id >= 0 
			upon success. Returns -1 if there was a failure. The animation id is used 
			in most of the API methods.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_ANIMATION_FROM_MEMORY, OpenAnimationFromMemory);
		/*
			Opens a `Chroma` animation file with the `.chroma` extension. Returns zero 
			upon success. Returns -1 if there was a failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_EDITOR_DIALOG, OpenEditorDialog);
		/*
			Open the named animation in the editor dialog and play the animation at 
			start.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_EDITOR_DIALOG_AND_PLAY, OpenEditorDialogAndPlay);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_EDITOR_DIALOG_AND_PLAY_D, OpenEditorDialogAndPlayD);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OPEN_EDITOR_DIALOG_D, OpenEditorDialogD);
		/*
			Sets the `duration` for all grames in the `Chroma` animation to the `duration` 
			parameter. Returns the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OVERRIDE_FRAME_DURATION, OverrideFrameDuration);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OVERRIDE_FRAME_DURATION_D, OverrideFrameDurationD);
		/*
			Override the duration of all frames with the `duration` value. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_OVERRIDE_FRAME_DURATION_NAME, OverrideFrameDurationName);
		/*
			Pause the current animation referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PAUSE_ANIMATION, PauseAnimation);
		/*
			Pause the current animation referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PAUSE_ANIMATION_NAME, PauseAnimationName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PAUSE_ANIMATION_NAME_D, PauseAnimationNameD);
		/*
			Plays the `Chroma` animation. This will load the animation, if not loaded 
			previously. Returns the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION, PlayAnimation);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_D, PlayAnimationD);
		/*
			`PluginPlayAnimationFrame` automatically handles initializing the `ChromaSDK`. 
			The method will play the animation given the `animationId` with looping 
			`on` or `off` starting at the `frameId`.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_FRAME, PlayAnimationFrame);
		/*
			`PluginPlayAnimationFrameName` automatically handles initializing the `ChromaSDK`. 
			The named `.chroma` animation file will be automatically opened. The animation 
			will play with looping `on` or `off` starting at the `frameId`.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_FRAME_NAME, PlayAnimationFrameName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_FRAME_NAME_D, PlayAnimationFrameNameD);
		/*
			`PluginPlayAnimationLoop` automatically handles initializing the `ChromaSDK`. 
			The method will play the animation given the `animationId` with looping 
			`on` or `off`.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_LOOP, PlayAnimationLoop);
		/*
			`PluginPlayAnimationName` automatically handles initializing the `ChromaSDK`. 
			The named `.chroma` animation file will be automatically opened. The animation 
			will play with looping `on` or `off`.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_NAME, PlayAnimationName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_ANIMATION_NAME_D, PlayAnimationNameD);
		/*
			`PluginPlayComposite` automatically handles initializing the `ChromaSDK`. 
			The named animation files for the `.chroma` set will be automatically opened. 
			The set of animations will play with looping `on` or `off`.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_COMPOSITE, PlayComposite);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PLAY_COMPOSITE_D, PlayCompositeD);
		/*
			Displays the `Chroma` animation frame on `Chroma` hardware given the `frameIndex`. 
			Returns the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PREVIEW_FRAME, PreviewFrame);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PREVIEW_FRAME_D, PreviewFrameD);
		/*
			Displays the `Chroma` animation frame on `Chroma` hardware given the `frameIndex`. 
			Animaton is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_PREVIEW_FRAME_NAME, PreviewFrameName);
		/*
			Reduce the frames of the animation by removing every nth element. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REDUCE_FRAMES, ReduceFrames);
		/*
			Reduce the frames of the animation by removing every nth element. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REDUCE_FRAMES_NAME, ReduceFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REDUCE_FRAMES_NAME_D, ReduceFramesNameD);
		/*
			Resets the `Chroma` animation to 1 blank frame. Returns the animation id 
			upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_RESET_ANIMATION, ResetAnimation);
		/*
			Resume the animation with loop `ON` or `OFF` referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_RESUME_ANIMATION, ResumeAnimation);
		/*
			Resume the animation with loop `ON` or `OFF` referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_RESUME_ANIMATION_NAME, ResumeAnimationName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_RESUME_ANIMATION_NAME_D, ResumeAnimationNameD);
		/*
			Reverse the animation frame order of the `Chroma` animation. Returns the 
			animation id upon success. Returns -1 upon failure. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REVERSE, Reverse);
		/*
			Reverse the animation frame order of the `Chroma` animation. Animation is 
			referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REVERSE_ALL_FRAMES, ReverseAllFrames);
		/*
			Reverse the animation frame order of the `Chroma` animation. Animation is 
			referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REVERSE_ALL_FRAMES_NAME, ReverseAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_REVERSE_ALL_FRAMES_NAME_D, ReverseAllFramesNameD);
		/*
			Save the animation referenced by id to the path specified.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SAVE_ANIMATION, SaveAnimation);
		/*
			Save the named animation to the target path specified.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SAVE_ANIMATION_NAME, SaveAnimationName);
		/*
			Set the animation color for a frame given the `1D` `led`. The `led` should 
			be greater than or equal to 0 and less than the `MaxLeds`. The animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_1D_COLOR, Set1DColor);
		/*
			Set the animation color for a frame given the `1D` `led`. The `led` should 
			be greater than or equal to 0 and less than the `MaxLeds`. The animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_1D_COLOR_NAME, Set1DColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_1D_COLOR_NAME_D, Set1DColorNameD);
		/*
			Set the animation color for a frame given the `2D` `row` and `column`. The 
			`row` should be greater than or equal to 0 and less than the `MaxRow`. 
			The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
			The animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_2D_COLOR, Set2DColor);
		/*
			Set the animation color for a frame given the `2D` `row` and `column`. The 
			`row` should be greater than or equal to 0 and less than the `MaxRow`. 
			The `column` should be greater than or equal to 0 and less than the `MaxColumn`. 
			The animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_2D_COLOR_NAME, Set2DColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_2D_COLOR_NAME_D, Set2DColorNameD);
		/*
			When custom color is set, the custom key mode will be used. The animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CHROMA_CUSTOM_COLOR_ALL_FRAMES, SetChromaCustomColorAllFrames);
		/*
			When custom color is set, the custom key mode will be used. The animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CHROMA_CUSTOM_COLOR_ALL_FRAMES_NAME, SetChromaCustomColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CHROMA_CUSTOM_COLOR_ALL_FRAMES_NAME_D, SetChromaCustomColorAllFramesNameD);
		/*
			Set the Chroma custom key color flag on all frames. `True` changes the layout 
			from grid to key. `True` changes the layout from key to grid. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CHROMA_CUSTOM_FLAG, SetChromaCustomFlag);
		/*
			Set the Chroma custom key color flag on all frames. `True` changes the layout 
			from grid to key. `True` changes the layout from key to grid. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CHROMA_CUSTOM_FLAG_NAME, SetChromaCustomFlagName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CHROMA_CUSTOM_FLAG_NAME_D, SetChromaCustomFlagNameD);
		/*
			Set the current frame of the animation referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CURRENT_FRAME, SetCurrentFrame);
		/*
			Set the current frame of the animation referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CURRENT_FRAME_NAME, SetCurrentFrameName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_CURRENT_FRAME_NAME_D, SetCurrentFrameNameD);
		/*
			Changes the `deviceType` and `device` of a `Chroma` animation. If the device 
			is changed, the `Chroma` animation will be reset with 1 blank frame. Returns 
			the animation id upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_DEVICE, SetDevice);
		/*
			SetEffect will display the referenced effect id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_EFFECT, SetEffect);
		/*
			When the idle animation is used, the named animation will play when no other 
			animations are playing. Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_IDLE_ANIMATION, SetIdleAnimation);
		/*
			When the idle animation is used, the named animation will play when no other 
			animations are playing. Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_IDLE_ANIMATION_NAME, SetIdleAnimationName);
		/*
			Set animation key to a static color for the given frame.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR, SetKeyColor);
		/*
			Set the key to the specified key color for all frames. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_ALL_FRAMES, SetKeyColorAllFrames);
		/*
			Set the key to the specified key color for all frames. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_ALL_FRAMES_NAME, SetKeyColorAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_ALL_FRAMES_NAME_D, SetKeyColorAllFramesNameD);
		/*
			Set the key to the specified key color for all frames. Animation is referenced 
			by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_ALL_FRAMES_RGB, SetKeyColorAllFramesRGB);
		/*
			Set the key to the specified key color for all frames. Animation is referenced 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_ALL_FRAMES_RGB_NAME, SetKeyColorAllFramesRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_ALL_FRAMES_RGB_NAME_D, SetKeyColorAllFramesRGBNameD);
		/*
			Set animation key to a static color for the given frame.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_NAME, SetKeyColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_NAME_D, SetKeyColorNameD);
		/*
			Set the key to the specified key color for the specified frame. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_RGB, SetKeyColorRGB);
		/*
			Set the key to the specified key color for the specified frame. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_RGB_NAME, SetKeyColorRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_COLOR_RGB_NAME_D, SetKeyColorRGBNameD);
		/*
			Set animation key to a static color for the given frame if the existing 
			color is not already black.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_NON_ZERO_COLOR, SetKeyNonZeroColor);
		/*
			Set animation key to a static color for the given frame if the existing 
			color is not already black.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_NON_ZERO_COLOR_NAME, SetKeyNonZeroColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_NON_ZERO_COLOR_NAME_D, SetKeyNonZeroColorNameD);
		/*
			Set the key to the specified key color for the specified frame where color 
			is not black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_NON_ZERO_COLOR_RGB, SetKeyNonZeroColorRGB);
		/*
			Set the key to the specified key color for the specified frame where color 
			is not black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_NON_ZERO_COLOR_RGB_NAME, SetKeyNonZeroColorRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_NON_ZERO_COLOR_RGB_NAME_D, SetKeyNonZeroColorRGBNameD);
		/*
			Set an array of animation keys to a static color for the given frame. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR, SetKeysColor);
		/*
			Set an array of animation keys to a static color for all frames. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_ALL_FRAMES, SetKeysColorAllFrames);
		/*
			Set an array of animation keys to a static color for all frames. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_ALL_FRAMES_NAME, SetKeysColorAllFramesName);
		/*
			Set an array of animation keys to a static color for all frames. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_ALL_FRAMES_RGB, SetKeysColorAllFramesRGB);
		/*
			Set an array of animation keys to a static color for all frames. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_ALL_FRAMES_RGB_NAME, SetKeysColorAllFramesRGBName);
		/*
			Set an array of animation keys to a static color for the given frame.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_NAME, SetKeysColorName);
		/*
			Set an array of animation keys to a static color for the given frame. Animation 
			is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_RGB, SetKeysColorRGB);
		/*
			Set an array of animation keys to a static color for the given frame. Animation 
			is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_COLOR_RGB_NAME, SetKeysColorRGBName);
		/*
			Set an array of animation keys to a static color for the given frame if 
			the existing color is not already black.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_NON_ZERO_COLOR, SetKeysNonZeroColor);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is not black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_NON_ZERO_COLOR_ALL_FRAMES, SetKeysNonZeroColorAllFrames);
		/*
			Set an array of animation keys to a static color for all frames if the existing 
			color is not already black. Reference animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_NON_ZERO_COLOR_ALL_FRAMES_NAME, SetKeysNonZeroColorAllFramesName);
		/*
			Set an array of animation keys to a static color for the given frame if 
			the existing color is not already black. Reference animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_NON_ZERO_COLOR_NAME, SetKeysNonZeroColorName);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is not black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_NON_ZERO_COLOR_RGB, SetKeysNonZeroColorRGB);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is not black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_NON_ZERO_COLOR_RGB_NAME, SetKeysNonZeroColorRGBName);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR, SetKeysZeroColor);
		/*
			Set an array of animation keys to a static color for all frames where the 
			color is black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES, SetKeysZeroColorAllFrames);
		/*
			Set an array of animation keys to a static color for all frames where the 
			color is black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES_NAME, SetKeysZeroColorAllFramesName);
		/*
			Set an array of animation keys to a static color for all frames where the 
			color is black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES_RGB, SetKeysZeroColorAllFramesRGB);
		/*
			Set an array of animation keys to a static color for all frames where the 
			color is black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_ALL_FRAMES_RGB_NAME, SetKeysZeroColorAllFramesRGBName);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_NAME, SetKeysZeroColorName);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_RGB, SetKeysZeroColorRGB);
		/*
			Set an array of animation keys to a static color for the given frame where 
			the color is black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEYS_ZERO_COLOR_RGB_NAME, SetKeysZeroColorRGBName);
		/*
			Set animation key to a static color for the given frame where the color 
			is black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_ZERO_COLOR, SetKeyZeroColor);
		/*
			Set animation key to a static color for the given frame where the color 
			is black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_ZERO_COLOR_NAME, SetKeyZeroColorName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_ZERO_COLOR_NAME_D, SetKeyZeroColorNameD);
		/*
			Set animation key to a static color for the given frame where the color 
			is black. Animation is referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_ZERO_COLOR_RGB, SetKeyZeroColorRGB);
		/*
			Set animation key to a static color for the given frame where the color 
			is black. Animation is referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_ZERO_COLOR_RGB_NAME, SetKeyZeroColorRGBName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_KEY_ZERO_COLOR_RGB_NAME_D, SetKeyZeroColorRGBNameD);
		/*
			Invokes the setup for a debug logging callback so that `stdout` is redirected 
			to the callback. This is used by `Unity` so that debug messages can appear 
			in the console window.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SET_LOG_DELEGATE, SetLogDelegate);
		/*
			`PluginStaticColor` sets the target device to the static color.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STATIC_COLOR, StaticColor);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STATIC_COLOR_D, StaticColorD);
		/*
			`PluginStopAll` will automatically stop all animations that are playing.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ALL, StopAll);
		/*
			Stops animation playback if in progress. Returns the animation id upon success. 
			Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ANIMATION, StopAnimation);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ANIMATION_D, StopAnimationD);
		/*
			`PluginStopAnimationName` automatically handles initializing the `ChromaSDK`. 
			The named `.chroma` animation file will be automatically opened. The animation 
			will stop if playing.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ANIMATION_NAME, StopAnimationName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ANIMATION_NAME_D, StopAnimationNameD);
		/*
			`PluginStopAnimationType` automatically handles initializing the `ChromaSDK`. 
			If any animation is playing for the `deviceType` and `device` combination, 
			it will be stopped.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ANIMATION_TYPE, StopAnimationType);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_ANIMATION_TYPE_D, StopAnimationTypeD);
		/*
			`PluginStopComposite` automatically handles initializing the `ChromaSDK`. 
			The named animation files for the `.chroma` set will be automatically opened. 
			The set of animations will be stopped if playing.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_COMPOSITE, StopComposite);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_STOP_COMPOSITE_D, StopCompositeD);
		/*
			Subtract the source color from the target color for all frames where the 
			target color is not black. Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES, SubtractNonZeroAllKeysAllFrames);
		/*
			Subtract the source color from the target color for all frames where the 
			target color is not black. Source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME, SubtractNonZeroAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_NAME_D, SubtractNonZeroAllKeysAllFramesNameD);
		/*
			Subtract the source color from the target color for all frames where the 
			target color is not black starting at offset for the length of the source. 
			Source and target are referenced by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET, SubtractNonZeroAllKeysAllFramesOffset);
		/*
			Subtract the source color from the target color for all frames where the 
			target color is not black starting at offset for the length of the source. 
			Source and target are referenced by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, SubtractNonZeroAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, SubtractNonZeroAllKeysAllFramesOffsetNameD);
		/*
			Subtract the source color from the target where color is not black for the 
			source frame and target offset frame, reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_OFFSET, SubtractNonZeroAllKeysOffset);
		/*
			Subtract the source color from the target where color is not black for the 
			source frame and target offset frame, reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_OFFSET_NAME, SubtractNonZeroAllKeysOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_ALL_KEYS_OFFSET_NAME_D, SubtractNonZeroAllKeysOffsetNameD);
		/*
			Subtract the source color from the target color where the target color is 
			not black for all frames. Reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES, SubtractNonZeroTargetAllKeysAllFrames);
		/*
			Subtract the source color from the target color where the target color is 
			not black for all frames. Reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME, SubtractNonZeroTargetAllKeysAllFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_NAME_D, SubtractNonZeroTargetAllKeysAllFramesNameD);
		/*
			Subtract the source color from the target color where the target color is 
			not black for all frames starting at the target offset for the length of 
			the source. Reference source and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET, SubtractNonZeroTargetAllKeysAllFramesOffset);
		/*
			Subtract the source color from the target color where the target color is 
			not black for all frames starting at the target offset for the length of 
			the source. Reference source and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME, SubtractNonZeroTargetAllKeysAllFramesOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_ALL_FRAMES_OFFSET_NAME_D, SubtractNonZeroTargetAllKeysAllFramesOffsetNameD);
		/*
			Subtract the source color from the target color where the target color is 
			not black from the source frame to the target offset frame. Reference source 
			and target by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_OFFSET, SubtractNonZeroTargetAllKeysOffset);
		/*
			Subtract the source color from the target color where the target color is 
			not black from the source frame to the target offset frame. Reference source 
			and target by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME, SubtractNonZeroTargetAllKeysOffsetName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_SUBTRACT_NON_ZERO_TARGET_ALL_KEYS_OFFSET_NAME_D, SubtractNonZeroTargetAllKeysOffsetNameD);
		/*
			Trim the end of the animation. The length of the animation will be the lastFrameId 
			+ 1. Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_END_FRAMES, TrimEndFrames);
		/*
			Trim the end of the animation. The length of the animation will be the lastFrameId 
			+ 1. Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_END_FRAMES_NAME, TrimEndFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_END_FRAMES_NAME_D, TrimEndFramesNameD);
		/*
			Remove the frame from the animation. Reference animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_FRAME, TrimFrame);
		/*
			Remove the frame from the animation. Reference animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_FRAME_NAME, TrimFrameName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_FRAME_NAME_D, TrimFrameNameD);
		/*
			Trim the start of the animation starting at frame 0 for the number of frames. 
			Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_START_FRAMES, TrimStartFrames);
		/*
			Trim the start of the animation starting at frame 0 for the number of frames. 
			Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_START_FRAMES_NAME, TrimStartFramesName);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_TRIM_START_FRAMES_NAME_D, TrimStartFramesNameD);
		/*
			Uninitializes the `ChromaSDK`. Returns 0 upon success. Returns -1 upon failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UNINIT, Uninit);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UNINIT_D, UninitD);
		/*
			Unloads `Chroma` effects to free up resources. Returns the animation id 
			upon success. Returns -1 upon failure. Reference the animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UNLOAD_ANIMATION, UnloadAnimation);
		/*
			D suffix for limited data types.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UNLOAD_ANIMATION_D, UnloadAnimationD);
		/*
			Unload the animation effects. Reference the animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UNLOAD_ANIMATION_NAME, UnloadAnimationName);
		/*
			Unload the the composite set of animation effects. Reference the animation 
			by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UNLOAD_COMPOSITE, UnloadComposite);
		/*
			Updates the `frameIndex` of the `Chroma` animation and sets the `duration` 
			(in seconds). The `color` is expected to be an array of the dimensions 
			for the `deviceType/device`. The `length` parameter is the size of the 
			`color` array. For `EChromaSDKDevice1DEnum` the array size should be `MAX 
			LEDS`. For `EChromaSDKDevice2DEnum` the array size should be `MAX ROW` 
			* `MAX COLUMN`. Returns the animation id upon success. Returns -1 upon 
			failure.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_UPDATE_FRAME, UpdateFrame);
		/*
			When the idle animation flag is true, when no other animations are playing, 
			the idle animation will be used. The idle animation will not be affected 
			by the API calls to PluginIsPlaying, PluginStopAnimationType, PluginGetPlayingAnimationId, 
			and PluginGetPlayingAnimationCount. Then the idle animation flag is false, 
			the idle animation is disabled. `Device` uses `EChromaSDKDeviceEnum` enums.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_USE_IDLE_ANIMATION, UseIdleAnimation);
		/*
			Set idle animation flag for all devices.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_USE_IDLE_ANIMATIONS, UseIdleAnimations);
		/*
			Set preloading animation flag, which is set to true by default. Reference 
			animation by id.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_USE_PRELOADING, UsePreloading);
		/*
			Set preloading animation flag, which is set to true by default. Reference 
			animation by name.
		*/
		CHROMASDK_DECLARE_METHOD(PLUGIN_USE_PRELOADING_NAME, UsePreloadingName);
#pragma endregion

		static int InitAPI();
		static bool IsInitializedAPI();
	};
}
