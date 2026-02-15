### Edif to DarkEdif conversion ###

1. Copy and open copy of DarkEdif Template project file
2. Remove any unneeded files (e.g. Actions if there's no actions)
4. Rename Ext.json to DarkExt.json
3. Move Identifier in JSON, move ACE and menus into "US English", or "UK English"
5. Move your properties over.
 * You can insert them into JSON and #define NOPROPS in project properties, then manually manage GetPropValue, SetPropValue etc (as you used to in older SDKs), or you can use the automatic memory management and an upgrade system for existing MFAs. You will need some sort of update to EDITDATA::eHeader::extVersion in CreateObject, so new objects get a higher version number to indicate the EDITDATA is DarkEdif structure.
 * Note that extVersion is only set by your object; currently, the SDK doesn't set it. If it differs between the MFX the MFA was saved with, and the MFX in the Extensions folder, Fusion will run UpdateEditStructure() if thatt's defined in your project. Usually, it'll be in General.cpp.
 * Use of NOPROPS means DarkEdif will still provide a list of properties to Fusion from the JSON; however, it will not save or read any of them from EDITDATA itself, so you'll have to implement that part in GetPropValue and related functions. There's examples of this in Bluewing Client and Server, as their EDITDATAs are set to replicate the Relay counterpart's memory layout.


### Notes about DarkEdif properties ###

In the current DarkEdif system, which is a static uneditable block of memory, changes to properties will result in existing EDITDATAs becoming invalid, and only a recreation of the object will result in correct EDITDATA size.
This is consistent with older SDKs, so it's not exactly a regression; in older SDKs, you'd have to write your own upgrade function via UpdateEditStructure. The Tab Control object has an example of this.

This will change when the DarkEdif smart property update is introduced; you'll be able to define your own upgrade function from manual memory, but you won't need to worry about rearranging properties, adding new ones, deleting them, etc.
The smart property system will automatically detect changes to JSON, and:

1. Add the defaults from JSON for new properties into EDITDATA;
2. Rearrange EDITDATA memory for moved properties; and
3. Drop properties no longer present in JSON.
4. If you rename properties and want the smart property system to move the old value to new property name instead of discarding it, you can add an "old name" variable to the renamed property in JSON.
