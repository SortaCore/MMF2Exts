/*
	This DarkEdif Template Fusion extension UWP port is under MIT license.
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
window['darkEdif'] = (window['darkEdif'] && window['darkEdif'].sdkVersion >= 20) ? window['darkEdif'] :
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
	this.sdkVersion = 20;
	this.checkSupportsSDKVersion = function (sdkVer) {
		if (sdkVer < 16 || sdkVer > 20) {
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
		// if prop set v2, then uint64 editor checkbox ptr
		// then checkbox list, one bit per checkbox, including non-checkbox properties
		// so skip numProps / 8 bytes
		// then moving to Data list:
		// size uint32 (includes whole Data), propType uint16, propNameSize uint8, propname u8 (lowercased), then data bytes

		let bytes = edPtrFile.ccfBytes.slice(edPtrFile.pointer);
		
		edPtrFile.skipBytes(ext.ho.privateData - 20); // sub size of eHeader; edPtrFile won't start with eHeader
		const verBuff = new Uint8Array(edPtrFile.readBuffer(4));
		const verStr = String.fromCharCode.apply('', verBuff.reverse());
		let propVer;
		if (verStr == 'DAR2') {
			propVer = 2;
		} else if (verStr == 'DAR1') {
			propVer = 1;
		} else {
			throw "Version string " + verStr + " unknown. Did you restore the file offset?";
		}
		// Pull out hash, hashTypes, numProps, pad, sizeBytes, visibleEditorProps
		let header = new Uint8Array(edPtrFile.readBuffer(4 + 4 + 2 + 2 + 4 + (propVer > 1 ? 8 : 0)));
		let headerDV = new DataView(header.buffer);
		this.numProps = headerDV.getUint16(4 + 4, true); // Skip past hash and hashTypes
		this.sizeBytes = headerDV.getUint32(4 + 4 + 4, true); // skip past numProps and pad

		let editData = edPtrFile.readBuffer(
			this.sizeBytes -
			// skip area between eHeader -> Props
			(ext.ho.privateData - 20) -
			// Skip DarkEdif header
			header.byteLength
		);
		this.chkboxes = editData.slice(0, Math.ceil(this.numProps / 8));
		let that = this;
		let IsComboBoxProp = function(propTypeID) {
			// PROPTYPE_COMBOBOX, PROPTYPE_COMBOBOXBTN, PROPTYPE_ICONCOMBOBOX
			return propTypeID == 7 || propTypeID == 20 || propTypeID == 24;
		};
		let RuntimePropSet = function(data) {
			let rsDV = new DataView(data.propData.buffer);
			let rs = /* RuntimePropSet */ { 
				// Always 'S', compared with 'L' for non-set list.
				setIndicator: String.fromCharCode(rsDV.getUint8(0)),
				// Number of repeats of this set, 1 is minimum and means one of this set
				numRepeats: rsDV.getUint16(1, true),
				// Property that ends this set's data, as a JSON index, inclusive
				lastSetJSONPropIndex: rsDV.getUint16(1 + 2, true),
				// First property that begins this set's data, as a JSON index
				firstSetJSONPropIndex: rsDV.getUint16(1 + (2 * 2), true),
				// Name property JSON index that will appear in list when switching set entry
				setNameJSONPropIndex: rsDV.getUint16(1 + (2 * 3), true),
				// Current set index selected (0+), present at runtime too, but not used there
				get setIndexSelected() {
					return rsDV.getUint16(1 + (2 * 4), true);
				},
				set setIndexSelected(i) {
					rsDV.setUint16(1 + (2 * 4), i, true);
				},
				// Set name, as specified in JSON. Don't confuse with user-specified set name.
				setName: that.textDecoder.decode(data.propData.slice(1 + (2 * 5))),
			};
			if (rs.setIndicator != 'S')
				throw "Not a prop set!";
			return rs;
		};
		let GetPropertyIndex = function(chkIDOrName) {
			if (propVer > 1) {
				let jsonIdx = -1;
				if (typeof chkIDOrName == 'number') {
					const p = that.props.find(function(p) { return p.index == chkIDOrName; });
					if (p == null) {
						throw "Invalid property name \"" + chkIDOrName + "\"";
					}
					jsonIdx = p.propJSONIndex;
				} else {
					const p = that.props.find(function(p) { return p.propName == chkIDOrName; });
					if (p == null) {
						throw "Invalid property name \"" + chkIDOrName + "\"";
					}
					jsonIdx = p.propJSONIndex;
				}
				// Look up prop index from JSON index - DarkEdif::Properties::PropIdxFromJSONIdx
				let data = that.props[0], i = 0;
				while (data.propJSONIndex != jsonIdx) {
					if (i >= that.numProps) {
						throw "Couldn't find property of JSON ID " + jsonIdx + ", hit property " + i + " of " + that.numProps + " stored.\n";
					}
					if (IsComboBoxProp(data.propTypeID) && String.fromCharCode(data.propData[0]) == 'S') {
						let rs = new RuntimePropSet(data);
						let rsContainer = data;
						// We're beyond all of this set's JSON range: skip past all repeats
						if (jsonIdx > rs.lastSetJSONPropIndex) {
							while (data.propJSONIndex != rs.lastSetJSONPropIndex) {
								data = that.props[i++];
							}
							rs = rsContainer = null;
						}
						// It's within this set's range
						else if (jsonIdx >= rs.firstSetJSONPropIndex && jsonIdx <= rs.lastSetJSONPropIndex) {
							if (rs.setIndexSelected > 0) {
								for (let j = 0; ;) {
									data = that.props[++i];
									
									// Skip until end of this entry, then move to next prop
									if (data.propJSONIndex == rs.lastSetJSONPropIndex) {
										if (++j == rs.setIndexSelected) {
											data = that.props[++i];
											break;
										}
									}
								}
								continue;
							} else {
								data = that.props[++i];
								continue;
							}
						}
						// else it's not in this set: continue to standard loop
						else {
							rs = rsContainer = null;
						}
					}
					
					data = that.props[++i];
				}
				return data.index;
			}
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
			];
			if (textPropIDs.indexOf(prop.propTypeID) != -1 || IsComboBoxProp(prop.propTypeID)) {
				// Prop ver 2 added repeating prop sets
				if (propVer == 2 && IsComboBoxProp(prop.propTypeID)) {
					const setIndicator = String.fromCharCode(prop.propData[0]);
					if (setIndicator == 'L') {
						return that.textDecoder.decode(prop.propData.slice(1));
					} else if (setIndicator == 'S') {
						throw "Property " + prop.propName + " is not textual.";
					}
					throw "Property " + prop.propName + " is not a valid list property.";
				}
				let t = that.textDecoder.decode(prop.propData);
				if (prop.propTypeID == 22) { //PROPTYPE_EDIT_MULTILINE
					t = t.replaceAll('\r', ''); // CRLF to LF
				}
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
		this['GetPropertyImageID'] = function(chkIDOrName, imgID) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return -1;
			}
			const prop = that.props[idx];
			if (prop.propTypeID != 23) { // PROPTYPE_IMAGELIST
				throw "Property " + prop.propName + " is not an image list.";
			}
			
			if ((~~imgID != imgID) || imgID < 0) {
				throw "Image index " + imgID + " is invalid.";
			}
			const dv = new DataView(prop.propData.buffer);
			if (imgID >= dv.getUint16(0, true)) {
				return -1;
			}
			
			return imgID.getUint16(2 * (1 + idx), true)
		};
		this['GetPropertyNumImages'] = function(chkIDOrName, imgID) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return -1;
			}
			const prop = that.props[idx];
			if (prop.propTypeID != 23) { // PROPTYPE_IMAGELIST
				throw "Property " + prop.propName + " is not an image list.";
			}
			
			return new DataView(prop.propData.buffer).getUint16(0, true);
		};
		this['GetSizeProperty'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return -1;
			}
			const prop = that.props[idx];
			if (prop.propTypeID != 8) { // PROPTYPE_SIZE
				throw "Property " + prop.propName + " is not an size property.";
			}
			
			const dv = new DataView(prop.propData.buffer);
			return { width: dv.getInt32(0, true), height: dv.getInt32(4, true) };
		};

		this['PropSetIterator'] = this.PropSetIterator = function(nameListJSONIdx, numSkippedSetsBefore, runSetEntry, props) {
			this.nameListJSONIdx = nameListJSONIdx;
			this.numSkippedSetsBefore = numSkippedSetsBefore;
			this.props = that.props;
			this.runSetEntry = runSetEntry;
			
			this.runPropSet = new RuntimePropSet(runSetEntry);
			this.runPropSet.setIndexSelected = 0;
			this.firstIt = true;
			let thatToo = this;
			this.next = function() {
				// next() is called for first iterator
				if (thatToo.firstIt) {
					thatToo.firstIt = false;
				} else {
					++thatToo.runPropSet.setIndexSelected;
				}
				return {
					value: thatToo.runPropSet.setIndexSelected,
					done: thatToo.runPropSet.setIndexSelected >= thatToo.runPropSet.numRepeats
				};
			};
			this[Symbol.iterator] = function () { return this; };
		};
		this['LoopPropSet'] = this.LoopPropSet = function(setName, numSkips = 0) {
			let d;
			for (let i = 0, j = 0; i < that.numProps; ++i) {
				d = that.props[i];
				if (IsComboBoxProp(d.propTypeID) && String.fromCharCode(d.propData[0]) == 'S') {
					if (new RuntimePropSet(d).setName == setName && ++j > numSkips)
						return new that.PropSetIterator(i, j - 1, d, this);
				}
			}
			throw "No set found with name " + setName + ".";
		}

		this.props = [];
		const data = editData.slice(this.chkboxes.length);
		const dataDV = new DataView(new Uint8Array(data).buffer);

		this.textDecoder = null;
		if (globalThis['TextDecoder'] != null) {
			this.textDecoder = new globalThis['TextDecoder']();
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
			pt += 4;
			const propTypeID = dataDV.getUint16(pt, true);
			pt += 2;
			// propJSONIndex does not exist in Data in DarkEdif smart props ver 1, so JSON index is same as prop index
			let propJSONIndex = i;
			if (propVer == 2) {
				propJSONIndex = dataDV.getUint16(pt, true);
				pt += 2;
			}
			const propNameLength = dataDV.getUint8(pt);
			pt += 1;
			const propName = this.textDecoder.decode(new Uint8Array(data.slice(pt, pt + propNameLength)));
			pt += propNameLength;
			const propData = new Uint8Array(data.slice(pt, propEnd));

			this.props.push({ index: i, propTypeID: propTypeID, propJSONIndex: propJSONIndex, propName: propName, propData: propData });
			pt = propEnd;
		}
	};
	this['Surface'] = function(rhPtr, needBitmapFuncs, needTextFuncs, width, height, alpha) {
		if (rhPtr == null || needBitmapFuncs == null || needTextFuncs == null || width == null || height == null || alpha == null)
			throw "Invalid Surface ctor arguments";
		this.rhPtr = rhPtr;
		this.hasGeometryCapacity = needBitmapFuncs;
		this.hasTextCapacity = needTextFuncs;
		this.canvas = document.createElement("canvas");
		this.context = this.canvas.getContext("2d");
		this.altered = false;
		this.canvas.width = width;
		this.canvas.height = height;
		this.mosaic = 0;
		this.xSpot = this.ySpot = 0;
		
		let surf = this;
		this.faux = { img: surf.canvas, mosaic: 0, xSpot: 0, ySpot: 0 };
		this['FillImageWith'] = function(sf) {
			if (sf.fillType == darkEdif['SurfaceFill']['FillType']['Flat']) {
				surf.context.rect(0, 0, surf.canvas.width, surf.canvas.height);
				surf.context.fillStyle = sf.color;
				surf.context.fill();
				this.altered = true;
				return true;
			}
		};
		this['GetAndResetAltered'] = function() {
			if (!this.altered) {
				return false;
			}
			this.altered = false;
			return true;
		}
		this.ext = null;
		this['SetAsExtensionDisplay'] = function(ext) {
			surf.ext = ext;
		};
		this['BlitToFrameWithExtEffects'] = function(renderer, pt) {
			const x = this.ext.ho.hoX + (pt ? pt.x : 0),
				y = this.ext.ho.hoY + (pt ? pt.y : 0);
			let angle = 0, scaleX = 1, scaleY = 1, inkEffect = 1, inkEffectParam = 0;
			if ((this.ext.ho.hoOEFlags & CObjectCommon.OEFLAG_SPRITES) != 0) {
				angle = this.ext.ho.roc.rcAngle;
				scaleX = this.ext.ho.roc.rcScaleX;
				scaleY = this.ext.ho.roc.rcScaleY;
				inkEffect = this.ext.ho.ros.rsEffect;
				inkEffectParam = this.ext.ho.ros.rsEffectParam;
				this.faux.xSpot = this.ext.ho.hoImgXSpot;
				this.faux.ySpot = this.ext.ho.hoImgYSpot;
			}
			
			surf.context.save();
			renderer._context.save();
			renderer.renderImage(this.faux, x, y, angle, scaleX, scaleY, inkEffect, inkEffectParam);
			renderer._context.restore();
			surf.context.restore();
		};
		this.img = surf.canvas;
		
		return this;
	};
	this['SurfaceFill'] = {
		'FillType': {
			'Flat': 0
		},
		'Solid': function(color) {
			this.fillType = darkEdif['SurfaceFill']['FillType']['Flat'];
			this.color = color;
			return this;
		}
	};
	this['ColorRGB'] = function(r,g,b) {
		return `rgba(${r}, ${g}, ${b}, 1.0)`;
	};
})();

/** @constructor */
function CRunDarkEdif_Template() {
	/// <summary> Constructor of Fusion object. </summary>
	CRunExtension.call(this);

	// DarkEdif SDK exts should have these four variables defined.
	// We need this[] and window[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
	this['ExtensionVersion'] = 1; // To match C++ version
	this['SDKVersion'] = 20; // To match C++ version
	this['DebugMode'] = true;
	this['ExtensionName'] = 'DarkEdif Template';

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
CRunDarkEdif_Template.prototype = {
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

CServices.extend(CRunExtension, CRunDarkEdif_Template);
