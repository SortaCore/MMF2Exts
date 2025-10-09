#pragma author Phi
#pragma description EDITDATA DarkEdif smart properties

import std.math;
import std.sys;
import std.string;

struct Data<auto Ver>
{
    // Size of property's data relative from start pos in dataForProps.
    u32 sizeBytes; // for jumping past in scrolling; includes this entire Data
    if (sizeBytes > 5000)
        std::print("sizeBytes invalid: {}", sizeBytes);
    else
    {
        // Property type ID - not Unicode incremented!
        u16 propTypeID;
        if (Ver == 2)
        {
            // New in prop v2: this is necessary if this property is part of a repeating set, and
            // its index in EDITDATA::Props does not match the JSON property index
            u16 propJSONIndex;
        }

        // Data includes:
        // 1) Prop name as retrieved from JSON, but in lowercase for faster string compare.
        // 2) Actual data for property (prop value)
        u8 propNameSize;
        char name[propNameSize];
        // Edit number, color, edit slider, edit spin,
        if (propTypeID == 6 || propTypeID == 9 || propTypeID == 11 || propTypeID == 12)
            s32 intValue;
        // Edit float, edit spin float
        else if (propTypeID == 21 || propTypeID == 27)
            float floatValue;
        // Edit string, filename, picture filename, editbox multiline, directory name
        else if (propTypeID == 5 || propTypeID == 16 || propTypeID == 19 || propTypeID == 22 || propTypeID == 26)
            char stringVal[sizeBytes - propNameSize - 9];
        // Combo box, combo box button, combo box icon
        else if (propTypeID == 7 || propTypeID == 20 || propTypeID == 24)
        {
            char setIndicator;
            if (setIndicator == 'S')
            {
                u16 numRepeats;
                u16 lastSetJSONPropIndex;
                u16 firstSetJSONPropIndex;
                u16 setNameJSONPropIndex;
                u16 setIndexSelected;
                char setName[sizeBytes - propNameSize - 20];
            }
            else if (setIndicator == 'L')
                char stringVal[sizeBytes - propNameSize - 10];
            else
                std::print("combo box setIndicator invalid: {}", setIndicator);
        }
        // size
        else if (propTypeID == 8)
            s32 width, height;
        // image list
        else if (propTypeID == 23)
        {
            u16 numImages;
            u16 imageIDs[numImages];
        }
        // direction control, group, folder start, folder end, button
        else
            u8 propData[sizeBytes - propNameSize - 9];
    }
};

bitfield CheckBoxes {
    chk0 : 1; chk1 : 1; chk2 : 1; chk3 : 1;
    chk4 : 1; chk5 : 1; chk6 : 1; chk7 : 1;
};
struct Properties
{
    // Version of this DarkEdif::Properties struct - reversed due to little endian
    char propVersion[4];

    // fnv1a hashes, used to read JSON properties to see if a property layout change has been made.
    u32 hash; // property titles and types
    u32 hashTypes; // property types only
    // Number of properties
    u16 numProps;

    // VS decided to pad this struct
    u16 pad;
    // Size of DataForProps - including EDITDATA (and thus EDITDATA::Properties)
    // Note that this is uint32, because initial EDITDATA is capped to uint16 by GetRunObjectInfos()'s EDITDATASize,
    // but the size after initial setup is in EDITDATA::eHeader::extSize, which is uint32.
    u32 sizeBytes;

    if (std::string::reverse(propVersion) == "DAR2")
    {
        u32 edVisiblePtr;
        u32 ptrPad;
    }

    // The actual data for properties, merged together
    // Starts with checkboxes, then data, which is Data struct: type ID followed by binary.
    CheckBoxes chkBoxes[std::math::ceil(numProps / 8.f)];
    if (std::string::reverse(propVersion) == "DAR2")
        Data<2> props[numProps];
    else if (std::string::reverse(propVersion) == "DAR1")
        Data<1> props[numProps];
    else
        std::print("Prop version \"{}\" is invalid", std::string::reverse(propVersion));
};

// EDITDATA::eHeader::privateData stores offset to EDITDATA::Props
u32 eHeaderPrivData @ 16;
Properties Props @ eHeaderPrivData;