/*
	This DarkScript Fusion extension UWP port is under MIT license.
*/

/* global console, darkEdif, alert, CRunExtension, CServices, window */
/* jslint esversion: 6, sub: true */

// This is strict, but that can be assumed
// "use strict";

/**
 * String.prototype.replaceAll() polyfill
 * https://gomakethings.com/how-to-replace-a-section-of-a-string-with-another-one-with-vanilla-js/
 * @author Chris Ferdinandi
 * @license MIT
 */
if (!String.prototype.replaceAll) {
	String.prototype.replaceAll = function(str, newStr) {
		// If a regex pattern
		if (Object.prototype.toString.call(str).toLowerCase() === '[object regexp]') {
			return this.replace(str, newStr);
		}

		// If a string
		return this.replace(new RegExp(str, 'g'), newStr);
	};
}

// Global data, including sub-applications, just how God intended.
// Note: This will allow newer SDK versions in later SDKs to take over.
// We need this[] and window[] instead of direct, because HTML5 Final Project minifies and breaks the names otherwise
// In case UWP is minified later, we'll keep these HTML5 minify precautions in UWP
window['darkEdif'] = (window['darkEdif'] && window['darkEdif'].sdkVersion >= 19) ? window['darkEdif'] :
	new (/** @constructor */ function() {
	this.data = {};
	this.getGlobalData = function (key) {
		key = key.toLowerCase();
		if (key in this.data) {
			return this.data[key];
		}
		return null;
	};
	this.setGlobalData = function (key, value) {
		key = key.toLowerCase();
		this.data[key] = value;
	};

	this.getCurrentFusionEventNumber = function (ext) {
		return ext.rh.rhEvtProg.rhEventGroup.evgLine || -1;
	};
	this.sdkVersion = 19;
	this.checkSupportsSDKVersion = function (sdkVer) {
		if (sdkVer < 16 || sdkVer > 19) {
			throw "UWP DarkEdif SDK does not support SDK version " + this.sdkVersion;
		}
	};

	// minifier will rename notMinified, so we can detect minifier simply
	this.minified = false;
	if (!this.hasOwnProperty('minified')) {
		this['minified'] = true;
	}

	this.consoleLog = function (ext, str) {
		// Log if DebugMode not defined, or true
		if (ext == null || ext['DebugMode'] == null || ext['DebugMode']) {
			// Exts will be renamed in minified
			if (this['minified'] && ext != null && ext['DebugMode'] == true) {
				console.warn("DebugMode left true for an extension in minified mode. Did the ext developer not set it false before distributing?");
				ext['DebugMode'] = false;
			}

			const extName = (ext == null || this['minified']) ? "Unknown DarkEdif ext" :
				ext['ExtensionName'] || ext.constructor.name.replaceAll('CRun', '').replaceAll('_',' ');
			console.log(extName + " - " + str);
		}
	};
	// UWP does not have FinalizationRegistry
	this.finalizer = { register: function(desc) { } };

	this['Properties'] = function(ext, edPtrFile, extVersion) {
		// DarkEdif SDK stores offset of DarkEdif props away from start of EDITDATA inside private data.
		// eHeader is 20 bytes, so this should be 20+ bytes.
		if (ext.ho.privateData < 20) {
			throw "Not smart properties - eHeader missing?";
		}
		// DarkEdif SDK header read:
		// header uint32, hash uint32, hashtypes uint32, numprops uint16, pad uint16, sizeBytes uint32 (includes whole EDITDATA)
		// then checkbox list, one bit per checkbox, including non-checkbox properties
		// so skip numProps / 8 bytes
		// then moving to Data list:
		// size uint32 (includes whole Data), propType uint16, propNameSize uint8, propname u8 (lowercased), then data bytes

		let header = new Uint8Array(edPtrFile.readBuffer(4 + 4 + 4 + 2 + 2 + 4));
		if (String.fromCharCode.apply('', [header[3], header[2], header[1], header[0]]) != 'DAR1') {
			throw "Did you read this.ho.privateData bytes?";
		}

		let headerDV = new DataView(header.buffer);
		this.numProps = headerDV.getUint16(4 + 4 + 4, true); // Skip past hash and hashTypes
		this.sizeBytes = headerDV.getUint32(4 + 4 + 4 + 4, true); // skip past numProps and pad

		let editData = edPtrFile.readBuffer(this.sizeBytes - header.byteLength);
		this.chkboxes = editData.slice(0, Math.ceil(this.numProps / 8));
		let that = this;
		let GetPropertyIndex = function(chkIDOrName) {
			if (typeof chkIDOrName == 'number') {
				if (that.numProps <= chkIDOrName) {
					throw "Invalid property ID " + chkIDOrName + ", max ID is " + (that.numProps - 1) + ".";
				}
				return chkIDOrName;
			}
			const p = that.props.find(function(p) { return p.propName == chkIDOrName; });
			if (p == null) {
				throw "Invalid property name \"" + chkIDOrName + "\"";
			}
			return p.index;
		};
		this['IsPropChecked'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return 0;
			}
			return (that.chkboxes[Math.floor(idx / 8)] & (1 << idx % 8)) != 0;
		};
		this['GetPropertyStr'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return "";
			}
			const prop = that.props[idx];
			const textPropIDs = [
				5, // PROPTYPE_EDIT_STRING:
				22, // PROPTYPE_EDIT_MULTILINE:
				16, // PROPTYPE_FILENAME:
				19, // PROPTYPE_PICTUREFILENAME:
				26, // PROPTYPE_DIRECTORYNAME:
				7, // PROPTYPE_COMBOBOX:
				20, // PROPTYPE_COMBOBOXBTN:
				24 // PROPTYPE_ICONCOMBOBOX:
			];
			if (textPropIDs.indexOf(prop.propTypeID) != -1) {
				let t = that.textDecoder.decode(prop.propData);
				if (prop.propTypeID == 22) //PROPTYPE_EDIT_MULTILINE
					t = t.replaceAll('\r', ''); // CRLF to LF
				return t;
			}
			throw "Property " + prop.propName + " is not textual.";
		};
		this['GetPropertyNum'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return 0.0;
			}
			const prop = that.props[idx];
			const numPropIDsInteger = [
				6, // PROPTYPE_EDIT_NUMBER
				9, // PROPTYPE_COLOR
				11, // PROPTYPE_SLIDEREDIT
				12, // PROPTYPE_SPINEDIT
				13 // PROPTYPE_DIRCTRL
			];
			const numPropIDsFloat = [
				21, // PROPTYPE_EDIT_FLOAT
				27 // PROPTYPE_SPINEDITFLOAT
			];
			if (numPropIDsInteger.indexOf(prop.propTypeID) != -1) {
				return new DataView(prop.propData.buffer).getUint32(0, true);
			}
			if (numPropIDsFloat.indexOf(prop.propTypeID) != -1) {
				return new DataView(prop.propData.buffer).getFloat32(0, true);
			}
			throw "Property " + prop.propName + " is not numeric.";
		};

		this.props = [];
		const data = editData.slice(this.chkboxes.length);
		const dataDV = new DataView(new Uint8Array(data).buffer);

		this.textDecoder = null;
		if (window['TextDecoder'] != null) {
			this.textDecoder = new window['TextDecoder']();
		}
		else {
			// one byte = one char - should suffice for basic ASCII property names
			this.textDecoder = {
				decode: function(txt) {
					return String.fromCharCode.apply("", txt);
				}
			};
		}

		for (let i = 0, pt = 0, propSize, propEnd; i < this.numProps; ++i) {
			propSize = dataDV.getUint32(pt, true);
			propEnd = pt + propSize;
			const propTypeID = dataDV.getUint16(pt + 4, true);
			const propNameLength = dataDV.getUint8(pt + 4 + 2);
			pt += 4 + 2 + 1;
			const propName = this.textDecoder.decode(new Uint8Array(data.slice(pt, pt + propNameLength)));
			pt += propNameLength;
			const propData = new Uint8Array(data.slice(pt, pt + propSize - (4 + 2 + 1 + propNameLength)));

			this.props.push({ index: i, propTypeID: propTypeID, propName: propName, propData: propData });
			pt = propEnd;
		}
	};
})();

/** @constructor */
function CRunDarkScript() {
	/// <summary> Constructor of Fusion object. </summary>
	CRunExtension.call(this);

	// DarkEdif SDK exts should have these four variables defined.
	// We need this[] and window[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
	this['ExtensionVersion'] = 1; // To match C++ version
	this['SDKVersion'] = 19; // To match C++ version
	this['DebugMode'] = true;
	this['ExtensionName'] = 'DarkScript';

	// Can't find DarkEdif wrapper
	if (!window.hasOwnProperty('darkEdif')) {
		throw "a wobbly";
	}
	window['darkEdif'].checkSupportsSDKVersion(this.SDKVersion);

	// ======================================================================================================
	// Actions
	// ======================================================================================================
	this.Action_ActionExample = function (ExampleParameter) {
		// nothing, as C++ does nothing
	};
	this.Action_SecondActionExample = function () {
		// nothing, as C++ does nothing
	};

	// ======================================================================================================
	// Conditions
	// ======================================================================================================
	this.Condition_AreTwoNumbersEqual = function (First, Second) {
		return First == Second;
	};

	// =============================
	// Expressions
	// =============================

	this.Expression_Add = function (First, Second) {
		return First + Second;
	};
	this.Expression_HelloWorld = function () {
		return "Hello world!";
	};

	// =============================
	// Function arrays
	// =============================

	this.$actionFuncs = [
	/* 0 */ this.Action_ActionExample,
	/* 1 */ this.Action_SecondActionExample
	];
	this.$conditionFuncs = [
	/* 0 */ this.Condition_AreTwoNumbersEqual

	// update getNumOfConditions function if you edit this!!!!
	];
	this.$expressionFuncs = [
	/* 0 */ this.Expression_Add,
	/* 1 */ this.Expression_HelloWorld
	];
}
//
CRunDarkScript.prototype = {
	/// <summary> Prototype definition </summary>
	/// <description> This class is a sub-class of CRunExtension, by the mean of the
	/// CServices.extend function which copies all the properties of
	/// the parent class to the new class when it is created.
	/// As all the necessary functions are defined in the parent class,
	/// you only need to keep the ones that you actually need in your code. </description>

	getNumberOfConditions: function() {
		/// <summary> Returns the number of conditions </summary>
		/// <returns type="Number" isInteger="true"> Warning, if this number is not correct, the application _will_ crash</returns>
		return 1; // $conditionFuncs not available yet
	},

	createRunObject: function(file, cob, version) {
		/// <summary> Creation of the Fusion extension. </summary>
		/// <param name="file"> A CFile object, pointing to the object's data zone </param>
		/// <param name="cob"> A CCreateObjectInfo containing infos about the created object</param>
		/// <param name="version"> version : the version number of the object, as defined in the C code </param>
		/// <returns type="Boolean"> Always false, as it is unused. </returns>

		// Use the "file" parameter to call the CFile object, and
		// gather the data of the object in the order as they were saved
		// (same order as the definition of the data in the EDITDATA structure
		// of the C code).
		// Please report to the documentation for more information on the CFile object

		if (this.ho == null) {
			throw "HeaderObject not defined when needed to be.";
		}

		// DarkEdif properties are accessible as on other platforms: IsPropChecked(), GetPropertyStr(), GetPropertyNum()
		let props = new darkEdif['Properties'](this, file, version);

		this.checkboxWithinFolder = props['IsPropChecked']("Checkbox within folder");
		this.editable6Text = props['GetPropertyStr']("Editable 6");

		// The return value is not used in this version of the runtime: always return false.
		return false;
	},

	handleRunObject: function() {
		/// <summary> This function is called at every loop of the game. You have to perform
		/// in it all the tasks necessary for your object to function. </summary>
		/// <returns type="Number"> One of two options:
		///							   0 : this function will be called during the next loop
		/// CRunExtension.REFLAG_ONESHOT : this function will not be called anymore,
		///								   unless this.reHandle() is called. </returns>
		return CRunExtension.REFLAG_ONESHOT;
	},

	condition: function(num, cnd) {
		/// <summary> Called when a condition of this object is tested. </summary>
		/// <param name="num" type="Number" integer="true"> The number of the condition; 0+. </param>
		/// <param name="cnd" type="CCndExtension"> a CCndExtension object, allowing you to retreive the parameters
		//			of the condition. </param>
		/// <returns type="Boolean"> True if the condition is currently true. </returns>

		const func = this.$conditionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised condition ID " + (~~num) + " passed to " + this['ExtensionName'] + ".";
		}

		// Note: New Direction parameter is not supported by this, add a workaround based on condition and parameter index;
		// SDL Joystick's source has an example.
		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = cnd.getParamExpString(this.rh, i);
		}

		return func.apply(this, args);
	},
	action: function(num, act) {
		/// <summary> Called when an action of this object is executed </summary>
		/// <param name="num" type="Number"> The ID/number of the action, as defined by
		///		its array index. </param>
		/// <param name="act" type="CActExtension"> A CActExtension object, allowing you to
		///		retrieve the parameters of the action </param>

		const func = this.$actionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised action ID " + (~~num) + " passed to " + this['ExtensionName'] + ".";
		}

		// Note: New Direction parameter is not supported by this, add a workaround based on action and parameter index;
		// SDL Joystick's source has an example.
		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = act.getParamExpression(this.rh, i);
		}

		func.apply(this, args);
	},
	expression: function(num) {
		/// <summary> Called during the evaluation of an expression. </summary>
		/// <param name="num" type="Number"> The ID/number of the expression. </param>
		/// <returns> The result of the calculation, a number or a string </returns>

		// Note that it is important that your expression function asks for
		// each and every one of the parameters of the function, each time it is
		// called. The runtime will crash if you miss parameters.

		const func = this.$expressionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised expression ID " + (~~num) + " passed to " + this['ExtensionName'] + ".";
		}

		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = this.ho.getExpParam();
		}

		return func.apply(this, args);
	}

	// No comma for the last function : the Google compiler
	// we use for final projects does not accept it
};

CServices.extend(CRunExtension, CRunDarkScript);
