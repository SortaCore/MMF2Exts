// @ts-check
/// <reference path="D:/Program Files (x86)/Steam/steamapps/common/Clickteam Fusion 2.5/Data/Runtime/Html5/Extensions.js" />
/// <reference path="D:/Program Files (x86)/Steam/steamapps/common/Clickteam Fusion 2.5/Data/Runtime/Html5/Services.js" />
/// <reference path="D:/Program Files (x86)/Steam/steamapps/common/Clickteam Fusion 2.5/Data/Runtime/Html5/RunLoop.js" />
/// <reference path="D:/Program Files (x86)/Steam/steamapps/common/Clickteam Fusion 2.5/Data/Runtime/Html5/Objects.js" />
/// <reference path="D:/Program Files (x86)/Steam/steamapps/common/Clickteam Fusion 2.5/Data/Runtime/Html5/Params.js" />

// tslint:disable: no-bitwise comment-format typedef no-empty class-name interface-name no-use-before-declare max-line-length
// tslint:disable: no-debugger

/*
	This DarkScript Fusion extension HTML5 port is under MIT license.

	Modification for purposes of tuning to your own HTML5 application is permitted, but must retain this notice and not be redistributed,
	outside of its (hopefully minified) presence inside your HTML5 website's source code.
*/
/* global console, darkEdif, globalThis, alert, CRunExtension, FinalizationRegistry, CServices */
/* eslint esversion: 6, sub: true, allowJs: true */

// This is strict, but that can be assumed
"use strict";

don't forget to do that DarkScript fixes of global ID and notification, and check the param index <= size checks are not off by 1



// Global data, including sub-applications, just how God intended.
// Note: This will allow newer SDK versions in later SDKs to take over.
// We need this[] and globalThis[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise

class DarkEdif {
	constructor() {
	// window variable is converted into __scope for some reason, so globalThis it is.
	/** @type object */ 
	this.data = {};
	/** @param {string} key
	 * @returns object | null */ 
	this.getGlobalData = function (key) {
		key = key.toLowerCase();
		if (key in this.data) {
			return this.data[key];
		}
		return null;
	};
	/** @param {string} key
	 *  @param {object | null} value
	 * @returns void */ 
	this.setGlobalData = function (key, value) {
		key = key.toLowerCase();
		this.data[key] = value;
	};

	/** @param {CRunExtension} ext
	 * @returns number */
	this.GetCurrentFusionEventNum = function (ext) {
		return ext.rh.rhEvtProg.rhEventGroup == null ? -1 : ext.rh.rhEvtProg.rhEventGroup.evgLine;
	};
	this.sdkVersion = 19;
	this.checkSupportsSDKVersion = function (sdkVer) {
		if (sdkVer < 16 || sdkVer > 19) {
			throw "HTML5 DarkEdif SDK does not support SDK version " + this.sdkVersion;
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
	if (!this['minified']) {
		let that = this;
		// Use this for debugging to make sure objects are deleted.
		// Note they're not garbage collected when last holder releases it, but at any point after,
		// when the GC decides to.
		// On Chrome, it took half a minute or so, and delay was possibly affected by whether the page has focus.
		// GC is not required, remember - the cleanup may not happen at all in some browsers.
		this.finalizer = new FinalizationRegistry(function (desc) {
			that.consoleLog(null, "Noting the destruction of [" + desc + "].");
		});
	}
	else {
		this.finalizer = { register: function (desc) { } };
	}
	
	this['Properties'] = function (ext, edPtrFile, extVersion) {
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
		let GetPropertyIndex = function (chkIDOrName) {
			if (typeof chkIDOrName == 'number') {
				if (that.numProps <= chkIDOrName) {
					throw "Invalid property ID " + chkIDOrName + ", max ID is " + (that.numProps - 1) + ".";
				}
				return chkIDOrName;
			}
			const p = that.props.find(function (p) { return p.propName == chkIDOrName; });
			if (p == null) {
				throw "Invalid property name \"" + chkIDOrName + "\"";
			}
			return p.index;
		};
		/** @returns boolean */
		this['IsPropChecked'] = function (chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return false;
			}
			return (that.chkboxes[Math.floor(idx / 8)] & (1 << idx % 8)) != 0;
		};
		this['GetPropertyStr'] = function (chkIDOrName) {
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
				if (prop.propTypeID == 22) { //PROPTYPE_EDIT_MULTILINE
					t = t.replaceAll('\r', ''); // CRLF to LF
				}
				return t;
			}
			throw "Property " + prop.propName + " is not textual.";
		};
		this['GetPropertyNum'] = function (chkIDOrName) {
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
		if (this.IsPropChecked == undefined) {
			this.IsPropChecked = this['IsPropChecked'];
			this.GetPropertyStr = this['GetPropertyStr'];
			this.GetPropertyNum = this['GetPropertyNum'];
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
				decode: function (txt) {
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

	/** @enum number */
	this.Selection = Object.freeze({
		// Selects all
		All: 0,
		// Includes selected explicitly by conditions, includes all other obj instances selected implicitly
		Implicit: 1,
		// Selected explicitly by conditions only, excludes objects that have all obj instances selected implicitly
		Explicit: 2
	});
	if (this['Selection'] == null) {
		this['Selection'] = this.Selection;
		if (this.Selection['All'] == null) {
			this.Selection['All'] = this['Selection'].All;
			this.Selection['Implicit'] = this['Selection'].Implicit;
			this.Selection['Explicit'] = this['Selection'].Explicit;
		}
	}

	// Iterates the object instances in qualifier OI or singular OI
	this.ObjectIterator = this['ObjectIterator'] = /** @constructor */ function(rhPtr, oiList, selection, includeDestroyed) {
		this.rhPtr = rhPtr;
		this.oiList = oiList;
		this.select = selection;
		this.includeDestroyed = includeDestroyed;
		this.curOiList = oiList;
		this.qualOiList = [];
		this.oil = null;
		this.curHo = null;
		this.curRo = null;
		this.qualOiListAt = 0;
		this.numNextRun = 0;
		this.skipNextTick = false;

		this.next = function() {
			if (this.oil == null || this.curHo == null) {
				return { done: true };
			}
			this.GetNext();
			return { value: this.curRo, done: this.curRo == null };
		};
		this[Symbol.iterator] = function () { return this; }

		this.GetNext = () => {
			if (this.skipNextTick) {
				this.skipNextTick = false;
				return;
			}
			while (true) {
				++this.numNextRun;
				let nextOi = -1;
				if (this.curHo) {
					const ecMatch2 = this.oil.get_EventCount() == this.rhPtr.GetRH2EventCount();
					nextOi = this.select == darkEdif['Selection']['All'] ? this.curHo.get_NumNext() : this.curHo.get_NextSelected();
		
					// If implicit selection and no condition-selection, select first OI in general
					if ((!ecMatch2 || (nextOi & 0xF000) != 0) && this.select == darkEdif['Selection']['Implicit']) {
						nextOi = this.curHo.get_NumNext();
					}
				}
				// Invalid Oi; jump to next object
				if ((nextOi & 0xF000) != 0) {
					if (this.qualOiList.length > this.qualOiListAt) {
						this.curOiList = this.qualOiList[this.qualOiListAt++];
						this.oil = this.rhPtr.GetOIListByIndex(this.curOiList);
		
						const ecMatch = this.oil.get_EventCount() == this.rhPtr.GetRH2EventCount();
						nextOi = this.select == darkEdif['Selection']['All'] ? this.oil.get_Object() : this.oil.get_ListSelected();
		
						// If implicit selection and no condition-selection, select first OI in general
						if ((!ecMatch || (nextOi & 0xF000) != 0) && this.select == darkEdif['Selection']['Implicit']) {
							nextOi = this.oil.get_Object();
						}
		
						// Skip to next frame: it'll either fall through to end of list, or grab next in qualifier
						if ((nextOi & 0xF000) != 0) {
							continue;
						}
					}
					// hit end of list
					else {
						this.curHo = null;
						this.curRo = null;
						this.oil = null;
						this.curOiList = -1;
						this.numNextRun = Number.MAX_VALUE;
						return;
					}
				}
				this.curRo = this.rhPtr.GetObjectListOblOffsetByIndex(nextOi);
				this.curHo = this.curRo != null ? this.curRo.get_rHo() : null;
				if (this.curRo && (this.includeDestroyed || (this.curHo.get_Flags() & CObject.HOF_DESTROYED) == CObject.HOF_NONE)) {
					return; // we got a valid one
				}
			}
		};
		if (this.curOiList != -1) {
			if ((this.curOiList & 0x8000) == 0) {
				this.qualOiList.push(this.oiList);
			}
			else {
				const qToOiList = this.rhPtr.GetQualToOiListByOffset(this.curOiList);
				if (qToOiList) {
					this.qualOiList = qToOiList.GetAllOiList();
				}
			}
			this.GetNext();
			this.skipNextTick = true;
		}
	};

	this.QualifierOIListIterator = this['QualifierOIListIterator'] = /** @constructor */ function(rhPtr, oiList, select) {
		this.rhPtr = rhPtr;
		this.oiList = oiList;
		this.select = select;
		this.curOiList = oiList;
		this.qualOiList = [];
		this.oil = null;
		this.qualOiListAt = 0;
		this.skipNextTick = false;

		this.GetNext = () => {
			if (this.skipNextTick) {
				this.skipNextTick = false;
				return;
			}
			for (let firstOi; this.qualOiListAt < this.qualOiList.length;) {
				this.curOiList = this.qualOiList[this.qualOiListAt++]; // always increment; end iterator is indicated by == length
				this.oil = this.rhPtr.GetOIListByIndex(this.curOiList);
				const ecMatch = this.oil.get_EventCount() == this.rhPtr.GetRH2EventCount();
				firstOi = this.select == darkEdif['Selection']['All'] ? this.oil.get_Object() : this.oil.get_ListSelected();

				// If implicit selection and no condition-selection, select first OI in general
				if ((!ecMatch || firstOi == -1) && this.select == darkEdif['Selection']['Implicit']) {
					firstOi = this.oil.get_Object();
				}
				if ((firstOi & 0xF000) == 0) {
					return; // invalid flag were not set, so this OiList has some valid instances; else continue loop and find next oilist
				}
			}

			// End of list, clear up
			this.oil = null;
			this.curOiList = -1;
		};
		this.next = function() {
			if (this.oil == null)
				return { done: true };
			this.GetNext();
			return { value: this.oil, done: this.oil == null};
		};
		
		this[Symbol.iterator] = function() {
			return this;
		}
		
		if (this.curOiList == -1) {
			return;
		}

		// Not a qualifier OI, make an iterator that only returns it
		if ((this.curOiList & 0xF000) == 0) {
			this.qualOiList.push(this.curOiList);
		}
		else {
			const qToOiList = this.rhPtr.GetQualToOiListByOffset(this.curOiList);
			if (qToOiList) {
				this.qualOiList = qToOiList.GetAllOiList();
			}
		}
		this.GetNext();
		this.skipNextTick = true;
	}
	
	this.AllObjectIterator = this['AllObjectIterator'] = /** @constructor */ function (rhPtr) {
		this.rhPtr = rhPtr;
		this.numObjectsInFrame = this.rhPtr.get_NObjects();
		this.curRo = null;
		this.objListAt = 0;
		this.objListTrueIndex = 0;
		this.skipNextTick = false;
		// In theory there cannot be 0 objects, since this code can't be run without an object to run it
		if (this.numObjectsInFrame > 0) {
			this.curRo = this.rhPtr.GetObjectListOblOffsetByIndex(0);
			this.skipNextTick = true;
		}

		this.next = function() {
			if (this.curRo == null)
				return { done: true };

			if (this.skipNextTick) {
				this.skipNextTick = false;
				return { value: this.curRo, done: this.curRo == null };
			}

			// haven't hit end of rhObjectList, but NObjects is how many valid entries are in rhObjectList,
			// so just abort early, rather than loop past all the reserved nulls in rhObjectList
			if (++this.objListAt == this.numObjectsInFrame) {
				this.curRo = null;
				return { done: true };
			}

			while (true) {
				// This can be null, even before maxObjectsInFrame is reached, if object is removed
				this.curRo = this.rhPtr.GetObjectListOblOffsetByIndex(++this.objListTrueIndex);
				if (this.curRo != null) {
					break;
				}
				// skip nulls in middle of rhObjectList
			}
			return { value: this.curRo, done: this.curRo == null };
		}
		
		this[Symbol.iterator] = function() {
			return this;
		}
	};
	
	this.AllOIListIterator = this['AllOIListIterator'] = /** @constructor */ function (rhPtr) {
		this.rhPtr = rhPtr;
		this.numOI = rhPtr.GetNumberOi();
		this.oil = null;
		this.oiListAt = 0;
		this.skipNextTick = false;
		if (this.numOI > 0) {
			this.oil = this.rhPtr.GetOIListByIndex(0);
			this.skipNextTick = true;
		}
		this.next = function () {
			if (this.oil == null)
				return { done: true };
			if (this.skipNextTick)  {
				this.skipNextTick = false;
				return { value: this.oil, done: this.oil == null };
			}
			if (++this.oiListAt == this.numOI) {
				this.oil = null;
			}
			else {
				this.oil = this.rhPtr.rhOiList[this.oiListAt];
			}
			return { value: this.oil, done: this.oil == null };
		}
		this[Symbol.iterator] = function() {
			return this;
		}
	};
	this.EdifRuntime = function(ext) { return new EdifRuntime(ext); }
}
};
class EdifRuntime { 
	/**
	 * 
	 * @param {CRunExtension} ext 
	 */
	constructor(ext) {
		this.hoPtr = ext.ho;
		this.rhPtr = ext.rh;
		this.ObjectSelection = ext.ObjectSelection = new ObjectSelection(ext.rh.rhApp);
		this.curCEvent = null;
		this.steadilyIncreasing = 0;
	}
	GetCurrentFusionFrameNumber(){
		// 0-based in HTML5, make 1-based
		return 1 + this.rhPtr.rhApp.currentFrame;
	}
	GetCurrentFusionEventNum(){
		return this.rhPtr.rhEvtProg.rhEventGroup.evgLine || -1;
	}
	GenerateEvent(id) {
		// If there is a fastloop in progress, generating an event inside an action will alter
		// Action Count, making it smaller. Thus Action Count constantly decrementing will create
		// an infinite loop, as Action Count never reaches the maximum needed to end the loop.
		// This bug usually shows when creating 2+ of an object inside a fastloop when the fastloop runs 3+ times.
		const oldActionCount = this.rhPtr.GetRH2ActionCount();
		const oldActionLoopCount = this.rhPtr.GetRH2ActionLoopCount();

		// This action count won't be reset, so to allow multiple of the same event with different object selection,
		// we change the count every time, and in an increasing manner.
		this.rhPtr.SetRH2ActionCount(oldActionCount + (++this.steadilyIncreasing));
		this.rhPtr.SetRH2ActionLoopCount(0);

		// Saving tokens allows events to be run from inside expressions
		// https://community.clickteam.com/forum/thread/108993-application-crashed-in-some-cases-when-calling-an-event-via-expression/?postID=769763#post769763
		// As of CF2.5 build 293.9, this is done by runtime anyway, but if you want to
		// support Fusion 2.0, and you're generating events from expressions, you should include this
		const saveExpToken = this.rhPtr.rh4CurToken;
		const rh4Tokens = this.rhPtr.rh4Tokens;

		// Fix event group being incorrect after event finishes.
		// This being incorrect doesn't have any major effects, as the event parsing part of
		// runtime sets rhEventGroup based on a local variable evgPtr, which it relies on instead
		const evg = this.rhPtr.rhEvtProg.rhEventGroup;
		
		// Fix rh2ActionOn - affects whether object selection is modified by expressions, or used
		const rh2ActionOn = this.rhPtr.rh2ActionOn;
		if (rh2ActionOn)
			this.rhPtr.rh2ActionOn = false;

		this.hoPtr.generateEvent(id);

		this.rhPtr.rh2ActionOn = rh2ActionOn;
		this.rhPtr.rhEvtProg.rhEventGroup = evg;
		this.rhPtr.rh4CurToken = saveExpToken;
		this.rhPtr.rh4Tokens = rh4Tokens;
		this.rhPtr.SetRH2ActionCount(oldActionCount);
		this.rhPtr.SetRH2ActionLoopCount(oldActionLoopCount);
	}
	Rehandle() { /* @ts-ignore */
		this.hoPtr.reHandle();
	}
	/**
	 * 
	 * @param {number} fv 
	 * @returns CRunExtension | null
	 */
	GetRunObjFromFixed(fv) { /* @ts-ignore */
		console.debug(`Getting object from fixed value ${fv}.`)
		return this.hoPtr.getObjectFromFixed(fv);
	}
	ReadGlobal(key) {
		return darkEdif.getGlobalData(key)
	}
	WriteGlobal(key, value) {
		darkEdif.setGlobalData(key, value);
	}
	/**
	 * For Object action parameters. Cancels other selected instances of the OI being looped through by Fusion runtime.
	 * Only necessary if you are looping the instances yourself, or doing a singleton pattern.
	 */
	CancelRepeatingObjectAction() {
		// The RunObject * passed to a Object action parameter points to the first instance of the selected objects
		// that this action has.
		// Normal behaviour is the runtime will call the action repeatedly, once for each object instance.
		// So you just run as if the RunObject * is the only instance it's being run on, and the Fusion runtime
		// will cycle through each RunObject * that is the object instances passed by the event.
		// Otherwise, you disable ACTFLAGS_REPEAT to prevent the internal loop.
		//
		// For conditions, Fusion passes Object directly as ParamObject * instead, which DarkEdif reads the oiList from
		// and passes that to the Ext::Condition function; any looping you want to do is to be done using that oiList,
		// e.g. by using a darkEdif.ObjectIterator() or rhPtr.hoAdRunHeader.GetOIListByIndex()
		let actStart = this.rhPtr.GetRH4ActionStart();
		actStart.set_evtFlags(actStart.get_evtFlags() & ~CAct.ACTFLAGS_REPEAT);
	}
	/**
	 * For Object action parameters. Returns the object/qualifier OIListIndex used in the events; only necessary if you are looping the instances yourself.
	 * Reads OI number from parameter, used with object parameter for actions
	 * @param {number} paramIndex Index of action/condition to read param from 
	 * @returns number
	 * @remarks This works for conditions too, but it should be unnecessary, as they're passed this OI directly.
	 */
	GetOIListIndexFromObjectParam(paramIndex) {
		const PARAM_OBJECT_ID = 1;
		if (paramIndex < 0 || paramIndex >= this.curCEvent.evtParams.length) {
			throw Error("GetOIListIndexFromObjectParam: Returning a OI for a nonexistent parameter.");
		}
		/** @type PARAM_OBJECT & { code:number } */
		const curParam = this.curCEvent.evtParams[paramIndex];
		if (curParam.code != PARAM_OBJECT_ID) {
			throw Error("GetOIListIndexFromObjectParam: Returning a OI for a non-Object parameter.");
		}
		console.debug(`GetOIListIndexFromObjectParam: Returning OiList ${curParam.oiList}, oi is ${curParam.oi}.`);
		return curParam.oiList;
	}
	RunObjPtrFromFixed(fixed) {
		return this.GetRunObjFromFixed(fixed);
	}
};
globalThis.darkEdif = new DarkEdif();
/*
globalThis['darkEdif'] = globalThis.darkEdif = (globalThis['darkEdif'] && globalThis['darkEdif'].sdkVersion >= 19) ? globalThis['darkEdif'] :
	@type DarkEdif new
})(); */

const wasSetup = function () {
	if (CRunExtension.prototype['rhPtr'] != null) {
		return true;
	}
	Object.defineProperty(CRunExtension.prototype, 'rhPtr', {
		get: function() { return this.rh; },
		set: function(_) { /* ignore */ }
	});
	Object.defineProperty(CRunExtension.prototype, 'rdPtr', {
		get: function() { return this; },
		set: function(_) { /* ignore */ }
	});
	let bob;
	Object.defineProperty(CRunExtension.prototype, 'Runtime', {
		get: function() {
			return bob || (bob = new darkEdif.EdifRuntime(this));
		},
		set: function(_) { /* ignore */ }
	});

	// Gets the RH2 event count, used in object selection
	CRun.prototype.GetRH2EventCount = function() {
		return this.rhEvtProg.rh2EventCount;
	};
	// Gets the RH4 event count for OR, used in object selection in OR-related events.
	CRun.prototype.GetRH4EventCountOR = function() {
		return this.rhEvtProg.rh4EventCountOR;
	};
	// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
	CRun.prototype.GetRH2ActionCount = function() {
		return this.rhEvtProg.rh2ActionCount;
	};
	// Reads the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions.
	CRun.prototype.GetRH2ActionLoopCount = function() {
		return this.rhEvtProg.rh2ActionLoopCount;
	};

	// Sets the rh2.rh2ActionCount variable, used in an action with multiple instances selected, to repeat one action.
	CRun.prototype.SetRH2ActionCount = function(newActionCount) {
		this.rhEvtProg.rh2ActionCount = newActionCount;
	};
	// Sets the rh2.rh2ActionLoopCount variable, used in a fastloop to loop the actions in an event.
	CRun.prototype.SetRH2ActionLoopCount = function(newActLoopCount) {
		this.rhEvtProg.rh2ActionLoopCount = newActLoopCount;
	};

	CRun.prototype.get_ObjectList = function() {
		return this.rhObjectList;
	};
	CRun.prototype.GetOIListByIndex = function(index) {
		console.assert(this.rhMaxOI > index); // invalid OI
		return this.rhOiList[index];
	};
	CRun.prototype.GetRH4ActionStart = function() {
		if (this.rhEvtProg.rh4ActionStart.get_evtNum == null) {
			globalThis.ActionPrototype(this.rhEvtProg.rh4ActionStart);
		}
		return this.rhEvtProg.rh4ActionStart;
	};
	CRun.prototype.GetRH2ActionOn = function() {
		// rh2ActionOn is sometimes 0, sometimes false
		return !!this.rhEvtProg.rh2ActionOn;
	};
	CRun.prototype.get_EventGroup = function() {
		return this.rhEvtProg.rhEventGroup;
	};
	CRun.prototype.GetNumberOi = function() {
		return this.rhMaxOI;
	};
	CRun.prototype.GetQualToOiListByOffset = function(byteOffset) {
		return this.rhEvtProg.qualToOiList[byteOffset & 0x7FFF];
	};
	CRun.prototype.GetObjectListOblOffsetByIndex = function(index) {
		return this.rhObjectList[index];
	};

	CRun.prototype.GetEVGFlags = function() {
		return this.rhEvtProg.rhEventGroup.evgFlags;
	};
	CRun.prototype.get_MaxObjects = function() {
		return this.rhMaxObjects;
	};
	CRun.prototype.get_NObjects = function() {
		return this.rhNObjects;
	};
	CRun.prototype.get_App = function() {
		return this.rhApp;
	};

	CObject.prototype.get_NextSelected = function() {
		return this.hoNextSelected;
	};
	CObject.prototype.get_CreationId = function() {
		return this.hoCreationId;
	};
	CObject.prototype.get_Number = function() {
		return this.hoNumber;
	};
	CObject.prototype.get_NumNext = function() {
		return this.hoNumNext;
	};
	CObject.prototype.get_Oi = function() {
		return this.hoOi;
	};
	CObject.prototype.GetFixedValue = function() {
		return (this.get_CreationId() << 16) | this.get_Number();
	};
	CObject.prototype.get_OiList = function() {
		return this.hoOiList;
	};
	CObject.prototype.get_SelectedInOR = function() {
		return this.hoSelectedInOR;
	};
	CObject.prototype.get_Flags = function() {
		return this.hoFlags;
	};
	CObject.prototype.get_AdRunHeader = function() {
		return this.hoAdRunHeader;
	};
	CObject.prototype.set_NextSelected = function(ns) {
		this.hoNextSelected = ns;
	};
	CObject.prototype.set_SelectedInOR = function(b) {
		this.hoSelectedInOR = b;
	};

	globalThis.ActionPrototype = function(ac) {
		const ac2 = ac.prototype || ac;
		ac2.get_evtNum = function() {
			return this.evtCode >>> 16;
		};
		ac2.get_evtOi = function() {
			return this.evtOi;
		};
		/*ac2.get_evtSize = function() {
			return this.evtSize;
		}*/
		ac2.get_evtFlags = function() {
			return this.evtFlags;
		};
		ac2.set_evtFlags = function(evtF) {
			this.evtFlags = evtF;
		};
		ac2.Next = function() {
			if (this.index == undefined) {
				throw new Error("CEvent.Next() is not expected outside of a lookup in DarkEdif.");
			}
			return this.parent.GetCAByIndex(this.index + 1);
		};
		ac2.GetIndex = function() {
			if (this.index == undefined) {
				throw new Error("CEvent.GetIndex() is not expected outside of a lookup in DarkEdif.");
			}
			return this.index;
		};
	}
	// Extend all actions and conditions... for some reason, the runtime
	// has them entirely independent of each other
	for (const f in globalThis) {
		if (f.startsWith('ACT_') || f.startsWith('CAct')) {
			console.error("Got ACT! in globalThis");
			globalThis.ActionPrototype(f);
		}
		else if (f.startsWith('CND_') || f.startsWith('CCnd')) {
			globalThis.ActionPrototype(f);
		}
	};
	for (const g in window) {
		if (g.startsWith('ACT_') || g.startsWith('CAct')) {
			console.error("Got ACT! in window");
			break;
		}
	}

	CObjectCommon.OEFLAG_NONE = 0;
	CObject.HOF_NONE = 0;
	CRunExtension.prototype.get_rHo = function() {
		return this.ho;
	};
	CRunExtension.prototype.get_roc = function() {
		return this.roc || null;
	};
	CRunExtension.prototype.get_rom = function() {
		return this.rom || null;
	};
	CRunExtension.prototype.get_roa = function() {
		return this.roa || null;
	};
	CRunExtension.prototype.get_ros = function() {
		return this.ros || null;
	};
	CRunExtension.prototype.get_rov = function() {
		return this.rov || null;
	};
	CExtension.prototype.get_rHo = function() {
		// In HTML5, HeaderObject stuff is stored in RunObject directly
		return this.ho || this;
	};
	CExtension.prototype.get_roc = function() {
		return this.roc || null;
	};
	CExtension.prototype.get_rom = function() {
		return this.rom || null;
	};
	CExtension.prototype.get_roa = function() {
		return this.roa || null;
	};
	CExtension.prototype.get_ros = function() {
		return this.ros || null;
	};
	CExtension.prototype.get_rov = function() {
		return this.rov || null;
	};
	CObject.prototype.get_rHo = function() {
		// In HTML5, HeaderObject stuff is stored in RunObject directly
		return this.ho || this;
	};
	CObject.prototype.get_roc = function() {
		return this.roc || null;
	};
	CObject.prototype.get_rom = function() {
		return this.rom || null;
	};
	CObject.prototype.get_roa = function() {
		return this.roa || null;
	};
	CObject.prototype.get_ros = function() {
		return this.ros || null;
	};
	CObject.prototype.get_rov = function() {
		return this.rov || null;
	};
	/*
	objectsList::GetOblOffsetByIndex = function(index) {
		return this[index].oblOffset;
	};*/

	CObjInfo.prototype.SelectNone = function(rhPtr) {
		this.set_NumOfSelected(0);
		this.set_ListSelected(-1);
		this.set_EventCount(rhPtr.GetRH2EventCount());
	};
	CObjInfo.prototype.SelectAll = function(rhPtr, explicitAll = false) {
		if (!explicitAll) {
			this.set_NumOfSelected(0);
			this.set_ListSelected(-1);
			this.set_EventCount(-5); // does not match rh2EventCount, so implictly selects all
			return;
		}
	
		this.set_NumOfSelected(this.get_NObjects());
		this.set_ListSelected(this.get_Object());
		this.set_EventCount(rhPtr.GetRH2EventCount());
		if (this.get_NObjects() == 0) {
			return;
		}
		const ourOiList = rhPtr.GetObjectListOblOffsetByIndex(this.get_Object()).get_rHo().get_Number();
		for (const ho of new darkEdif.ObjectIterator(rhPtr, ourOiList, darkEdif.Selection.All, false)) {
			ho.get_rHo().set_NextSelected(ho.get_rHo().get_NumNext());
		}
	};
	CObjInfo.prototype.get_EventCount = function() {
		return this.oilEventCount;
	};
	CObjInfo.prototype.get_ListSelected = function() {
		return this.oilListSelected;
	};
	CObjInfo.prototype.get_NumOfSelected = function() {
		return this.oilNumOfSelected;
	};
	CObjInfo.prototype.get_Oi = function() {
		return this.oilOi;
	};
	CObjInfo.prototype.get_NObjects = function() {
		return this.oilNObjects;
	};
	CObjInfo.prototype.get_Object = function() {
		return this.oilObject;
	};
	CObjInfo.prototype.get_name = function() {
		return this.oilName;
	};
	CObjInfo.prototype.get_oilNext = function() {
		return this.oilNext;
	};
	CObjInfo.prototype.get_oilNextFlag = function() {
		return this.oilNextFlag;
	};
	CObjInfo.prototype.get_oilCurrentRoutine = function() {
		return this.oilCurrentRoutine;
	};
	CObjInfo.prototype.get_oilCurrentOi = function() {
		return this.oilCurrentOi;
	};
	CObjInfo.prototype.get_oilActionCount = function() {
		return this.oilActionCount;
	};
	CObjInfo.prototype.get_oilActionLoopCount = function() {
		return this.oilActionLoopCount;
	};
	CObjInfo.prototype.set_NumOfSelected = function(ns){
		this.oilNumOfSelected = ns;
	};
	CObjInfo.prototype.set_ListSelected = function(sh) {
		this.oilListSelected = sh;
	};
	CObjInfo.prototype.set_EventCount = function(ec) {
		this.oilEventCount = ec;
	};
	CObjInfo.prototype.set_EventCountOR = function(ec) {
		this.oilEventCountOR = ec;
	};
	CObjInfo.prototype.get_QualifierByIndex = function(index) {
		return this.oilQualifiers[index];
	};
	// returns the object in this qualifier
	CQualToOiList.prototype.get_Oi = function(idx) {
		return this.qoiList[idx * 2];
	};
	CQualToOiList.prototype.get_OiList = function(idx) {
		return this.qoiList[idx * 2 + 1];
	};
	// Returns all OiList from internal array, used for looping through a qualifier's object IDs
	CQualToOiList.prototype.GetAllOi = function() {
		return this.qoiList.filter((_, index) => index % 2 === 0);
	};
	// Returns all OiList from internal array, used for looping through a qualifier's objInfoList
	CQualToOiList.prototype.GetAllOiList = function() {
		return this.qoiList.filter((_, index) => index % 2 === 1);
	}

	CEventGroup.prototype.get_evgNCond = function() {
		return this.evgNCond;
	};
	CEventGroup.prototype.get_evgNAct = function() {
		return this.evgNAct;
	};
	CEventGroup.prototype.get_evgIdentifier = function() {
		return this.evgIdentifier;
	};
	CEventGroup.prototype.get_evgInhibit = function() {
		return this.evgInhibit;
	};
	CEventGroup.prototype.GetCAByIndex = function(index) {
		if (index >= this.evgNCond + this.evgNAct) {
			return null;
		}
		// Extend the return so Next() and GetIndex() work after
		const e = this.evgEvents[index];
		if (e.index == undefined) {
			e.index = index;
			e.parent = this;
		}
		if (e.get_evtNum == undefined) {
			globalThis.ActionPrototype(e);
		}
		return e;
	};
	return true;
}();
/** Manages discrepancies between supplied type and expected type @enum number */
const DarkScript_ConversionStrictness = Object.freeze({
	/** Requested type must match input type @constant */
	Exact: 0,
	/** Float and integers can be exchanged, text <-> number is errors  @constant */
	Numeric: 1,
	/** Strings containing numbers can be converted to numbers and vice versa
		If conversion fails, errors is used  @constant */
	AnyWithAborts: 2,
	/** Strings containing numbers can be converted to numbers and vice versa
		If conversion fails, default is used @constant */
	AnyWithDefaults: 3
});
/** Sets contraint where no return value set is allowed when function finishes @enum number */
const DarkScript_AllowNoReturnValue = Object.freeze({
	/** All functions must have a return value set when they return, even if they don't use them @constant */
	Never: 0,
	/** Allow foreach actions and delayed function calls, which don't use the func return directly
		to not set a return value @constant */
	ForeachDelayedActionsOnly: 1,
	/** Allow foreach actions, delayed function calls, and anonymous functions @constant */
	AnonymousForeachDelayedActions: 2,
	/** Any function can forgo setting return value and use the defaults of "" or 0 @constant */
	AllFunctions: 3
});

/** Per object list of selected instances */
class DarkScript_FusionSelectedObjectListCache {
	/** @param {CObjInfo} poil */
	constructor(poil) {
		/** @type CObjInfo */
		this.poil = poil;
		/** hoNumber for this object info @type number[] */
		this.selectedObjects = []; // short[]
	}
}
/** Parameter and return type enum for DarkScript @enum number */
const DarkScript_Type = Object.freeze({
	// Any allows weakly-typed functions. Not sure if that'd ever be useful, but there ya go.
	Any: 0,
	Integer: 1,
	String: 2, // String must be before float, because ExpReturnType has it that way
	Float: 3,
	
	// UPDATE THESE WHEN CHANGING THE ABOVE
	NumCallableReturnTypes: 3,
	MaskReturnTypes: 0x3,
	NumCallableParamTypes: 3,
	MaskParamTypes: 0x3,
});

/** Stores parameters and their defaults in DarkScript functions */
class DarkScript_Value {
	/** @param {DarkScript_Type} type */
	constructor (type) {
		/** @type {DarkScript_Type} */
		this.type = type == null ? DarkScript_Type.Any : type;
		/** @type string | number */
		this.data = 0; // string, integer, decimal
		this.dataSize = 0;
	}
	/** Creates a deep copy of a value */
	clone() {
		let v = new DarkScript_Value(this.type);
		v.data = this.data;
		v.dataSize = this.dataSize;
		return v;
	};
}

/** Stores parameters and their defaults in DarkScript functions */
class DarkScript_Param {
	/** @param {string} name
		@param {DarkScript_Type} typ */
	constructor(name, typ) {
		/** Name of parameter @type string */
		this.name = name;
		/** Name of parameter, lowercased @type string */
		this.nameL = name.toLowerCase();
		/** Type of parameter. Can be Any to allow any type of defaultVal;
		 *  if defaultVal.type == Any, it is unset @type DarkScript_Type */
		this.type = typ;
		/** Type as Any for no default @type DarkScript_Value */
		this.defaultVal = new DarkScript_Value(DarkScript_Type.Any);
	}
}
class DarkScript_ScopedVar extends DarkScript_Param {
	/** @param {string} name
		@param {DarkScript_Type} typ
		@param {boolean} recursiveOverride
		@param {number} level */
	constructor (name, typ, recursiveOverride, level) {
		super(name, typ);
		/** If true, resets value to defaultVal when existing scoped var
		 *  if not, inherits value. @type boolean */
		this.recursiveOverride = recursiveOverride;
		/** Indicates how deep in runningFunc vector this is set
		 *  0 is global, 1 is first func layer, etc @type number */
		this.level = level;
	}
}
/** Whether something is expected, denied or optional @enum number */
const DarkScript_Expected = Object.freeze({
	Never: 0,
	Always: 1,
	Either: 2,
});
/** Flags indicating variant of DarkScript function, e.g. object selection kept inside @enum {number} 
 *  @remarks If this is updated, look everywhere for KR func regexes, like running from script and declaration/template setup */
const DarkScript_Flags = Object.freeze({
	None: 0b00,
	KeepObjSelection: 0b01,
	Repeat: 0b10,
	Both: 0b11
});
/** Reflects SDK ExpReturnType enum @enum number */
const DarkScript_ExpReturnType = Object.freeze({
	Integer: 0,
	String: 1,
	Float: 2,
});
/** Reflects SDK ExpParams enum @enum number */
const DarkScript_ExpParams = Object.freeze({
	Integer: 1,
	Float: 1,
	String: 3
});
/** Reflects SDK ExpInfo class */
class DarkScript_ExpInfo {
	constructor(ID, paramCount) {
		/** Fusion expression ID (0+) @type number */
		this.ID = ID;
		/** Integer with 1 << n indicating param n is float, not int @type number */
		this.FloatFlags = 0;
		/** @type DarkScript_ExpReturnType*/
		this.Flags = 0;
		/** @type number */
		this.NumOfParams = paramCount;
		/** @type DarkScript_ExpParams[] */
		this.Parameter = new Array(paramCount);
	}
}

class DarkScript_FunctionTemplate {
	/** 
	 * @param {CRunDarkScript} ext 
	 * @param {DarkScript_Expected} delayable 
	 * @param {string} funcName Function name
	 * @param {boolean} recursable Recursable
	 * @param {DarkScript_Expected} repeatable 
	 * @param {DarkScript_Type} returnType
	 */
	constructor(ext, funcName, delayable, repeatable, recursable, returnType) {
		/** Function name @type string */
		this.name = funcName;
		/** Function name, lowercase @type string */
		this.nameL = funcName.toLowerCase();
		/** @type DarkScript_Expected*/
		this.repeating = repeatable;
		/** @type DarkScript_Expected */
		this.delaying = delayable;
		/** @type boolean */
		this.recursiveAllowed = recursable;
		/** If false, no events are generated, it returns default return value instantly to caller @type boolean */
		this.isEnabled = true;
		/** return type can be Any, or match defaultReturnValue @type DarkScript_ExpReturnType */
		this.returnType = returnType;
		/** @type DarkScript_Value */
		this.defaultReturnValue = new DarkScript_Value(returnType);
		/** @type DarkScript_Param[] */
		this.params = [];
		/** @type DarkScript_ScopedVar[] */
		this.scopedVarOnStart = [];
		/** Extension global ID to generate events on @type string*/
		this.globalID = ext.globalID;
		/** NULL is invalid! @type CRunDarkScript*/
		this.ext = ext;
		/** Function name to redirect to. Redirects do not combine. @type string */
		this.redirectFunc = "";
		/** @type DarkScript_FunctionTemplate | null */
		this.redirectFuncPtr = null;
		/** @type boolean */
		this.isAnonymous = false;
	};
};

/** @constructor */
class DarkScript_RunningFunction {
	/**
	 * 
	 * @param {DarkScript_FunctionTemplate} funcTemplate 
	 * @param {boolean} active 
	 * @param {number} numRepeats 
	 */
	constructor(funcTemplate, active, numRepeats) {
		/** @type DarkScript_FunctionTemplate*/
		this.funcTemplate = funcTemplate;
		/** If true, currently running; if false, waiting to be run, or aborting. @type boolean */
		this.active = active;
		/** Current abort reason. Can't double-abort. @type string */
		this.abortReason = "";
		/** If true, an On Function Aborted that matched this func name ran @type boolean */
		this.abortWasHandled = false;

		/** If true, an On Function that matched this func name ran @type boolean */
		this.eventWasHandled = false;

		/** For all functions: If true, current iteration's On Function will be triggered. @type boolean */
		this.currentIterationTriggering = true;
		/** For repeating functions: If true, next iteration of a repeating event will run. @type boolean */
		this.nextRepeatIterationTriggering = true;
		/** For foreach functions: If false, following foreach object will not be looped through. @type boolean */
		this.foreachTriggering = true;

		/** 0+: index of this function. Once index > numRepeats, function is removed. @type number */
		this.index = 0;
		/** 0 = no repeats, 1 run in total. @type number */
		this.numRepeats = numRepeats;

		/** If true, is a K-type of function, keeping object selection. @type boolean */
		this.keepObjectSelection = false;
		/** All selected objects from the starting function.
		 *  Note: in a foreach function, does NOT include the foreach object list. @type DarkScript_FusionSelectedObjectListCache[] */
		this.selectedObjects = [];
		/** Current object fixed value in a foreach @type number */
		this.currentForeachObjFV = 0;
		/** Current object OIL (including qualifier) @type number */
		this.currentForeachOil = -1;
		/** If true, return value does not need to be set, as it's being run inside an action
		 *  It still *can* be set, though. @type boolean */
		this.isVoidRun = false;

		/** Includes both passed and default parameters - see numPassedParams @type DarkScript_Value[] */
		this.paramValues = [];
		/** Number of parameters explicitly passed by user in expression @type number */
		this.numPassedParams = Number.MIN_VALUE;
		/** Current return value. If Any, is not set yet.  @type DarkScript_Value */
		this.returnValue = funcTemplate.defaultReturnValue;

		/** Fusion event #(num) or delayed event. @type string */
		this.runLocation = "";

		/** If non-empty, the original function name before it was redirected to this one @type string */
		this.redirectedFromFunctionName = "";

		for (let i = 0; i < funcTemplate.params.length; ++i) {
			this.paramValues.push(funcTemplate.params[i].defaultVal);
		}
	};
};

class DarkScript_DelayedFunction {
	/**
	 * 
	 * @param {DarkScript_RunningFunction} func Function to delay
	 * @param {number} startFrame Extension tick this was queued at (not queued for)
	 */
	constructor(func, startFrame) {
		/** Tick where this function was queued @type number */
		this.startFrame = startFrame;
		/** Units may be milliseconds or ticks; see useTicks @type number */
		this.numUnitsUntilRun = Number.MAX_VALUE;
		/** If true, numUnitsUntilRun is frame ticks, if false, it's milliseconds @type boolean */
		this.useTicks = false;
		/** If true, this delayed function remains in the queue and should trigger even if Fusion frame changes
		// (requires the new frame to have a DarkScript with a matching global ID) @type boolean */
		this.keepAcrossFrames = false;
		/** Number of times this delayed event ticks @type number */
		this.numRepeats = 0;
		/** Tick count until this is run (might be N/A, see useTicks) @type number */
		this.runAtTick = Number.MAX_VALUE;
		/** Time when this is run (might be N/A, see useTicks) @type Date */
		this.runAtTime = new Date(0);
		/** Function to run with delay @type DarkScript_RunningFunction */
		this.funcToRun = func;

		/** Fusion event #(num) or delayed event. @type string */
		this.fusionStartEvent = "";

		// Can't keep object selection in a delayed func
		if (func.keepObjectSelection) {
			console.warn("Warning: template was set to keep object selection in delayed function.");
		}
	}
};
class DarkScript_GlobalData {
	constructor () {
		/** Extensions using this GlobalData @type CRunDarkScript[] */
		this.exts = [];
		/** Templates, otherwise called declarations @type DarkScript_FunctionTemplate[] */
		this.functionTemplates = [];
		/** Functions delayed but will run later @type DarkScript_DelayedFunction[] */
		this.pendingFuncs = [];
		/** Functions that are running @type DarkScript_RunningFunction[] */
		this.runningFuncs = [];
		/** All scoped vars available at all levels @type DarkScript_ScopedVar[] */
		this.scopedVars = [];
		/** The curFrame number on frame end @type number */
		this.curFrameOnFrameEnd = 0;
		/** If runtime is paused, this is set to pause time @type Date */
		this.runtimePausedTime = new Date();
	}
}

class CRunDarkScript extends CRunExtension {
	/// <summary> Constructor of Fusion object. </summary>
	constructor() {
		super();

		// DarkEdif SDK exts should have these four variables defined.
		// We need this[] and globalThis[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
		this['ExtensionVersion'] = 3; // To match C++ version
		this['SDKVersion'] = 19; // To match C++ version
		this['DebugMode'] = true;
		this['ExtensionName'] = 'DarkScript';

		// Can't find DarkEdif wrapper
		if (!globalThis.hasOwnProperty('darkEdif')) {
			throw "a wobbly";
		}
		globalThis['darkEdif'].checkSupportsSDKVersion(this.SDKVersion);

		// These are properly inited later, in createRunObject

		/** Fusion frame number (1+) @type number | -1 */
		this.fusionFrameNum = -1;
		/** @type string */
		this.globalID = "<unset>";
		/** @type DarkScript_ConversionStrictness */
		this.conversionStrictness = DarkScript_ConversionStrictness.Exact;
		/** @type DarkScript_AllowNoReturnValue */
		this.whenAllowNoRVSet = DarkScript_AllowNoReturnValue.Never;
		
		/** If true, a function can't be run without a pre-existing template @type boolean */
		this.funcsMustHaveTemplate = false;
		/** If true, sends template list to new Fusion frames after frame switch.
			Requires global ID of receiving object to match. @type boolean */
		this.keepTemplatesAcrossFrames = false;
		/** If true, sends scoped vars that are global to new Fusion frames after frame switch.
			Requires global ID of receiving object to match. @type boolean */
		this.keepGlobalScopedVarsAcrossFrames = false;
		/** If true, creates error if no On Function event matches when a function is run
			If false, ignores it.
			On Any Function will also work. @type boolean */
		this.createErrorForUnhandledEvents = false;
		/** If true, creates error if no On Function Aborted event matches when a function is run
			If false, ignores it.
			On Any Function Aborted will also work. @type boolean */
		this.createErrorForUnhandledAborts = false;
		/** If true, enables On Any Function XX events. @type boolean */
		this.enableOnAnyFunctions = false;
		/** If true, when hiding scoped variables by virtue of a lower level scoped var having same name
			as a higher level, create an error. This is usually unintended behaviour. @type boolean */
		this.createErrorOverridingScopedVars = false;
		/** If true, prevents any recursive function calls. Recursion is usually a bad design choice,
			and 99% of recursive functions can be written without recursion.
			A noteable exception is deleting a folder tree; but even those have system commands or Sphax File-Folder. @type boolean */
		this.preventAllRecursion = false;
		/** If true, when a function runs on another ext, the aborts the function generates
			are run on the destination ext.
			If false, aborts/errors are run on the extension that starts the function (default).
			Errors are usually the fault of the caller, some syntax error - so they'll be wherever is local. @type boolean */
		this.runAbortsOnDestination = false;
		/** If true, allows a ForEach run on a qualifier to trigger for objects in qualifier, not just On Each qualifier. @type boolean */
		this.allowQualifierToTriggerSingularForeach = false;
		/** If true, allows a ForEach run on an object to trigger On Each Qualifier for qualifiers containing that object. @type boolean*/
		this.allowSingularToTriggerQualifierForeach = false;
		/** If true, parents' parameters are passed as scoped variables, but read-only. @type boolean */
		this.inheritParametersAsScopedVariables = false;

		// Function settings passed by action -> expression

		/** If more than 0, current function is being executed by delay action @type number */
		this.lastDelaySetting = 0;
		/** If true, all functions available are being triggered to analyse event numbers @type boolean */
		this.selfAwareness = false;
		/** Function set up, in case we're sending templates across frames @type DarkScript_GlobalData */
		this.globals = null;
		/** current DarkScript error @type string */
		this.curError = "";
		/** All the juicy details @type string */
		this.curLog = "";
		/** Internal loop name @type string */
		this.curLoopName = "";
		/** If calling subapp ext for running our expression, we generate errors from caller side,
			not from the subapp ext @type CRunDarkScript | null */
		this.errorExt = null;
		/** Last return value - useful @type DarkScript_Value */
		this.lastReturn = new DarkScript_Value(DarkScript_Type.Any);

		/** Current frame index (while functions pending, this number accumulates by 1 per event loop via Handle) @type Number */
		this.curFrame = Number.MIN_VALUE;

		/** @type */
		this.curDelayedFuncLoop = null;
		this.curDelayedFunc = null;
		this.curFuncTemplateLoop = null;
		this.curParamLoop = null;
		this.curScopedVarLoop = null;
		this.internalLoopIndex = Number.MIN_VALUE;

		/** @type {DarkScript_RunningFunction | null} */
		this.foreachFuncToRun = null;
		
		/** See numFuncVariantsToGenerateCF25 comments in original C++
			Number of parameters that expressions permit @constant @type Number */
		this.numParamsToAllow = 6;
		/** Result of numFuncVariantsToGenerateCF25 in C++ side; it's complicated math @constant @type Number */
		this.numFuncVariantsToGenerate = 13116;
		/** Last dummy expression ID @type Number*/
		this.lastNonFuncID = 59;
		/**	@type CRun */
		this.rhPtr = this.rhPtr || this.rh;
		/**	@type CRunExtension */
		this.rdPtr = this.rdPtr || this;
		/** @type {(number | string)[]} */
		this.actParameters = [];
		/**	@type {EdifRuntime} */
		this.Runtime = null;// new darkEdif.EdifRuntime(this);
	
		// =============================
		// Function arrays
		// =============================

		/** @type {Function[]} */
		this.$actionFuncs = [
			/* 0 */ this.Template_SetFuncSignature,
			/* 1 */ this.Template_SetDefaultReturnI,
			/* 2 */ this.Template_SetDefaultReturnF,
			/* 3 */ this.Template_SetDefaultReturnS,
			/* 4 */ this.Template_SetDefaultReturnN,
			/* 5 */ this.Template_Param_SetDefaultValueI,
			/* 6 */ this.Template_Param_SetDefaultValueF,
			/* 7 */ this.Template_Param_SetDefaultValueS,
			/* 8 */ this.Template_Param_SetDefaultValueN,
			/* 9 */ this.Template_SetScopedVarOnStartI ,
			/* 10 */ this.Template_SetScopedVarOnStartF,
			/* 11 */ this.Template_SetScopedVarOnStartS,
			/* 12 */ this.Template_CancelScopedVarOnStart,
			/* 13 */ this.Template_SetGlobalID,
			/* 14 */ this.Template_SetEnabled,
			/* 15 */ this.Template_RedirectFunction,
			/* 16 */ this.Template_Loop,
			/* 17 */ this.DelayedFunctions_Loop,
			/* 18 */ this.DelayedFunctions_CancelByPrefix,
			/* 19 */ this.RunFunction_ActionDummy_Num,
			/* 20 */ this.RunFunction_ActionDummy_String,
			/* 21 */ this.RunFunction_Foreach_Num,
			/* 22 */ this.RunFunction_Foreach_String,
			/* 23 */ this.RunFunction_Delayed_Num_MS,
			/* 24 */ this.RunFunction_Delayed_String_MS,
			/* 25 */ this.RunFunction_Delayed_Num_Ticks,
			/* 26 */ this.RunFunction_Delayed_String_Ticks,
			/* 27 */ this.RunFunction_Script,
			/* 28 */ this.RunningFunc_SetReturnI,
			/* 29 */ this.RunningFunc_SetReturnF,
			/* 30 */ this.RunningFunc_SetReturnS,
			/* 31 */ this.RunningFunc_ScopedVar_SetI,
			/* 32 */ this.RunningFunc_ScopedVar_SetF,
			/* 33 */ this.RunningFunc_ScopedVar_SetS,
			/* 34 */ this.RunningFunc_Params_Loop,
			/* 35 */ this.RunningFunc_ScopedVar_Loop,
			/* 36 */ this.RunningFunc_StopFunction,
			/* 37 */ this.RunningFunc_ChangeRepeatSetting,
			/* 38 */ this.RunningFunc_Abort,
			/* 39 */ this.Logging_SetLevel,
			/* 40 */ this.Template_ImportFromAnotherFrame,
		];
		/** @type {Function[]} */
		this.$conditionFuncs = [
			/* 0 */ this.AlwaysTrue, // OnDarkScriptError
			/* 1 */ this.OnFunction,
			/* 2 */ this.OnForeachFunction,
			/* 3 */ this.OnFunctionAborted,
			/* 4 */ this.IsRunningFuncStillActive,
			/* 5 */ this.DoesFunctionHaveTemplate,
			/* 6 */ this.IsFunctionInCallStack,
			/* 7 */ this.LoopNameMatch, // OnTemplateLoop
			/* 8 */ this.LoopNameMatch, // OnPendingFunctionLoop
			/* 9 */ this.LoopNameMatch, // OnParameterLoop
			/* 10 */ this.LoopNameMatch, // OnScopedVarLoop
			/* 11 */ this.Logging_OnAnyFunction,
			/* 12 */ this.Logging_OnAnyFunctionCompletedOK,
			/* 13 */ this.Logging_OnAnyFunctionAborted,
			/* 14 */ this.IsLastRepeatOfFunction,
		
			// update getNumOfConditions function if you edit this!!!!
		];
		/** @type {Function[]} */
		this.$expressionFuncs = [
			/* 0 */ this.Logging_GetDarkScriptError,
			/* 1 */ this.Logging_GetAbortReason,
			/* 2 */ this.Logging_GetCurrentLog,
		
			/* 3 */ this.RunningFunc_GetRepeatIndex,
			/* 4 */ this.RunningFunc_GetNumRepeatsLeft,
			/* 5 */ this.RunningFunc_GetNumRepeatsTotal,
			/* 6 */ this.RunningFunc_ForeachFV,
			/* 7 */ this.RunningFunc_NumParamsPassed,
			/* 8 */ this.RunningFunc_ScopedVar_GetI,
			/* 9 */ this.RunningFunc_ScopedVar_GetF,
			/* 10 */ this.RunningFunc_ScopedVar_GetS,
			/* 11 */ this.RunningFunc_GetParamValueByIndexI,
			/* 12 */ this.RunningFunc_GetParamValueByIndexF,
			/* 13 */ this.RunningFunc_GetParamValueByIndexS,
			/* 14 */ this.RunningFunc_GetAllParamsAsText,
			/* 15 */ this.RunningFunc_GetCallStack,
		
			/* 16 */ this.InternalLoop_GetIndex,
			/* 17 */ this.InternalLoop_GetVarName,
			/* 18 */ this.InternalLoop_GetVarType,
		
			/* 19 */ this.FuncTemplate_GetFunctionName,
			/* 20 */ this.FuncTemplate_GetNumRequiredParams,
			/* 21 */ this.FuncTemplate_GetNumPossibleParams,
			/* 22 */ this.FuncTemplate_GetReturnType,
			/* 23 */ this.FuncTemplate_ShouldRepeat,
			/* 24 */ this.FuncTemplate_ShouldBeDelayed,
			/* 25 */ this.FuncTemplate_RecursionAllowed,
			/* 26 */ this.FuncTemplate_IsEnabled,
			/* 27 */ this.FuncTemplate_GetRedirectFuncName,
			/* 28 */ this.FuncTemplate_GlobalID,
			/* 29 */ this.FuncTemplate_ParamNameByIndex,
			/* 30 */ this.FuncTemplate_ParamTypeByIndex,
			/* 31 */ this.FuncTemplate_ParamDefaultValByIndex,
			/* 32 */ this.FuncTemplate_ParamIndexByName,
			/* 33 */ this.FuncTemplate_ParamTypeByName,
			/* 34 */ this.FuncTemplate_ParamDefaultValByName,
		
			/* 35 */ this.LastReturn_AsInt,
			/* 36 */ this.LastReturn_AsFloat,
			/* 37 */ this.LastReturn_AsString,
			/* 38 */ this.LastReturn_Type,
			/* 39 */ this.RunningFunc_GetCalledFuncName,
			/* 40 */ this.TestFunc,
			// Dummies until ID 59; 60 is first non-dummy generated expression
		];
		
		/** @type {DarkScript_ExpInfo[]} */
		this.ExpInfos = [];
		this.AutoGenerateExpressions();
			
		/// <summary> Prototype definition </summary>
		/// <description> This class is a sub-class of CRunExtension, by the mean of the
		/// CServices.extend function which copies all the properties of
		/// the parent class to the new class when it is created.
		/// As all the necessary functions are defined in the parent class,
		/// you only need to keep the ones that you actually need in your code. </description>
		this.getNumberOfConditions = function() {
			/// <summary> Returns the number of conditions </summary>
			/// <returns type="Number" isInteger="true"> Warning, if this number is not correct, the application _will_ crash</returns>
			return this.$conditionFuncs.length;
		};
		this.createRunObject = function(file, cob, version) {
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

			// Used to make sure if we're doing a foreach, make sure we don't call a DarkScript function on another frame
			this.fusionFrameNum = this.Runtime.GetCurrentFusionFrameNumber();

			// Copy all object properties from EDITDATA to Extension
			this.globalID = props['GetPropertyStr']('Global ID');
			let strictProp = props['GetPropertyStr']('Conversion strictness level');
			this.conversionStrictness = 
				strictProp == "Exact" ? DarkScript_ConversionStrictness.Exact :
				strictProp == "Float <-> Integer OK" ? DarkScript_ConversionStrictness.Numeric :
				strictProp == "Any conversion (e.g. \"1\" <-> 1.0)" ? DarkScript_ConversionStrictness.AnyWithAborts :
				strictProp == "Any (no errors)" ? DarkScript_ConversionStrictness.AnyWithDefaults : -1;
			if (this.conversionStrictness == -1) {
				darkEdif.consoleLog(this, `Unrecognised conversion strictness property value "${strictProp}".`);
			}
			
			this.funcsMustHaveTemplate = props['IsPropChecked']("Require declaration for all functions");
			this.keepTemplatesAcrossFrames = props['IsPropChecked']("Keep declarations across frame switches");
			this.keepGlobalScopedVarsAcrossFrames = props['IsPropChecked']("Keep global vars across frame switches");
			this.createErrorForUnhandledEvents = props['IsPropChecked']("Create error for unhandled functions");
			this.createErrorForUnhandledAborts = props['IsPropChecked']("Create error for unhandled aborts");
			this.enableOnAnyFunctions = props['IsPropChecked']("Enable Any Function conditions");
			this.createErrorOverridingScopedVars = props['IsPropChecked']("Create an error when overriding scoped values");
			this.preventAllRecursion = props['IsPropChecked']("Prevent any recursion");
			this.runAbortsOnDestination = props['IsPropChecked']("Run errors/aborts on called extension");
			this.allowQualifierToTriggerSingularForeach = props['IsPropChecked']("Allow qualifier objects to trigger singular foreach");
			this.allowSingularToTriggerQualifierForeach = props['IsPropChecked']("Allow singular objects to trigger qualifier foreach");
			this.inheritParametersAsScopedVariables = props['IsPropChecked']("Inherit parameters as scoped variables");

			let allowNoReturnProp = props['GetPropertyStr']("Allow no return value when");
			this.whenAllowNoRVSet =
				allowNoReturnProp == "Never" ? DarkScript_AllowNoReturnValue.Never :
				allowNoReturnProp == "Foreach, delayed only" ? DarkScript_AllowNoReturnValue.ForeachDelayedActionsOnly :
				allowNoReturnProp == "Anonymous, foreach, delayed only" ? DarkScript_AllowNoReturnValue.AnonymousForeachDelayedActions :
				allowNoReturnProp == "All functions" ? DarkScript_AllowNoReturnValue.AllFunctions : -1;
			if (this.whenAllowNoRVSet == -1) {
				darkEdif.consoleLog(this, `Unrecognised allow no return value property value "${allowNoReturnProp}".`);
			}

			// Strange combo; we don't make an outright error though, because we can still use the value
			if (this.funcsMustHaveTemplate && this.whenAllowNoRVSet == DarkScript_AllowNoReturnValue.AnonymousForeachDelayedActions) {
				darkEdif.consoleLog(this, `Warning: "Allow no return value set" property is set to "${allowNoReturnProp}", but allowing anonymous functions are turned off.`);
			}

			const globalScoped = props['GetPropertyStr']("Global scoped vars");
			// TODO: Loop global scoped ^ and insert
			let gc = null;
			if (this.globalID != "") {
				gc = darkEdif.getGlobalData("DarkScript" + this.globalID);
			}
			if (gc == null) {
				this.globals = new DarkScript_GlobalData();
				if (this.globalID != "") {
					darkEdif.setGlobalData("DarkScript" + this.globalID, this.globals);
				}
			}
			else {
				this.globals = gc;
				this.curFrame = this.globals.curFrameOnFrameEnd;
			}
			this.globals.exts.push(this);
			this.selfAwareness = true;

			// The return value is not used in this version of the runtime: always return false.
			return false;
		};

		this.handleRunObject = function() {
			/// <summary> This function is called at every loop of the game. You have to perform
			/// in it all the tasks necessary for your object to function. </summary>
			/// <returns type="Number"> One of two options:
			///							   0 : this function will be called during the next loop
			/// CRunExtension.REFLAG_ONESHOT : this function will not be called anymore,
			///								   unless this.reHandle() is called. </returns>
			++this.curFrame;
			const nowMS = Date.now();
			const now = new Date();
		
			for (let i = 0; i < this.globals.pendingFuncs.length; ++i) {
				if (this.globals.pendingFuncs[i].useTicks ? 
					this.globals.pendingFuncs[i].runAtTick <= this.curFrame : 
					this.globals.pendingFuncs[i].runAtTime < now) {
					let pf = this.globals.pendingFuncs[i];
		
					if (pf.useTicks && pf.startFrame < this.curFrame) {
						this.CreateError("Warning: delayed function \"%s\" started too late (%i < %i).", 
						pf.funcToRun.funcTemplate.name, pf.startFrame, this.curFrame);
					}
		
					this.curDelayedFunc = pf;
					pf.funcToRun.runLocation = this.Sub_GetLocation(23);
					pf.funcToRun.isVoidRun = true;
					this.ExecuteFunction(null, pf.funcToRun);
					pf.funcToRun.runLocation = "";
					if (--pf.numRepeats < 0 || !pf.funcToRun.active) {
						darkEdif.consoleLog(this, `Info: Delayed function ${pf.funcToRun.funcTemplate.name} was dequeued (num repeats = ${pf.numRepeats}, abort reason: "${pf.funcToRun.abortReason}".`);
						this.globals.pendingFuncs.splice(i, 1);
						--i;
					}
					else {
						if (pf.useTicks) {
							pf.runAtTick = pf.numUnitsUntilRun;
						}
						else {
							pf.runAtTime = new Date(now.getUTCMilliseconds() + pf.numUnitsUntilRun);
						}
					}
					this.curDelayedFunc = null;
				}
			}
		
			return this.globals.pendingFuncs.length == 0 ? CRunExtension.REFLAG_ONESHOT : 0;
		};

		this.destroyRunObject = function(bFast) {
			/// <summary> Destruction of the object.</summary>
			/// <description> Called when the object is actually destroyed. This will always be
			/// after the main game loop, and out of the actions processing : the
			/// destroy process is queued until the very end of the game loop. </description>
			/// <param name="bFast" type="Boolean"> Fast close. true if quickly clearing
			///	all frame objects at end of frame/app, otherwise false. </param>

			this.isDestroyed = true;
			darkEdif.consoleLog(this, "Was destroyed!");

			for (let i = 0; i < this.globals.exts.length; ++i) {
				if (this.globals.exts[i] == this) {
					this.globals.exts.splice(i, 1);
					break;
				}
			}
			if (this.globals.exts.length != 0) {
				return; // only last ext cleans up
			}

			// not a shared object; we can clear everything
			if (this.globalID.length == 0) {
				this.globals = null;
				return;
			}
		
			// should this ever be true
			if (this.globals.runningFuncs.length != 0) {
				darkEdif.consoleLog(this, "Unexpectedly, functions were still executing during destroy.");
			}
		
			this.globals.pendingFuncs = this.globals.pendingFuncs.filter(pf => !pf.keepAcrossFrames);
		
			// Check function declaration should be cleared on end
			if (!this.keepTemplatesAcrossFrames) {
				this.globals.functionTemplates.length = 0;
			}
			if (!this.keepGlobalScopedVarsAcrossFrames) {
				this.globals.scopedVars.length = 0;
			}
			this.globals.curFrameOnFrameEnd = this.curFrame;
		};
		this.pauseRunObject = function() {
			if (this.globals.exts[0] == this && this.globals.pendingFuncs.length != 0) {
				this.globals.runtimePausedTime = new Date();
			}
		};

		this.continueRunObject = function() {
			if (this.globals.exts[0] == this && this.globals.pendingFuncs.length != 0) {
				const diffMS = new Date().getTime() - this.globals.runtimePausedTime.getTime();
				this.globals.pendingFuncs.forEach(f => {
					if (!f.useTicks) {
						f.runAtTime.setUTCMilliseconds(f.runAtTime.getUTCMilliseconds() + diffMS);
					}
				});
			}
			return 0;
		};
		this.condition = function(num, cnd) {
			this.Runtime.curCEvent = cnd;
			/// <summary> Called when a condition of this object is tested. </summary>
			/// <param name="num" type="Number" integer="true"> The number of the condition; 0+. </param>
			/// <param name="cnd" type="CCndExtension"> a CCndExtension object, allowing you to retreive the parameters
			//			of the condition. </param>
			/// <returns type="Boolean"> True if the condition is currently true. </returns>

			const func = this.$conditionFuncs[~~num];
			if (func == null) {
				throw "Unrecognised condition ID " + (~~num) + " passed to DarkScript.";
			}

			// Note: New Direction parameter is not supported by this, add a workaround based on condition and parameter index;
			// SDL Joystick's source has an example.
			const args = new Array(func.length);
			for (let i = 0; i < args.length; ++i) {
				if (func == this.OnForeachFunction && i == 1) {
					args[i] = cnd.evtParams[i].oiList;
					continue;
				}
				args[i] = cnd.getParamExpString(this.rh, i);
			}

			return func.apply(this, args);
		};
		this.action = function(num, act) {
			this.Runtime.curCEvent = act;
			/// <summary> Called when an action of this object is executed </summary>
			/// <param name="num" type="Number"> The ID/number of the action, as defined by
			///		its array index. </param>
			/// <param name="act" type="CActExtension"> A CActExtension object, allowing you to
			///		retrieve the parameters of the action </param>

			const func = this.$actionFuncs[~~num];
			if (func == null) {
				throw "Unrecognised action ID " + (~~num) + " passed to DarkScript.";
			}

			// Note: New Direction parameter is not supported by this, add a workaround based on action and parameter index;
			// SDL Joystick's source has an example.
			const args = new Array(func.length);
			// So DarkScript expression can read preceding action args
			this.actParameters = args;
			for (let i = 0; i < args.length; ++i) {
				// Get object specially
				if (i == 0 && (func == this.RunFunction_Foreach_Num || func == this.RunFunction_Foreach_String)) {
					// In Windows ver, this direct param was ignored and looked up later
					args[i] = act.evtParams[i].oiList;
					continue;
				}
				args[i] = act.getParamExpression(this.rh, i);
			}

			func.apply(this, args);
		};
		this.expression = function(num) {
			/// <summary> Called during the evaluation of an expression. </summary>
			/// <param name="num" type="Number"> The ID/number of the expression. </param>
			/// <returns> The result of the calculation, a number or a string </returns>

			// Note that it is important that your expression function asks for
			// each and every one of the parameters of the function, each time it is
			// called. The runtime will crash if you miss parameters.

			num = ~~num; // to int
			let func = this.$expressionFuncs[num];
			if (func == null) {
				// TODO: Confirm max range here is correct
				if (num > this.lastNonFuncID && num - this.lastNonFuncID <= this.numFuncVariantsToGenerate) {
					func = this.VariableFunction;
				}
				else {
					throw "Unrecognised expression ID " + num + " passed to DarkScript.";
				}
			}

			const numArgs = func != this.VariableFunction ? 
				func.length :
				this.ExpInfos[num].NumOfParams;

			const args = new Array(numArgs);
			for (let i = 0; i < args.length; ++i) {
				args[i] = this.ho.getExpParam();
			}
			if (func == this.VariableFunction) {
				return this.VariableFunction.call(this, args[0], this.ExpInfos[num], args);
			}

			return func.apply(this, args);
		};

	};
	AutoGenerateExpressions() {
		if (this.ExpInfos && this.ExpInfos.length != 0) {
			return;
		}
		this.ExpInfos = new Array(this.lastNonFuncID + 1);

		const GenerateExpressionFuncFor = function(inputID) { 
			const NumVariants = DarkScript_Flags.Both + 1;
			let funcID = inputID;
			let flags = (funcID & DarkScript_Flags.Both);
			funcID = (funcID >> 2); // remove last two bits (Flags) and shift remaining down in their place

			//for (int i = 0; i < 30; i++)
			//	console.info("Anticipating that for function ID %i, %i is the template.", i, (i - 4 - (i % 4)));

			let last;
			// Get last function before this template, with None variant.
			// For example, FuncS should look up FuncI.
			if (inputID >= NumVariants) {
				last = this.ExpInfos[this.lastNonFuncID + 1 + (inputID - NumVariants - (inputID % NumVariants))];
			}
			else { // We're on the first set of functions
				last = new DarkScript_ExpInfo();
				last.ID = this.lastNonFuncID;
				last.FloatFlags = 0;
				last.Flags = DarkScript_ExpReturnType.Integer;
				// First parameter of expression function is the function name
				last.NumOfParams = 1;
				last.Parameter[0] = DarkScript_ExpParams.String;
			}

			const GetParam = function(id) {
				console.assert(id < last.NumOfParams);
				if (last.Parameter[id] == DarkScript_ExpParams.String) {
					return DarkScript_Type.String;
				}
				else if (last.Parameter[id] == DarkScript_ExpParams.Integer) {
					return last.FloatFlags & (1 << id) ? DarkScript_Type.Float : DarkScript_Type.Integer;
				}
				else {
					throw new Error("Parameter type not recognised");
					return DarkScript_Type.Any;
				}
			};

			let needNewParam = false;
			// If we're progressing to add a new parameter, we're on last variant, and highest value ExpReturnType, float,
			// and all parameters are string
			if (last.Flags == DarkScript_ExpReturnType.Float) {
				needNewParam = true;
				// Skip parameter 0 (function ID) - bear in mind last is None variant, so no repeat count
				for (let i = 1; i < last.NumOfParams; ++i) {
					if (GetParam(i) != DarkScript_Type.Float) {
						needNewParam = false;
						break;
					}
				}
			}

			// Fourth, what remains is an index with parameters being passed.
			// Don't forget to take into account repeat count.

			let paramTypes = [];
			paramTypes.push(DarkScript_Type.String); // function name
			if (flags & DarkScript_Flags.Repeat) {
				paramTypes.push(DarkScript_Type.Integer); // number of repeats
			}

			// Get return type by modulus num return types
			const returnType = ((funcID % DarkScript_Type.NumCallableReturnTypes) + 1);

			// If we're adding a new parameter, we start with int-returning funcs
			console.assert(!needNewParam || returnType == DarkScript_Type.Integer);

			let intermediate = Math.floor(funcID / DarkScript_Type.NumCallableReturnTypes) - 1;
			for (let i = paramTypes.length; i < (last.NumOfParams + (needNewParam ? 1 : 0) + (flags & DarkScript_Flags.Repeat ? 1 : 0)); i++) {
				// too many params?
				if (intermediate < 0) {
					debugger;
				}
				// While there's 4 valid parameter types (Any included),
				// the function can only be called with 3.
				paramTypes.push((Math.floor(intermediate % DarkScript_Type.NumCallableParamTypes) + 1));

				if (paramTypes[i] < DarkScript_Type.Integer) {
					// miscalculated type
					debugger;
					break;
				}
				intermediate = Math.floor(intermediate / DarkScript_Type.NumCallableParamTypes) - 1;
			}

			if (intermediate > 0) {
				debugger; // missed a param?
			}

			// Parameters checked; allocate new info
			let ExpInfo = new DarkScript_ExpInfo(paramTypes.length);

			// If a non-triggered condition, set the correct flags
			ExpInfo.ID = inputID + this.lastNonFuncID + 1;
			ExpInfo.NumOfParams = paramTypes.length;

			let str = "";
			switch (returnType) {
			case DarkScript_Type.Integer:
				ExpInfo.Flags = DarkScript_ExpReturnType.Integer;
				str += "int";
				break;
			case DarkScript_Type.Float:
				ExpInfo.Flags = DarkScript_ExpReturnType.Float;
				str += "float";
				break;
			case DarkScript_Type.String:
				ExpInfo.Flags = DarkScript_ExpReturnType.String;
				str += "string";
				break;
			default:
				throw new Error(`Can't use return type ${returnType}, not set up`);
			}

			let str2 = "";

			str2 +=
				(flags & DarkScript_Flags.KeepObjSelection ? "K" : "") +
				(flags & DarkScript_Flags.Repeat ? "R" : "") +
				(ExpInfo.Flags == DarkScript_ExpReturnType.Float ? "F" : "") +
				"Func";

			str += " " +
				(flags & DarkScript_Flags.KeepObjSelection ? "k" : "") +
				(flags & DarkScript_Flags.Repeat ? "r" : "") +
				"func_" + inputID + '(';

			str += "string_funcname, ";
			ExpInfo.Parameter[0] = DarkScript_ExpParams.String;

			// Set up each parameter (there's 1+, since func name is there)
			for (let c = 1, d = 1; c < paramTypes.length; ++c, ++d) {
				if (c == 1 && (flags & DarkScript_Flags.Repeat)) {
					str += "int_repeatcount, ";
					ExpInfo.Parameter[1] = DarkScript_ExpParams.Integer;
					--d;
					continue;
				}
				switch (paramTypes[c]) {
				case DarkScript_Type.Integer:
					ExpInfo.Parameter[c] = DarkScript_ExpParams.Integer;
					str += `int_${d}, `;
					str2 += 'I';
					break;
				case DarkScript_Type.Float:
					ExpInfo.Parameter[c] = DarkScript_ExpParams.Float;
					ExpInfo.FloatFlags |= (1 << c);
					str += `float_${d}, `;
					str2 += 'F';
					break;
				case DarkScript_Type.String:
					ExpInfo.Parameter[c] = DarkScript_ExpParams.String;
					str += `string_${d}, `;
					str2 += 'S';
					break;
				default:
					throw new Error(`Can't use parameter type ${paramTypes[c]}, not set up.`);
				}
			}

			let ret = "";
			if (this['DebugMode']) {
				ret = str;
				str = ret.substring(0, ret.length - 2) + ") [ID " + ExpInfo.ID + ", param count " + ExpInfo.NumOfParams + "]\n";
				ret = str;

				str2 += (ExpInfo.Flags == DarkScript_ExpReturnType.String ? "$" : "") + 
					"() [expression ID " + ExpInfo.ID + ", " + ExpInfo.ID + "].";
				ret = str2;
				console.debug(ret);
			}

			// Add to table
			this.ExpInfos.push(ExpInfo);
			//this.$expressionFuncs[inputID + this.lastNonDummyFunc + 1] = this.VariableFunction;

			if (false) { // (inputID % 4 == 0)
				console.info("Outputted dynamic function, expression ID %i, funcID %i.\n%s", ExpInfo.ID, inputID, ret);
			}
		};
		for (let i = 0; i < this.numFuncVariantsToGenerate; ++i) {
			GenerateExpressionFuncFor.call(this, i); 
		}
	};
	/** @returns string | number */
	VariableFunction(funcName, exp, args) {
		let funcTemplate;

		// otherwise not variable func
		console.assert(exp.ID > this.lastNonFuncID && exp.ID <= this.lastNonFuncID + this.numFuncVariantsToGenerate);
		let callingAction = this.rhPtr.GetRH4ActionStart();
		let actID = callingAction != null ? callingAction.get_evtNum() - 80 : -1;
		let runImmediately = true, isVoidRun = false, isDelayed = false;
		// We don't run now if we have the Foreach or Delayed action being executed.
		// Instead, parameters are collected here, then when the Action func runs, it triggers On Function.
		if (actID >= 19 && actID <= 26 && callingAction.get_evtOi() == this.rdPtr.get_rHo().get_Oi())
		{
			// dummy func: run immediately, but don't reset actID, we use it later in Sub_GetLocation()
			if (actID >= 19 && actID <= 20) {
				console.info(`Called for Dummy action! Action ID is ${actID}.`);
			}
			else {
				runImmediately = false;
				isDelayed = actID >= 23; // foreach is 21, 22, delayed 23-26
				console.info(`Called for Foreach or Delayed action! Action ID is ${actID}; qual act cause singl cond = ${this.allowQualifierToTriggerSingularForeach ? "yes" : "no"}, singl act cause qual cond = ${this.allowSingularToTriggerQualifierForeach ? "yes" : "no"}.`);
			}
			isVoidRun = true;
		}
		else {
			console.info(`NOT called from Foreach/Delayed action! Oi is ${callingAction ? callingAction.evtOi : -1}, action ID is ${actID}.`);
			actID = -1;
		}

		let funcID = exp.ID - this.lastNonFuncID - 1;
		let nameL = this.ToLower(funcName), redirectedFromName = "";

		let funcTemplateIt = this.globals.functionTemplates.find((f) => { return f.nameL == nameL; });

		// Handles redirection to another function
		if (funcTemplateIt != null && funcTemplateIt.redirectFunc.length != 0)
		{
			console.info(`Redirecting from function "${funcName}" to "${funcTemplateIt.redirectFunc}".`);
			let funcTemplateIt2 = this.globals.functionTemplates.findIndex(f => f.redirectFuncPtr);

			// To redirect, the function must exist as a template, even with anonymous functions allowed. So if it doesn't, that's a pretty big problem
			if (funcTemplateIt2 == -1 /* && funcsMustHaveTemplate */)
			{
				this.CreateError(`Can't call function "${funcName}"; was redirected to function "${funcTemplateIt.redirectFunc}", which was defined when redirection was set, but does not exist in templates now. ` +
					`Returning the default return value of "${funcName}".`);
				this.lastReturn = funcTemplateIt.defaultReturnValue;
				return this.lastReturn.type == DarkScript_Type.String ? (this.lastReturn.data ? this.lastReturn.data : "") : this.lastReturn.data;
			}
			redirectedFromName = funcName;
			funcName = this.globals.functionTemplates[funcTemplateIt2].name;
			nameL = this.globals.functionTemplates[funcTemplateIt2].nameL;
			funcTemplateIt = this.globals.functionTemplates[funcTemplateIt2];
		}

		let expParamIndex = 1; // skip func name (index 0), we already read it
		// If 0, then function does not run
		let repeatTimes = 1;
		let ReadNextArgAs = function(t) {
			if (t == DarkScript_Type.String)
				return args[expParamIndex];
			if (t == DarkScript_Type.Float)
				return parseFloat(args[expParamIndex]);
			if (t == DarkScript_Type.Integer)
				return parseInt(args[expParamIndex]);
			throw Error("Unexpected argument");
		};

		if (funcID & DarkScript_Flags.Repeat) {
			repeatTimes = ReadNextArgAs(DarkScript_Type.Integer);
			if (repeatTimes < 0) {
				this.CreateError(`Can't call function ${funcName} with a negative number of repeats (you supplied ${repeatTimes} repeats).`);
				if (funcTemplateIt == null) {
					this.lastReturn = new DarkScript_Value(exp.Flags + 1);
				}
				else {
					this.lastReturn = funcTemplateIt.defaultReturnValue;
				}
				return this.lastReturn.type == DarkScript_Type.String ? (this.lastReturn.data ? this.lastReturn.data : "") : this.lastReturn.data;
			}
		}
		let DummyReturn = exp.Flags == DarkScript_ExpReturnType.String ? "" : 0;

		// Switch from returning dummy to returning whatever's in lastReturn
		let CreateError2V = function(a, ...b)
		{
			this.CreateError(a, b);
			if (runImmediately)
				return this.lastReturn.data;
			return DummyReturn;
		};

		// Function template doesn't exist; generate a temporary one
		if (funcTemplateIt == null)
		{
			this.lastReturn = new DarkScript_Value(exp.Flags + 1);
			if (this.funcsMustHaveTemplate)
				return CreateError2V(`Function with name "${funcName}" not found.`);
			if (repeatTimes == 0)
				return this.lastReturn.data;

			console.warn(`Generating anonymous function "${funcName}".`);
			funcTemplate = new DarkScript_FunctionTemplate(this, funcName, DarkScript_Expected.Either, DarkScript_Expected.Either, true, exp.Flags + 1);
			let name = "a";
			for (let i = expParamIndex; i < exp.NumOfParams; i++, name = String.fromCharCode(name.charCodeAt(0) + 1))
			{
				// Note the ++name[0] in for(;;><), gives variable names a, b, c
				let type = exp.Parameter[i] == DarkScript_ExpParams.String ? DarkScript_Type.String :
					exp.Parameter[i] == DarkScript_ExpParams.Integer ?
					(exp.FloatFlags & (1 << i) ? DarkScript_Type.Float : DarkScript_Type.Integer) : DarkScript_Type.Any;
				funcTemplate.params.push(new DarkScript_Param(name, type));
			}
			funcTemplate.isAnonymous = true;
		}
		else {
			funcTemplate = funcTemplateIt;
			if (runImmediately)
				this.lastReturn = funcTemplate.defaultReturnValue;

			if ((funcID & DarkScript_Flags.Repeat) == DarkScript_Flags.Repeat && funcTemplate.repeating == DarkScript_Expected.Never) {
				return CreateError2V(`Can't call function ${funcName} on repeat, template does not allow repeating.`);
			}
			if ((funcID & DarkScript_Flags.Repeat) == DarkScript_Flags.None && funcTemplate.repeating == DarkScript_Expected.Always) {
				return CreateError2V(`Can't call function ${funcName} without repeating, template expects repeating.`);
			}

			if (!isDelayed && funcTemplate.delaying == DarkScript_Expected.Never) {
				return CreateError2V(`Can't call function ${funcName} delayed, template does not allow delaying.`);
			}
			if (isDelayed && funcTemplate.delaying == DarkScript_Expected.Always) {
				return CreateError2V(`Can't call function ${funcName} without delaying, template expects delaying.`);
			}

			if ((!funcTemplate.recursiveAllowed || this.preventAllRecursion) &&
				this.globals.runningFuncs.find(rf => { return rf.funcTemplate == funcTemplate; }) != null) {
				return CreateError2V(`Can't call function ${funcName} recursively, ${this.preventAllRecursion ? "recursion has been prevented globally" :"template does not allow recursing"}.`);
			}

			// Too many parameters (ignore func name and repeat count)
			if ((exp.NumOfParams - expParamIndex) > funcTemplate.params.length) {
				return CreateError2V(`Can't call function ${funcTemplate.name} with ${exp.NumOfParams} parameters, template expects up to ${funcTemplate.params} parameters.`);
			}
		}


		// Running while current func is aborting - that's confusing behaviour
		if (this.globals.runningFuncs.length != 0 && !this.globals.runningFuncs[this.globals.runningFuncs.length - 1].active) {
			return CreateError2V(`Can't run function "${funcName}", current function "${this.globals.runningFuncs[this.globals.runningFuncs.length - 1].funcTemplate.name}" is aborted. Abort must be handled before functions can be run.`);
		}

		// User requested a looping function to run 0 times - return default value from function
		if (repeatTimes == 0) {
			if (runImmediately) {
				return this.lastReturn.data;
			}
			return DummyReturn;
		}

		let newFunc = new DarkScript_RunningFunction(funcTemplate, runImmediately, repeatTimes - 1);
		newFunc.keepObjectSelection = (funcID & DarkScript_Flags.KeepObjSelection) != 0;
		newFunc.isVoidRun = isVoidRun;
		newFunc.redirectedFromFunctionName = redirectedFromName;

		let numNotInParamsVector = expParamIndex;
		let numPassedExpFuncParams = exp.NumOfParams - numNotInParamsVector; // ignore func name and num repeats

		newFunc.numPassedParams = numPassedExpFuncParams;

		// User passed too many parameters
		if (funcTemplate.params.length < numPassedExpFuncParams)
			return CreateError2V(`${funcName}: called with ${numPassedExpFuncParams} parameters, but only expects ${funcTemplate.params.length}.`);

		// don't confuse expParamIndex (A/C/E, will include funcName, may include numRepeats)
		// vs. paramIndex (index of paramValues)
		let paramIndex = 0;

		for (; paramIndex < numPassedExpFuncParams; paramIndex++)
		{
			let paramTypeInTemplate = newFunc.funcTemplate.params[paramIndex].type;

			// Param type has default string, free it so we can replace with user-supplied param

			switch (exp.Parameter[expParamIndex])
			{
			case DarkScript_ExpParams.Float: // also integer
				if ((exp.FloatFlags & (1 << expParamIndex)) != 0) // float passed
				{
					if (paramTypeInTemplate == DarkScript_Type.Float || paramTypeInTemplate == DarkScript_Type.Any)
					{
						newFunc.paramValues[paramIndex].type = DarkScript_Type.Float;
						newFunc.paramValues[paramIndex].dataSize = 4; // sizeof(float)
						newFunc.paramValues[paramIndex].data = ReadNextArgAs(DarkScript_Type.Float);
					}
					else {
						return CreateError2V("Function %s: Parameter %zu should have been an %s, but was called with a %s instead.",
							funcName, paramIndex, this.TypeToString(paramTypeInTemplate), "float");
					}
				}
				else { // integer passed
					if (paramTypeInTemplate == DarkScript_Type.Integer || paramTypeInTemplate == DarkScript_Type.Any) {
						newFunc.paramValues[paramIndex].type = DarkScript_Type.Integer;
						newFunc.paramValues[paramIndex].dataSize = 4; // sizeof int
						newFunc.paramValues[paramIndex].data = ReadNextArgAs(DarkScript_Type.Integer);
					}
					else {
						return CreateError2V("Function %s: Parameter %zu should have been an %s, but was called with a %s instead.",
							funcName, paramIndex, this.TypeToString(paramTypeInTemplate), "integer");
					}
				}
				break;
			case DarkScript_ExpParams.String:
				if (paramTypeInTemplate == DarkScript_Type.String || paramTypeInTemplate == DarkScript_Type.Any)
				{
					newFunc.paramValues[paramIndex].type = DarkScript_Type.String;
					// pre-freed earlier
					const param = ReadNextArgAs(DarkScript_Type.String);
					newFunc.paramValues[paramIndex].dataSize = param.length;
					newFunc.paramValues[paramIndex].data = param;
				}
				else
				{
					return CreateError2V("Function %s: Parameter %zu should have been an %s, but was called with a %s instead.",
						funcName, paramIndex, this.TypeToString(paramTypeInTemplate), "string");
				}
				break;
			default:
				console.error(`DarkScript error: Param type for param index ${paramIndex} not recognised for function ID ` +
					`${exp.ID - this.lastNonFuncID - 1} (expr ID ${exp.ID}).`);
				return DummyReturn;
			}
		}

		// User didn't pass these, but template expects them; load from param defaults
		for (; paramIndex < funcTemplate.params.length; paramIndex++)
		{
			// No default, and user didn't pass one: no good.
			if (funcTemplate.params[paramIndex].type != DarkScript_Type.Any &&
				funcTemplate.params[paramIndex].defaultVal.type == DarkScript_Type.Any) {
				return CreateError2V(`${funcName}: parameter ${funcTemplate.params[paramIndex].name} (index ${paramIndex}) was not passed, but must be supplied.`);
			}
			else {
				newFunc.paramValues[paramIndex] = newFunc.funcTemplate.params[paramIndex].defaultVal;
			}
		}

		if (newFunc.active)
		{
			newFunc.runLocation = this.Sub_GetLocation(actID);
			let l = this.ExecuteFunction(null, newFunc);
			newFunc.runLocation = "";
			return l;
		}

		// else Foreach or Delayed func
		// ForEach
		if (this.$actionFuncs[actID] == this.RunFunction_Foreach_Num ||
			this.$actionFuncs[actID] == this.RunFunction_Foreach_String)
		{
			if (this.foreachFuncToRun != null)
				console.error(`Possible crash! Foreach function is already set, but is being set again. Original foreach: "${this.foreachFuncToRun.funcTemplate.name}", current foreach: "${newFunc.funcTemplate.name}".`);

			// Can't run a foreach on another frame; how could we select?
			if (funcTemplate.ext != this && funcTemplate.ext.fusionFrameNum != this.fusionFrameNum) {
				return CreateError2V(`Can't run a foreach loop on function ${funcName}, its events are in DarkScript global ID ${funcTemplate.ext.globalID}, in another frame.`);
			}

			this.foreachFuncToRun = newFunc;
			newFunc.isVoidRun = true;
			newFunc.runLocation = this.Sub_GetLocation(actID);
			// actParameters[1] is being calculated right now by this call of VariableFunc.

		}
		// Delayed func
		else if (this.$actionFuncs[actID] == this.RunFunction_Delayed_Num_MS ||
			this.$actionFuncs[actID] == this.RunFunction_Delayed_String_MS ||
			this.$actionFuncs[actID] == this.RunFunction_Delayed_Num_Ticks ||
			this.$actionFuncs[actID] == this.RunFunction_Delayed_String_Ticks)
		{
			let firstRunDelayedFor = this.actParameters[0];
			let numRepeats = this.actParameters[1];
			let delayBetweenRuns = this.actParameters[2];
			let keepIfFusionFrameChanged = this.actParameters[3];
			// actParameters[4] is being calculated right now by this call of VariableFunc().

			let delayFunc = new DarkScript_DelayedFunction(newFunc, this.curFrame);
			delayFunc.keepAcrossFrames = keepIfFusionFrameChanged != 0;
			delayFunc.numRepeats = numRepeats;
			delayFunc.numUnitsUntilRun = delayBetweenRuns;
			delayFunc.useTicks =
				this.$actionFuncs[actID] == this.RunFunction_Delayed_Num_Ticks ||
				this.$actionFuncs[actID] == this.RunFunction_Delayed_String_Ticks;
			if (delayFunc.useTicks) {
				delayFunc.runAtTick = this.curFrame + firstRunDelayedFor;
			}
			else {
				delayFunc.runAtTime = new Date(Date.now() + firstRunDelayedFor);
			}
			delayFunc.fusionStartEvent = this.Sub_GetLocation(-1);
			this.globals.pendingFuncs.push(delayFunc);

			this.Runtime.Rehandle();
		}
		else {
			throw Error("Wrong call");
		}

		return DummyReturn;
	};

	evt_SaveSelectedObjects(selectedObjectsFunc) {
		// first param was a ref in C++, now a setter func for list
		let wasFound = false;
		const DSOi = this.ho.get_Oi();
		let selectedObjects = [];
		let oiListIndex = -1;
		for (const poil of new darkEdif['AllOIListIterator'](this.rhPtr)) {
			++oiListIndex;
			// Skip our ext, it'll always appear in selection as it's how this code right here in our ext is running
			// Since every generated condition will use DarkScript, we shouldn't need to re-select after generated events finish
			// although... if the user is strange enough to mix DS with other loop-type objects, maybe.
			if (poil.get_Oi() == DSOi) {
				console.info(`Skipping object "${poil.get_name()}", oi ${poil.get_Oi()}, it should be DarkScript.`);
				wasFound = true;
				continue;
			}

			// Selection was caused by condition, or by action
			if (poil.get_EventCount() != this.rhPtr.GetRH2EventCount()) {
				console.info(`Skipping object "${poil.get_name()}", event count differs (it is ${poil.get_EventCount()}, but should be ${this.rhPtr.GetRH2EventCount()}) so it is not selected.`);
				continue;
			}
			// Already in the list?
			// TODO: Is this even possible? Multiple CRuns, maybe, but rhPtr is one CRun.
			let j;
			for (j = 0; j < selectedObjects.length; ++j) {
				if (selectedObjects[j].poil == poil) {
					break;
				}
			}

			let pSel;
			if (j < selectedObjects.length) {
				// In the list already => replace selection
				pSel = selectedObjects[j];
				pSel.selectedObjects.length = 0;
			}
			else {
				// Not in the list yet, add new selection
				let sel = new DarkScript_FusionSelectedObjectListCache(poil);
				selectedObjects.push(sel);
				pSel = selectedObjects[selectedObjects.length - 1];
			}

			pSel.poil = poil;

			// Store explicitly selected objects; we ignore Implicit via eventcount check anyway
			let numSelected = 0;
			for (const pHoFound of new darkEdif.ObjectIterator(this.rhPtr, oiListIndex, darkEdif.Selection.Explicit)) {
				//auto&& pHoFound = rhPtr->GetObjectListOblOffsetByIndex(num);
				// TODO: With multiple CRuns revealed, is this still functional across subapps?
				if (pHoFound == this.rdPtr) {
					wasFound = true;
				}
				if (pHoFound == null) {
					break;
				}
				++numSelected;
				pSel.selectedObjects.push(pHoFound.get_rHo().get_Number());
			//	num = pHoFound->get_rHo()->get_NextSelected();
			}
			if (numSelected > 0) {
				console.info(`Added object selection: object "${pSel.poil.get_name()}" num instances ${pSel.selectedObjects.length}.`);
			}
		}

		if (!wasFound) {
			console.error("Couldn't find DarkScript in selected objects!");
		}
		selectedObjectsFunc(selectedObjects);
	};
	evt_RestoreSelectedObjects(selectedObjects, unselectAllExisting) {
		// Unselect all objects
		if (unselectAllExisting) {
			for (const poil of new darkEdif.AllOIListIterator(this.rh)) {
				// Skip our ext, it'll always appear in selection because otherwise, how is this code right here in our ext running?
				if (poil.get_Oi() == this.rdPtr.get_rHo().get_Oi()) {
					continue;
				}
	
				// Invalidate the selection by making the event count not match, as opposed to explicitly selecting all
				poil.SelectAll(this.rh, false);
			}
		}
	
		const rh2EventCount = this.rh.GetRH2EventCount();
		for (let i = 0; i < selectedObjects.length; ++i) {
			const sel = selectedObjects[i];
			let poil = sel.poil;
	
			poil.set_EventCount(rh2EventCount);
			poil.set_ListSelected(-1);
			poil.set_NumOfSelected(0);
			if (sel.selectedObjects.length > 0) {
				let pHoPrev = this.rhPtr.GetObjectListOblOffsetByIndex(sel.selectedObjects[0]);
				if (pHoPrev != null) {
					poil.set_ListSelected(sel.selectedObjects[0]);
					poil.set_NumOfSelected(poil.get_NumOfSelected()+1);
					for (let j = 1; j < sel.selectedObjects.length; ++j) {
						let num = sel.selectedObjects[j];
						let pHo = this.rhPtr.GetObjectListOblOffsetByIndex(num);
						//pHo->SelectedInOR = 1;
						if (pHo == null) {
							break;
						}
						pHoPrev.get_rHo().set_NextSelected(num);
						poil.set_NumOfSelected(poil.get_NumOfSelected() + 1);
						pHoPrev = pHo;
					}
					pHoPrev.get_rHo().set_NextSelected(-1);
				}
			}
		}
	};

	ToLower(str2) {
		return str2.toLowerCase();
	};

	StringToType(typeSetFunc, typeStr) {
		// Unlike C++ ref for first param, this calls typeSetFunc and passes variable to set
		const typeStrL = this.ToLower(typeStr);
		if (typeStrL == "any" || typeStrL == "a") {
			typeSetFunc(DarkScript_Type.Any);
		}
		else if (typeStrL == "integer" || typeStrL == "int" || typeStrL == "i") {
			typeSetFunc(DarkScript_Type.Integer);
		}
		else if (typeStrL == "float" || typeStrL == "f") {
			typeSetFunc(DarkScript_Type.Float);
		}
		else if (typeStrL == "string" || typeStrL == "funcName" || typeStrL == "text" || typeStrL == "s") {
			typeSetFunc(DarkScript_Type.String);
		}
		else {
			return false;
		}
	
		return true;
	};
	TypeToString(type) {
		const typeStrs = [
			"Any", "Integer", "Float", "String"
		];
		return typeStrs[~~type];
	}

	/**
	 * 
	 * @param {string} cppFuncName C++ function name, for error reporting
	 * @param {string} funcNameOrBlank DarkScript func name, or blank for current running func
	 * @returns DarkScript_FunctionTemplate | null
	 */
	Sub_GetFuncTemplateByName(cppFuncName, funcNameOrBlank) {
		// Blank func: use current on-going
		if (funcNameOrBlank.length == 0) {
			if (this.curFuncTemplateLoop) {
				return this.curFuncTemplateLoop;
			}
			if (this.globals.runningFuncs.length == 0) {
				return this.CreateErrorExp(null, "%s: no function running and not a func template loop, so a function template name must be passed.", cppFuncName);
			}
			return this.globals.runningFuncs[this.globals.runningFuncs.length - 1].funcTemplate;
		}
	
		// Match by name
		const nameL = this.ToLower(funcNameOrBlank);
		const res = this.globals.functionTemplates.find(f => { return f.nameL == nameL; });
		if (res == null) {
			return this.CreateErrorExp(null, "%s: couldn't find a function template with name %s.", cppFuncName, funcNameOrBlank);
		}
		return res;
	};
	/**
	 * 
	 * @param {string} cppFuncName C++ function name, for error reporting
	 * @param {string} funcNameOrBlank DarkScript func name, or blank for current running func
	 * @returns DarkScript_RunningFunction | null
	 */
	Sub_GetRunningFunc(cppFuncName, funcNameOrBlank) {
		if (this.globals.runningFuncs.length == 0) {
			return this.CreateError("Use of %s without a function active.", cppFuncName), null;
		}
		if (funcNameOrBlank.length == 0) {
			return this.globals.runningFuncs[this.globals.runningFuncs.length - 1];
		}
	
		// Match by name
		const nameL = this.ToLower(funcNameOrBlank);
		const res = this.globals.runningFuncs.find(function(f) { return f.funcTemplate.nameL == nameL;} );
		if (res == null) {
			return this.CreateError("%s: couldn't find a running function with name %s.", cppFuncName, funcNameOrBlank), null;
		}
		return res;
	};
	Sub_GetAvailableVars(rf, includeParam) {
		let availableVars = [];
		let str = "";
		if (includeParam != DarkScript_Expected.Never) {
			for (let i = 0; i < rf.funcTemplate.params.length; ++i) {
				const tp = rf.funcTemplate.params[i];
				const pv = rf.paramValues[i];

				// Already outputted; a later function could overwrite an earlier scoped variable, so skip any we're doing in reverse
				if (availableVars.find((t) => { return t == tp.nameL; }) != null) {
					continue;
				}
				availableVars.push(tp.nameL);
				if (pv.type == DarkScript_Type.Integer) {
					str += `[int]    ${tp.name} = ${pv.data}\n`;
				}
				else if (pv.type == DarkScript_Type.Float) {
					str += `[float]  ${tp.name} = ${pv.data}\n`;
				}
				else if (pv.type == DarkScript_Type.String) {
					str += `[string] ${tp.name} = "${pv.data}"\n`;
				}
				else {
					str += `[any]    ${tp.name} = unset (${pv.dataSize} bytes)\n`;
				}
			}
		}
		if (includeParam != DarkScript_Expected.Always) {
			for (const it of this.globals.scopedVars) {
				// Already outputted; a later function could overwrite an earlier scoped variable, so skip any we're doing in reverse
				if (availableVars.find(t=> { return t == it.nameL; }) != null) {
					continue;
				}
				availableVars.push(it.nameL);
				if (it.defaultVal.type == DarkScript_Type.Integer) {
					str += `[int]    ${it.name} = ${it.defaultVal.data}\n`;
				}
				else if (it.defaultVal.type == DarkScript_Type.Float) {
					str += `[float]  ${it.name} = ${it.defaultVal.data}\n`;
				}
				else if (it.defaultVal.type == DarkScript_Type.String) {
					str += `[string] ${it.name} = "${it.defaultVal.data}"\n`;
				}
				else {
					str += `[any]    ${it.name} = unset (${it.defaultVal.dataSize} bytes)\n`;
				}
			}
		}

		let availableVarsStr = "";
		str = "";

		let types;
		if (includeParam == DarkScript_Expected.Either)
			types = "params/scoped vars";
		else if (includeParam == DarkScript_Expected.Always)
			types = "params";
		else
			types = "scoped vars";

		if (availableVarsStr.length == 0) {
			str += `No ${types} available.`;
		}
		else {
			str += `Available ${types}:\n${availableVarsStr.substring(0,availableVarsStr.length - 1)}`;
		}
		return str;
	};
	/**
	 * @param {string} cppFuncName
	 * @param {number} paramNum
	 * @returns DarkScript_Value | null
	 */
	Sub_CheckParamAvail(cppFuncName, paramNum) {
		if (paramNum < 0) {
			return this.CreateErrorExp(null, "%s: param index below 0. (%i)", cppFuncName, paramNum);
		}
	
		const rf = this.Sub_GetRunningFunc(cppFuncName, "");
		if (!rf) {
			return null;
		}
	
		if (rf.funcTemplate.params.length < paramNum) {
			return this.CreateErrorExp(null, "%s: param index out of range (0 < %i < %zu).",
				cppFuncName, paramNum, rf.funcTemplate.params.length);
		}
	
		return rf.paramValues[paramNum];
	};
	/**
	 * 
	 * @param {boolean} Opt 
	 * @param {null} Ret 
	 * @param {string} Error format
	 * @param  {...any} a 
	 * @returns null
	 */
	CreateErrorExpOpt(Opt, Ret, Error, ...a) {
		if (Opt) {
			this.CreateError.apply(this, [Error].concat(a));
		}
		return Ret;
	};
	CreateErrorExp(Ret, Error, ...a) { 
		this.CreateError(Error, a);
		return Ret;
	};
	/**
	 * 
	 * @param {string} cppFuncName C++ function name, for error reporting
	 * @param {string} scopedVarName Scoped variable name to look for
	 * @param {DarkScript_Expected} shouldBeParam Allows you to search for param alone, or as well as scoped vars.
	 * @param {boolean} makeError If true, makes error here. If not, just returns.
	 * @param {function | null} paramTo If passed, the param is written to this.
	 * @returns DarkScript_Value | null
	 */
	Sub_CheckScopedVarAvail(cppFuncName, scopedVarName, shouldBeParam, makeError, paramTo = null) {
		if (scopedVarName.length == 0) {
			return this.CreateErrorExpOpt(makeError, null, "%s: param/scoped var name is blank.", cppFuncName);
		}
	
		let rf = this.globals.runningFuncs.length == 0 && !makeError ? null : this.Sub_GetRunningFunc(cppFuncName, "");
		if (!rf) {
			return null;
		}
	
		const strL = this.ToLower(scopedVarName);
	
		if (shouldBeParam != DarkScript_Expected.Always) {
			const scopedVar = this.globals.scopedVars.find(
				function(a) { return a.nameL == strL; }
			);
			if (scopedVar != null) {
				if (paramTo != null) {
					paramTo(scopedVar);
				}
				return scopedVar.defaultVal;
			}
	
			// Must be scoped var, not param, and not found - ded
			if (shouldBeParam == DarkScript_Expected.Never) {
				return this.CreateErrorExpOpt(makeError, null, "%s: %s: scoped var name \"%s\" not found.\n%s",
					cppFuncName, rf.funcTemplate.name, scopedVarName, this.Sub_GetAvailableVars(rf, shouldBeParam));
			}
		}
		// shouldBeParam is either Either or Always, Never would've returned in ^ already
	
		const param = rf.funcTemplate.params.findIndex(
			function(a) { return a.nameL == strL; }
		);
		if (param == -1)
		{
			return this.CreateErrorExpOpt(makeError, null, "%s: %s: param%s name \"%s\" not found.%s\n%s",
				cppFuncName, rf.funcTemplate.name, shouldBeParam == DarkScript_Expected.Never ? "" : "/scoped var", scopedVarName,
				!rf.funcTemplate.isAnonymous ? "" : " Anonymous functions use \"a\", \"b\" for parameters.\n",
				this.Sub_GetAvailableVars(rf, shouldBeParam)
			);
		}
		if (paramTo) {
			paramTo(rf.funcTemplate.params[param]);
		}
		return rf.paramValues[param];
	};
	Sub_GetTemplateParam(cppFuncName, f, paramNumOrStr) {
		if (!f) {
			return null;
		}
		if (typeof paramNumOrStr == 'number') {
			let paramNum = paramNumOrStr;
			if (paramNum < 0) {
				return this.CreateErrorExp(null, "%s: param index below 0. (%i)", cppFuncName, paramNum);
			}
		
			if (f.params.length < paramNum) {
				return this.CreateErrorExp(null, "%s: param index out of range (0 < %i < %zu).",
					cppFuncName, paramNum, f.params.length);
			}
		
			return f.params[paramNum];
		}
		if (typeof paramNumOrStr == 'string') {
			const paramName = paramNumOrStr;
			if (!f) {
				return null;
			}
		
			if (paramName.length == 0) {
				return this.CreateErrorExp(null, "%s: param name is blank.", cppFuncName);
			}
		
			const strL = this.ToLower(paramName);
			const param = f.params.find(a => { return a.nameL == strL; });
			if (param == null) {
				return this.CreateErrorExp(null, "%s: param name \"%s\" not found.", cppFuncName, paramName);
			}
		
			return param;
		}

		throw Error("Unexpected type passed to Sub_GetTemplateParam");
	};
	Sub_GetOrCreateTemplateScopedVar(cppFuncName, funcName, scopedVarName) {
		if (scopedVarName.length == 0) {
			return this.CreateErrorExp(null, "%s: scoped var name is blank.", cppFuncName);
		}
		if (funcName.length == 0) {
			return this.CreateErrorExp(null, "%s: func name is blank.", cppFuncName);
		}
	
		const f = this.Sub_GetFuncTemplateByName(cppFuncName, funcName);
		if (!f) {
			return null;
		}
	
		const strL = this.ToLower(scopedVarName);
		const scopedVar = f.scopedVarOnStart.find(a => { return a.nameL == strL; });
		if (scopedVar == null) {
			f.scopedVarOnStart.push(new DarkScript_ScopedVar(scopedVarName, DarkScript_Type.Any, false, -1));
			return f.scopedVarOnStart[f.scopedVarOnStart.length - 1];
		}
	
		return scopedVar;
	};

	Sub_GetValAsInteger(val) {
		switch (val.type)
		{
		case DarkScript_Type.String:
		{
			let ret = parseInt(val.data);
			if (ret === null) {
				this.CreateErrorExp(0, `Error while converting value "${val.data}" to integer.`);
			}
			return ret;
		}
		case DarkScript_Type.Float:
			return Math.floor(val.data); // casting to float will cause round-down
		case DarkScript_Type.Integer:
			return val.data;
		case DarkScript_Type.Any:
		default:
			return this.CreateError("Failed to interpret Value as integer - invalid type."), 0;
			// while the default can be unset and thus remain any, it should be replaced when running
		}
	};
	Sub_GetValAsFloat(val) {
		switch (val.type)
		{
		case DarkScript_Type.String:
		{
			let ret = parseFloat(val.data);
			if (ret === null) {
				this.CreateErrorExp(0, `Error while converting value "${val.data}" to float.`);
			}
			return ret;
		}
		case DarkScript_Type.Float:
		case DarkScript_Type.Integer:
			return val.data; // In JS, there is only one Number to rule them all
		case DarkScript_Type.Any:
		default:
			return this.CreateError("Failed to interpret Value as float - invalid type."), 0;
			// while the default can be unset and thus remain any, it should be replaced when running
		}
	};
	Sub_GetValAsString(val) {
		switch (val.type)
		{
		case DarkScript_Type.String:
			return val.data ? val.data : "";
		case DarkScript_Type.Float:
			return this.Sub_ConvertToString(val.data, true);
		case DarkScript_Type.Integer:
			return this.Sub_ConvertToString(val.data, false);
		case DarkScript_Type.Any:
		default:
			return this.CreateError("Failed to interpret Value as string - invalid type."), "";
			// while the default can be unset and thus remain any, it should be replaced when running
		}
	};
	Sub_ConvertToString(val, readAsFloat) {
		if (!readAsFloat)
		{
			if (typeof val !== "number" || !Number.isInteger(val)) {
				this.CreateError(`Failed to convert integer; "${val}" is not an integer. Will best guess.`);
				return "" + Math.floor(val);
			}
			return val.toString();
		}
		if (typeof val !== "number") {
			this.CreateError(`Failed to convert float; "${val}" is not a number. Will best guess.`);
		}

		return val.toString();
	};

	/**
	 * 
	 * @param {DarkScript_RunningFunction} a 
	 * @param {DarkScript_RunningFunction} b 
	 * @returns boolean
	 */
	Sub_FunctionMatches(a, b) {
		if (a == b || a.funcTemplate == b.funcTemplate)
			return true;
	
		// If templates must exist, there cannot be lambda, so the func templates are always shared
		// by being in globals->funcTemplates, so the above if ^ would have triggered from template matching
		if (this.funcsMustHaveTemplate) {
			return false;
		}
	
		// Possible lambda, have to check func name
		return a.funcTemplate.nameL == b.funcTemplate.nameL;
	};
	/**
	 * 
	 * @param {number} actID 
	 * @returns string
	 */
	Sub_GetLocation(actID) {
		const GetActionIndex = function(ext) {
			let exp = ext.rhPtr.GetRH4ActionStart();
			if (exp == null) {
				return "a condition";
			}
			let evg = ext.rhPtr.get_EventGroup();
			if (evg == null) {
				return "unknown (action?)";
			}
	
			let cur = evg.GetCAByIndex(0);
			let i = 0;
			for (; i < evg.get_evgNCond(); ++i) {
				cur = cur.Next();
			}
			let lastOI = cur.get_evtOi();
			let str = "";
	
			for (i = 0; i < evg.get_evgNAct(); ++i) {
				if (cur == exp) {
					lastOI = cur.get_evtOi();
					// We're using 1-based for action index, so increment i
					++i;
	
					// If it's DarkScript, try not to say "DarkScript action running DarkScript".
					// We check the name, in case there's multiple DarkScripts named differently by user.
					const name = ext.Runtime.ObjectSelection.GetOILFromOI(lastOI).get_name();
	
					if (ext.rdPtr.get_rHo().get_Oi() != lastOI || name != "DarkScript") {
						str += `${name}'s action (index ${i})`;
					}
					else {
						str += `action index ${i}`;
					}
					return str;
				}
				cur = cur.Next();
			}
			if (cur && cur == exp)
			{
				str += ext.Runtime.ObjectSelection.GetOILFromOI(lastOI).get_name() + `'s action (index ${i + 1})`;
				return str;
			}
			return "unknown";
		};
		let callerOrNull =
			this.globals.runningFuncs.length == 0 ? null : this.globals.runningFuncs[this.globals.runningFuncs.length - 1];
		// we're not multithreaded, so either it's the Handle() from a delayed function, or it's a Fusion event.
		const curFusionEvent = darkEdif.GetCurrentFusionEventNum(this);
		let str = "";
	
		// Not run inside a DarkScript action, so we'll assume it's run by expression
		if (actID == -1) {
			if (curFusionEvent != -1) {
				str += `DarkScript expression in Fusion event #${curFusionEvent}, in ${GetActionIndex(this)}`;
				return str;
			}
	
			// If this is running, it wasn't detected as being run in a DarkScript action,
			// and there is no current Fusion event, so it must be getting triggered by Handle(),
			// but we don't look up location for delayed func then, we look it up when delayed func is queued.
	
			debugger;
			return "Unknown event";
		}
	
		if (this.$actionFuncs[actID] == this.RunFunction_ActionDummy_Num ||
			this.$actionFuncs[actID] == this.RunFunction_ActionDummy_String)
		{
			// currently this overrides when a delayed funcA calls a non-delayed funcB which calls a non-delayed funcC.
			//callerOrNull->funcToRun->level
			str += `DarkScript dummy action in Fusion event #${curFusionEvent}, in ${GetActionIndex(this)}`;
			return str;
		}
		if (this.$actionFuncs[actID] == this.RunFunction_Foreach_Num ||
			this.$actionFuncs[actID] == this.RunFunction_Foreach_String) {
			str += `DarkScript foreach action in Fusion event #${curFusionEvent}, in ${GetActionIndex(this)}`;
			return str;
		}
		if (this.$actionFuncs[actID] == this.RunFunction_Script) {
			str += `DarkScript scripting action in Fusion event #${curFusionEvent}, in ${GetActionIndex(this)}`;
			return str;
		}
		if (this.$actionFuncs[actID] == this.RunFunction_Delayed_Num_MS ||
			this.$actionFuncs[actID] == this.RunFunction_Delayed_Num_Ticks ||
			this.$actionFuncs[actID] == this.RunFunction_Delayed_String_MS ||
			this.$actionFuncs[actID] == this.RunFunction_Delayed_String_Ticks)
		{
			str += "DarkScript delayed function call, queued by " + 
				this.curDelayedFunc?.fusionStartEvent.substring("DarkScript expression in ".length);
			return str; // _T("delayed function \""s) + callerOrNull->funcToRun->funcTemplate->name + _T("\" queued by "s) + callerOrNull->fusionStartEvent;
		}
	
		console.assert(false && "Unrecognised action ID");
		return "";
	};
	Sub_ReplaceAllString(str, from, to) {
		if (str == null || from == null || to == null)
			throw Error("Replace was passed null");
		return str.replaceAll(from, to);
	};

	/**
	 * 
	 * @param {CRunExtension | null} objToRunOn Single object instance for foreach, or null for non-foreach
	 * @param {DarkScript_RunningFunction} rf Function to run
	 * @returns string | number
	 */
	ExecuteFunction(objToRunOn, rf) {
		// If running on another ext like subapp, we want to use its globals.
		let globalsRunningOn = rf.funcTemplate.ext.globals;

		// TODO: Can optimize this by adding "isrunning" active to template
		if ((this.preventAllRecursion || !rf.funcTemplate.recursiveAllowed) && globalsRunningOn.runningFuncs.length > 1) {
			let olderIt = globalsRunningOn.runningFuncs.find(
				function (f) { return this.Sub_FunctionMatches(rf, f); }
			);
			if (olderIt != null)
			{
				rf.abortReason = "Aborted from DarkScript recursion error.";
				this.CreateError("Running a function recursively, when not allowed. Older run was from %s; current, aborted run is from %s. Aborting.",
					olderIt.runLocation, rf.runLocation);
			}
		}

		rf.active = true;

		// If not foreach, we want to save and restore scoped variables?
		// TODO: Shouldn't we do this for foreach too, or is it a optimization based on not actually running here?
		let numScopedVarsBeforeCall = Number.MAX_VALUE;
		if (!objToRunOn) {
			numScopedVarsBeforeCall = globalsRunningOn.scopedVars.length;
			if (this.inheritParametersAsScopedVariables && globalsRunningOn.runningFuncs.length != 0) {
				let lastFunc = globalsRunningOn.runningFuncs[globalsRunningOn.runningFuncs.length - 1];
				for (let i = 0; i < lastFunc.funcTemplate.params.length; ++i) {
					globalsRunningOn.scopedVars.push(new DarkScript_ScopedVar(lastFunc.funcTemplate.params[i].name, DarkScript_Type.Any, true, globalsRunningOn.runningFuncs.length));
					globalsRunningOn.scopedVars[globalsRunningOn.scopedVars.length - 1].defaultVal = lastFunc.paramValues[i];
				}
			}
			globalsRunningOn.runningFuncs.push(rf);
			for (const sv of rf.funcTemplate.scopedVarOnStart) {
				if (sv.recursiveOverride || 1 == globalsRunningOn.runningFuncs.filter(
					f => this.Sub_FunctionMatches(rf, f)).length) {
					globalsRunningOn.scopedVars.push(sv);
					globalsRunningOn.scopedVars[globalsRunningOn.scopedVars.length - 1].level = globalsRunningOn.runningFuncs.length;
				}
			}
		}

		// Set up subapp ext for running cross-frame, and in case they do a cross-frame themselves, we'll store what they're using
		let orig = rf.funcTemplate.ext.errorExt;
		rf.funcTemplate.ext.errorExt = this;

		if (objToRunOn) {
			console.info("Running On Foreach function \"%s\", passing object \"%s\", FV %i.", rf.funcTemplate.name, objToRunOn.get_rHo().get_OiList().get_name(), rf.currentForeachObjFV);
		}
		else {
			console.info("Running On Function \"%s\". Set to repeat %i times.", rf.funcTemplate.name, rf.numRepeats);
		}
		do {
			// On Obj Function, or On Function
			rf.funcTemplate.ext.Runtime.GenerateEvent(objToRunOn ? 2 : 1);

			if (this.enableOnAnyFunctions) {
				rf.funcTemplate.ext.Runtime.GenerateEvent(11); // On Any Function
			}

			// No On XX Function event ran
			if (!rf.eventWasHandled) {
				if (this.createErrorForUnhandledEvents) {
					if (objToRunOn) {
						this.CreateError("Function \"%s\" was not handled by any On Function \"%s\" for each %s events.",
							rf.funcTemplate.name, rf.funcTemplate.name,
							globalsRunningOn.runningFuncs[globalsRunningOn.runningFuncs.length - 1].currentForeachOil < 0 ? "(qualifier)" : 
							objToRunOn.get_rHo().get_OiList().get_name());
					}
					else {
						this.CreateError("Function \"%s\" was not handled by any On Function \"%s\" events.", rf.funcTemplate.name,
							rf.funcTemplate.name);
					}
				}
				// No events are triggering, no point running the rest
				rf.foreachTriggering = false;
				break;
			}

			// User cancelled the loop
			if (!rf.nextRepeatIterationTriggering) {
				break;
			}

			// In case they cancelled current iteration's On Function, reset back to normal for next loop
			rf.currentIterationTriggering = true;
		} while (rf.active && rf.numRepeats >= ++rf.index);

		// We need return value, no default return value, but we're returning with no return value set... uh oh
		if (rf.returnValue.dataSize == 0 && rf.funcTemplate.defaultReturnValue.dataSize == 0) {
			// Anonymous functions can have no return value set, and we'll just return 0 or ""
			// If void run, it's a delayed or foreach action
			if (this.whenAllowNoRVSet == DarkScript_AllowNoReturnValue.AllFunctions ||
				(rf.isVoidRun && this.whenAllowNoRVSet >= DarkScript_AllowNoReturnValue.ForeachDelayedActionsOnly) ||
				(rf.funcTemplate.isAnonymous && this.whenAllowNoRVSet >= DarkScript_AllowNoReturnValue.AnonymousForeachDelayedActions))
			{
				console.info(`${objToRunOn ? "Foreach " : ""}Function "${rf.funcTemplate.name}" has no default return value, and no return ` +
					`value was set by any On ${objToRunOn ? "Foreach " : ""}Function "${rf.funcTemplate.name}" events. ` +
					`Due to "allow no default value" property, ignoring it and returning 0/"".`);
				// Return to calling expression - return int and float directly as they occupy same memory address
				if (rf.returnValue.type != DarkScript_Type.String) {
					rf.returnValue.dataSize = 4; // sizeof int or float, same thing
				}
				else {
					rf.returnValue.data = "";
				}
			}
			// else error: do foreach error, or do normal error
			else if (objToRunOn) {
				this.CreateError("Function \"%s\" has no default return value, and no return value was set by any On Function \"%s\" for each %s events.",
					rf.funcTemplate.name, rf.funcTemplate.name,
					rf.currentForeachOil < 0 ? "(qualifier)" : objToRunOn.get_rHo().get_OiList().get_name());
			}
			else { 
				this.CreateError("Function \"%s\" has no default return value, and no return value was set by any On Function \"%s\" events.",
					rf.funcTemplate.name, rf.funcTemplate.name);
			}
		}

		// No abort
		if (rf.abortReason.length == 0) {
			if (this.enableOnAnyFunctions) {
				rf.funcTemplate.ext.Runtime.GenerateEvent(12); // On any function completed OK
			}
		}
		else { // Function was aborted
			rf.funcTemplate.ext.Runtime.GenerateEvent(3); // On function aborted
			if (this.enableOnAnyFunctions) {
				rf.funcTemplate.ext.Runtime.GenerateEvent(13); // On any function aborted
			}

			if (!rf.abortWasHandled && this.createErrorForUnhandledAborts) {
				this.CreateError("Function abort \"%s\" was not handled by any On Function \"%s\" Aborted events.", rf.funcTemplate.name, rf.funcTemplate.name);
			}
		}

		// Reset error handling ext back to what it was
		rf.funcTemplate.ext.errorExt = orig;

		// If not a foreach (which keeps runningFunc itself), delete our func
		if (!objToRunOn) {
			globalsRunningOn.runningFuncs.splice(globalsRunningOn.runningFuncs.length - 1);
		}

		// Remove all scoped vars we added on this level
		// Since any function we call from this one will have erased their own, in theory this will only delete ours
		if (numScopedVarsBeforeCall != Number.MAX_VALUE) {
			globalsRunningOn.scopedVars.splice(numScopedVarsBeforeCall);
		}

		// Store return value, in case we're running a foreach and later actions need the return
		this.lastReturn = rf.returnValue;

		// Return to calling expression - return int and float directly as they occupy same memory address
		if (rf.returnValue.type != DarkScript_Type.String) {
			return rf.returnValue.data;
		}

		// Clone string
		let tc = rf.returnValue.data;
		// assert(tc); // shouldn't have it as null under any scenario
		return tc ? tc : "";
	};
	/**
	 * 
	 * @param {number} oil 
	 * @param {DarkScript_RunningFunction} runningFunc 
	 * @returns 
	 */
	Sub_RunPendingForeachFunc(oil, runningFunc) {
		console.info(`Sub_RunPendingForeachFunc executing for oil ${oil} (non-qual: ${oil & 0x7FFF}), function "${runningFunc.funcTemplate.name}".`);

		/** @type CRunExtension[] */
		let list = [];

		for (const pqoi of new darkEdif.ObjectIterator(this.rhPtr, oil, darkEdif.Selection.Implicit, false)) {
			list.push(pqoi);
		}
		// else -1, just leave list empty
		// oil could be -1 if valid object type, but no instances - or invalid obj type, e.g. global events but no corresponding frame obj

		// No instances available
		if (list.length == 0) {
			console.info("Foreach loop \"%s\" not starting! No object instances available when action was run.\n",
				runningFunc.funcTemplate.name);
			return;
		}

		/** @type DarkScript_FusionSelectedObjectListCache[] */
		let selObjList = [];
		this.evt_SaveSelectedObjects((s) => { selObjList = s;});

		if (this['DebugMode']) {
			let totalSel = 0;
			for (const a of selObjList) {
				totalSel += a.selectedObjects.length;
			}
			console.debug(`Event ${darkEdif.GetCurrentFusionEventNum(this)} starting foreach ` +
				`func "${runningFunc.funcTemplate.name}"; selection was saved with ${selObjList.length} context ` + 
				`objects, ${totalSel} instances within. Will for-each over ${list.length} instances.`);
		}
		runningFunc.currentForeachOil = oil;
		/** @type CRunExtension */
		let pHo;
		for (const runObj of list) {
			pHo = runObj.get_rHo();
			// One of the ExecuteFunctions may destroy obj we haven't looped through yet
			if ((pHo.get_Flags() & CObject.HOF_DESTROYED) != CObject.HOF_NONE) {
				continue;
			}

			console.info(`Event ${darkEdif.GetCurrentFusionEventNum(this)} running current foreach ` +
				`func "${runningFunc.funcTemplate.name}" for object "${pHo.get_OiList().get_name()}", `+
				`FV ${pHo.GetFixedValue()}, num ${pHo.get_Number()} [object list], oi ${pHo.get_Oi()} [oilist].`);
			runningFunc.currentForeachObjFV = pHo.GetFixedValue();
			this.ExecuteFunction(runObj, runningFunc);
			// User cancelled the foreach loop or aborted it
			if (!runningFunc.foreachTriggering || !runningFunc.active) {
				if (!runningFunc.active) {
					console.info("Aborted foreach func \"%s\" with reason \"%s\".", runningFunc.funcTemplate.name, runningFunc.abortReason);
				}
				else {
					console.info("Foreach func \"%s\" was cancelled.", runningFunc.funcTemplate.name);
				}
				this.evt_RestoreSelectedObjects(selObjList, true);
				return;
			}
			this.lastReturn = runningFunc.returnValue;
		}
		console.info("End of foreach func \"%s\", OI %i, restoring object selection.", runningFunc.funcTemplate.name, oil);
		this.evt_RestoreSelectedObjects(selObjList, true);
	};
	/**
	 * Attempts to read the DarkScript value from text, writes to function if it can
	 * @param {string} cppFuncName C++ function name, for error reporting
	 * @param {string} valueTextToParse Text value of param to read
	 * @param {DarkScript_Param} paramExpected DarkScript parameter type
	 * @param {number} paramIndex DarkScript param index
	 * @param {function} writeToFunc Where to write the interpreted value to
	 * @returns boolean
	 */
	Sub_ParseParamValue(cppFuncName, valueTextToParse, paramExpected, paramIndex, writeToFunc) {
		// String type
		if (valueTextToParse[0] == '"') {
			if (paramExpected.type != DarkScript_Type.Any && paramExpected.type != DarkScript_Type.String) {
				return this.CreateError("%s: Parameter \"%s\" (index %zu) has a string default value, but is set to type %s.",
					cppFuncName, paramExpected.name, paramIndex, this.TypeToString(paramExpected.type)), false;
			}
	
			// Starts with " but no ending "; or is just a " by itself.
			if (valueTextToParse[valueTextToParse.length - 1] != '"' || valueTextToParse.length == 1) {
				return this.CreateError("%s: Parameter \"%s\" (index %zu) has corrupt default value [%s]. Make sure your string parameters end with quotes and are escaped properly.",
					cppFuncName, paramExpected.name, paramIndex, valueTextToParse), false;
			}
	
			// remove starting and ending quotes
			valueTextToParse = valueTextToParse.slice(1, valueTextToParse.length - 2);
	
			// Double-quote within the string, without a \ before it.
			// This turned up during a test and was incredibly annoying, as it silently merged 2nd and 3rd parameter as 2nd didn't correctly end with a ".
			// Example case: foo("bar",""yes")
			for (let i = valueTextToParse.indexOf('"'); i != -1; i = valueTextToParse.indexOf('"', i + 1)) {
				if (valueTextToParse[i] == '"' && (i == 0 || valueTextToParse[i - 1] != '\\')) {
					return this.CreateError("%s: Parameter \"%s\" (index %zu) has unescaped double-quote at index %zu [%s%s%s].",
						cppFuncName, paramExpected.name, paramIndex, i, i < 10 ? "..." : "", valueTextToParse.substr(Math.max(0, i - 10), 20), valueTextToParse.length - 20 > 0 ? "..." : ""), false;
				}
			}
			for (let i = valueTextToParse.indexOf('\\'); i != -1; i = valueTextToParse.indexOf('\\', i + 1)) {
				// ends with a '\' and nothing following... ignore it
				if (i == valueTextToParse.length) {
					break;
				}
	
				// escaped backslash, or quote: just remove the escaping
				if (valueTextToParse[i + 1] == '\\' || valueTextToParse[i + 1] == '"') {
					valueTextToParse = valueTextToParse.substring(0, i) +
					valueTextToParse.substring(i + 1);
					--i;
				}
				// We remove \\r entirely
				else if (valueTextToParse[i + 1] == 'r') {

					valueTextToParse = valueTextToParse.substring(0, i) +
						valueTextToParse.substring(i + 1);
					--i;
				}
				// \\n is replaced with native newline
				else if (valueTextToParse[i + 1] == 'n') {
					valueTextToParse = valueTextToParse.substring(0, i) + '\n' +
						valueTextToParse.substring(i + 2);
				}
				else if (valueTextToParse[i + 1] == 't') {
					valueTextToParse = valueTextToParse.substring(0, i) + '\t'
						valueTextToParse.substring(i + 2);
				}
			}
	
			let r = new DarkScript_Value(DarkScript_Type.String);
			r.data = valueTextToParse;
			r.dataSize = valueTextToParse.length + 1;
			writeToFunc(r);
			return true;
		}
	
		// expected float
		const endsWithF = valueTextToParse[valueTextToParse.length - 1].toLowerCase() == 'f';
		if (endsWithF || valueTextToParse.indexOf('.') != -1) {
			if (paramExpected.type != DarkScript_Type.Any && paramExpected.type != DarkScript_Type.Float) {
				return this.CreateError("%s: Parameter \"%s\" (index %zu) has a float default value, but is set to type %s.",
					cppFuncName, paramExpected.name, paramIndex, this.TypeToString(paramExpected.type)), false;
			}
	
			if (endsWithF) {
				valueTextToParse = valueTextToParse.substring(valueTextToParse.length - 1);
			}
	
			let f = parseFloat(valueTextToParse);
			// Returns 0 and endPos should be set to start pos if failed to convert
			// If out of bounds of float, should return ERANGE
			if (isNaN(f) && valueTextToParse.substring(0, 3).indexOf('N') == -1) {
				return this.CreateError("%s: Parameter \"%s\" (index %zu) has corrupt default float value [%s].",
					cppFuncName, paramExpected.name, paramIndex, valueTextToParse), false;
			}
	
			let r = new DarkScript_Value(DarkScript_Type.Float);
			r.data = f;
			r.dataSize = 4; // sizeof(float), although JS uses larger doubles
			writeToFunc(r);
			return true;
		}
	
		if (paramExpected.type != DarkScript_Type.Any && paramExpected.type != DarkScript_Type.Integer) {
			return this.CreateError("%s: Parameter \"%s\" (index %zu) has an integer default value, but is set to type %s.",
				cppFuncName, paramExpected.name, paramIndex, this.TypeToString(paramExpected.type)), false;
		}
	
		const d = parseInt(valueTextToParse);
		// Returns NaN if failed to convert
		if (isNaN(d)) {
			return this.CreateError("%s: Parameter \"%s\" (index %zu) has corrupt default integer value [%s].",
				cppFuncName, paramExpected.name, paramIndex, valueTextToParse), false;
		}
	
		let r = new DarkScript_Value(DarkScript_Type.Integer);
		r.data = d;
		r.dataSize = 4; // sizeof(int), although JS uses larger doubles
		writeToFunc(r);
		return true;
	};

	CreateError(err, ...a) {
		// Orig C++ function is variadic
		if (a.length > 0) {
			let idx = 0;
			err = err.replace(/%[hl]{0,2}z?[sud]/g, function () {
				return a[idx++];
			});
		}

		console.trace(`DarkScript error: ${err}`);

		if (this.curError.length != 0) {
			console.error("DarkScript error: a recursive error occurred while handling a DarkScript error. Stack memory overflow bug prevented.\n" +
					"Original error:\n" + this.curError + "\nNew error while handling original error:\n" + err);
			return; // don't generate event
		}
		let fusionEventNum = darkEdif.GetCurrentFusionEventNum(this);
		let prefix = "";
		if (fusionEventNum != -1)
			prefix = `[Fusion event #${fusionEventNum}] `;
		this.curError = prefix + err;

		let list = [];
		this.evt_SaveSelectedObjects((l) => list = l);
		this.Runtime.GenerateEvent(0);
		this.evt_RestoreSelectedObjects(list, true);

		this.curError = ""; // to prevent recursion
	}
	// ======================================================================================================
	// Actions
	// ======================================================================================================
	Template_SetFuncSignature(funcSig, delayable, repeatable, recursable) {
		if (funcSig[0].length == 0) {
			return this.CreateError("Template_SetFuncSignature: You must supply a function signature, not blank.");
		}
				
		// Test these first cos they're faster
		if ((recursable & 1) != recursable) {
			return this.CreateError(`Template_SetFuncSignature: Parameter recursable must be 0 or 1, you supplied ${recursable}.`);
		}
		if (delayable < 0 || delayable > 2) {
			return this.CreateError(`Template_SetFuncSignature: Parameter "delay expected" must be 0, 1 or 2, you supplied ${delayable}.`);
		}
		if (repeatable < 0 || repeatable > 2) {
			return this.CreateError(`Template_SetFuncSignature: Parameter "repeating expected" must be 0, 1 or 2, you supplied ${repeatable}.`);
		}

		// This regex takes into account strings with backslash escaping in default values, double quotes, etc.
		// Anything is allowed for parameter names, as long as it's 1+ character, does not contain whitespace or brackets, and is unique.
		// So, to confuse yourself, {} and [] are allowed as parameter names.
		let funcSigParser = new RegExp(String.raw`^(any|int|string|float)\s+([^\s(]+)\s*\(((?:\s*(?:[^\s]+)\s+(?:[^,\s]+)(?:\s*=\s*(?:[^",]+|(?:"(?:(?=(?:\\?)).)+?")))?,\s*)*(?:\s*(?:[^\s]+)\s+(?:[^,)\s=]+)(?:\s*=\s*(?:[^",]+|(?:"(?:(?=(?:\\?)).)*?")))?)?)\)$`);

		let funcSigBreakdown = funcSigParser.exec(funcSig);
		if (funcSigBreakdown == null) {
			return this.CreateError(`Template_SetFuncSignature: Function signature "${funcSig}" not parseable.`);
		}

		const funcNameL = this.ToLower(funcSigBreakdown[2]);

		// Make sure it's not a KRFuncXXX() function name. That would work, but the script engine
		// (used in "run text as script") will find it ambiguous.
		const isKRFunc = new RegExp("^k?r?f?func[fis]*\\$?$");
		if (isKRFunc.test(funcNameL)) {
			return this.CreateError(`Template_SetFuncSignature: Function name "${funcSigBreakdown[2]}" is invalid; KRFuncXX format will confuse the script parser.`);
		}

		/** @type DarkScript_Type */
		let returnTypeValid = -1;
		if (!this.StringToType((v) => { returnTypeValid = v; }, funcSigBreakdown[1])) {
			return this.CreateError(`Template_SetFuncSignature: Return type "${funcSigBreakdown[1]}" not recognised. Use Any, String, Integer, or Float.`);
		}

		let params = [];

		// do parameters exist?
		if (funcSigBreakdown[3].length != 0) {
			// Regex is dumb and for repeating groups, only the last one will be considered;
			// so we re-run a regex on the parameter list, looping all matches.
			// .NET regex does capture repeating groups nicely, but no other regex engines support it.
			const paramListParser = new RegExp(String.raw`\s*([^\s]+)\s+([^,)\s=]+)(?:\s*=\s*([^",]+|(?:"(?:(?=(?:\\?)).)*?")))?\s*,`, "g");
			const paramList = funcSigBreakdown[3] + ',';
			const paramListMatches = paramList.matchAll(paramListParser);
			let j = 1;
			for (const i of paramListMatches) {
				const paramTypeStr = i[1];
				const paramName = i[2];
				const paramNameL = this.ToLower(paramName);
				/** @type DarkScript_Type */
				let paramTypeValid = -1;
				if (!this.StringToType((a) => {paramTypeValid = a;}, paramTypeStr)) {
					return this.CreateError(`Template_SetFuncSignature: Parameter "${paramName}" (index ${j}) has unrecognised type "${paramTypeStr}". Use Any, String, Integer, or Float.`);
				}

				let existingParamFoundAt = -1;
				const existingParam = params.find((p, v) => { if (p.nameL == paramNameL) { existingParamFoundAt = v; return true; } return false; });
				if (existingParam != null) {
					return this.CreateError(`Template_SetFuncSignature: Parameter "${paramName}" (index ${j}) has the same name as previous parameter index ${existingParamFoundAt}.`);
				}

				// type funcCallBreakdown[i], name funcS
				params.push(new DarkScript_Param(paramName, paramTypeValid));
				if (i[3] == null) {
					// No default is fine as long as last param didn't have a default
					if (j > 1 && params[j - 2].defaultVal.type != DarkScript_Type.Any) {
						return this.CreateError(`Template_SetFuncSignature: Parameter "${paramName}" (index ${j}) has no default value, but earlier parameter "${params[j - 2].name}" (index ${j - 1}) has a default.` +
							` All parameters with defaults must be at the end of the parameter list.`);
					}
					continue;
				}

				// there was an =, but no content
				// Note that "= ," and "= )" will match, so we check for defaultVal being whitespace.
				// Whitespace is normally absorbed by the previous greedy matcher, but it will ignore one
				// whitespace if there is no default val, rather than failing the regex.
				// So, if it starts with whitespace, it ignored one to match, so boo.
				const defaultVal = i[3];
				if (defaultVal.length == 0 || /\s/.test(defaultVal[0][0])) {
					return this.CreateError(`Template_SetFuncSignature: Parameter "${paramName}" (index ${j}) has an empty default value; don't include the '=' if you want no default.`);
				}
				const justAdded = params[params.length - 1];
				if (!this.Sub_ParseParamValue("Template_SetFuncSignature", defaultVal, justAdded, j, (t) => { justAdded.defaultVal = t; })) {
					return;
				}
			}
		}

		// Too many parameters to ever call this with all of them - they should use scoped vars on start instead
		// Subtract 2: one for func name, one for repeat count, since last function will be a KRFunc
		if (params.length > this.numParamsToAllow) {
			return this.CreateError(`Template_SetFuncSignature: Parsed "${funcSigBreakdown[2]}" function signature has too many parameters to run via expression. ` +
				`You have ${params.length} parameters, but max is ${this.numParamsToAllow}. Consider using scoped vars instead.`);
		}

		let func = null;
		let funcExisting = this.globals.functionTemplates.find(f => { return f.nameL == funcNameL; });
		if (funcExisting == null) {
			func = new DarkScript_FunctionTemplate(this, funcSigBreakdown[2], delayable, repeatable, recursable != 0, returnTypeValid);
			this.globals.functionTemplates.push(func);
		}
		else {
			func = funcExisting;
			func.delaying = delayable;
			func.repeating = repeatable;
			func.recursiveAllowed = recursable != 0;
			func.defaultReturnValue = new DarkScript_Value(returnTypeValid);
		}
		func.params = params;
	};
	Template_SetDefaultReturnN(funcName) {
		const funcExisting = this.Sub_GetFuncTemplateByName("Template_SetDefaultReturnN", funcName);
		if (funcExisting == null) {
			return;
		}

		const val = funcExisting.defaultReturnValue;
		val.data = null;
		val.dataSize = 0;
		val.type = DarkScript_Type.Any;
	};
	Template_SetDefaultReturnI(funcName, value) {
		const funcExisting = this.Sub_GetFuncTemplateByName("Template_SetDefaultReturnI", funcName);
		if (funcExisting == null) {
			return;
		}

		const val = funcExisting.defaultReturnValue;
		val.data = value;
		val.dataSize = 4; // sizeof int
		val.type = DarkScript_Type.Integer;
	};
	Template_SetDefaultReturnF(funcName, value) {
		const funcExisting = this.Sub_GetFuncTemplateByName("Template_SetDefaultReturnF", funcName);
		if (funcExisting == null) {
			return;
		}

		const val = funcExisting.defaultReturnValue;
		val.data = value;
		val.dataSize = 4; // sizeof float
		val.type = DarkScript_Type.Float;
	};
	Template_SetDefaultReturnS(funcName, newVal) {
		const funcExisting = this.Sub_GetFuncTemplateByName("Template_SetDefaultReturnS", funcName);
		if (funcExisting == null) {
			return;
		}

		const val = funcExisting.defaultReturnValue;
		val.data = newVal;
		val.dataSize = newVal.length + 1;
		val.type = DarkScript_Type.String;
	};
	Template_Param_SetDefaultValueI(funcName, paramName, paramValue, useTheAnyType) {
		const f = this.Sub_GetFuncTemplateByName("Template_Param_SetDefaultValueI", funcName);
		if (!f) {
			return;
		}

		const p = this.Sub_GetTemplateParam("Template_Param_SetDefaultValueI", f, paramName);
		if (!p) {
			return;
		}
		const val = p.defaultVal;
		val.data = paramValue;
		val.dataSize = 4; //sizeof(int);
		val.type = DarkScript_Type.Integer;
		p.type = useTheAnyType ?  DarkScript_Type.Any : DarkScript_Type.Integer;
	};
	Template_Param_SetDefaultValueF(funcName, paramName, paramValue, useTheAnyType) {
		const f = this.Sub_GetFuncTemplateByName("Template_Param_SetDefaultValueF", funcName);
		if (!f) {
			return;
		}

		const p = this.Sub_GetTemplateParam("Template_Param_SetDefaultValueF", f, paramName);
		if (!p) {
			return;
		}
		const val = p.defaultVal;
		val.data = paramValue;
		val.dataSize = 4; //sizeof(float);
		val.type = DarkScript_Type.Float;
		p.type = useTheAnyType ?  DarkScript_Type.Any : DarkScript_Type.Float;
	};
	Template_Param_SetDefaultValueS(funcName, paramName, paramValue, useTheAnyType) {
		const f = this.Sub_GetFuncTemplateByName("Template_Param_SetDefaultValueS", funcName);
		if (!f) {
			return;
		}

		const p = this.Sub_GetTemplateParam("Template_Param_SetDefaultValueS", f, paramName);
		if (!p) {
			return;
		}
		const val = p.defaultVal;
		val.data = paramValue;
		val.dataSize = paramValue.length + 1;
		val.type = DarkScript_Type.String;
		p.type = useTheAnyType ?  DarkScript_Type.Any : DarkScript_Type.String;
	};
	Template_Param_SetDefaultValueN(funcName, paramName, useTheAnyType) {
		const f = this.Sub_GetFuncTemplateByName("Template_Param_SetDefaultValueN", funcName);
		if (!f) {
			return;
		}

		const p = this.Sub_GetTemplateParam("Template_Param_SetDefaultValueN", f, paramName);
		if (!p) {
			return;
		}
		const val = p.defaultVal;
		val.data = null;
		val.dataSize = 0;
		val.type = DarkScript_Type.Any;

		// Leave as whatever type it was otherwise
		if (useTheAnyType != 0) {
			p.type = DarkScript_Type.Any;
		}
	};
	Template_SetScopedVarOnStartI(funcName, varName, paramValue, overrideWhenRecursing) {
		if ((overrideWhenRecursing & 1) != overrideWhenRecursing) {
			return this.CreateError("Couldn't set scoped var on start; \"override when recursing\" must be 0 or 1, you supplied %i.", overrideWhenRecursing);
		}
	
		const p = this.Sub_GetOrCreateTemplateScopedVar("Template_SetScopedVarOnStartI", funcName, varName);
		if (!p) {
			return;
		}
	
		p.recursiveOverride = overrideWhenRecursing != 0;
		const val = p.defaultVal;
		val.data = paramValue;
		val.dataSize = 4; //sizeof(int);
		val.type = DarkScript_Type.Integer;
	};
	Template_SetScopedVarOnStartF(funcName, varName, paramValue, overrideWhenRecursing) {
		if ((overrideWhenRecursing & 1) != overrideWhenRecursing) {
			return this.CreateError("Couldn't set scoped var on start; \"override when recursing\" must be 0 or 1, you supplied %i.", overrideWhenRecursing);
		}
	
		const p = this.Sub_GetOrCreateTemplateScopedVar("Template_SetScopedVarOnStartF", funcName, varName);
		if (!p) {
			return;
		}
	
		p.recursiveOverride = overrideWhenRecursing != 0;
		const val = p.defaultVal;
		val.data = paramValue;
		val.dataSize = 4; //sizeof(float);
		val.type = DarkScript_Type.Float;
	};
	Template_SetScopedVarOnStartS(funcName, varName, varValue, overrideWhenRecursing) {
		if ((overrideWhenRecursing & 1) != overrideWhenRecursing) {
			return this.CreateError("Couldn't set scoped var on start; \"override when recursing\" must be 0 or 1, you supplied %i.", overrideWhenRecursing);
		}
	
		const p = this.Sub_GetOrCreateTemplateScopedVar("Template_SetScopedVarOnStartS", funcName, varName);
		if (!p) {
			return;
		}
	
		p.recursiveOverride = overrideWhenRecursing != 0;
		const val = p.defaultVal;
		val.data = varValue;
		val.dataSize = varValue.length + 1; //sizeof(float);
		val.type = DarkScript_Type.Float;
	};
	Template_CancelScopedVarOnStart(funcName, varName) {
		/** @type DarkScript_FunctionTemplate */
		const funcTemplate = this.Sub_GetFuncTemplateByName("Template_CancelScopedVarOnStart", funcName);
		if (!funcTemplate) {
			return;
		}
		if (varName.length == 0) {
			return this.CreateError("%s: scoped var name is blank.", "Template_CancelScopedVarOnStart");
		}

		const varNameL = this.ToLower(varName);
		const scopedVarIt = funcTemplate.scopedVarOnStart.findIndex(
			function(s) { return s.nameL == varNameL; });
		// already removed
		if (scopedVarIt == -1) {
			return;
		}
		funcTemplate.scopedVarOnStart.splice(scopedVarIt, 1);
	};
	Template_SetGlobalID(funcName, globalIDToRunOn) {
		const f = this.Sub_GetFuncTemplateByName("Template_SetGlobalID", funcName);
		if (!f) {
			return;
		}

		const key = "DarkScript" + globalIDToRunOn;
		const gd = this.Runtime.ReadGlobal(key);
		if (gd == null) {
			return this.CreateError("Couldn't set global ID to \"%s\" for function template \"%s\", no matching extension with that global ID found.", globalIDToRunOn, funcName);
		}
		if (gd.exts.length == 0) {
			return this.CreateError("Couldn't set global ID to \"%s\" for function template \"%s\", global ID found but no extension using it currently.", globalIDToRunOn, funcName);
		}
		if (gd.exts.length > 1) {
			this.CreateError("Warning: global ID \"%s\" has more than one extension owning it. The first created one will be used.", globalIDToRunOn);
		}
		f.globalID = globalIDToRunOn;
		f.ext = gd.exts[0];
	};
	Template_SetEnabled(funcName, funcEnabled) {
		if ((funcEnabled & 1) != funcEnabled) {
			return this.CreateError("Couldn't set function \"%s\" enabled; parameter should be 0 or 1, and was %d.", funcName, funcEnabled);
		}
	
		const f = this.Sub_GetFuncTemplateByName("Template_SetEnabled", funcName);
		if (!f) {
			return;
		}
		f.isEnabled = funcEnabled != 0;
	};
	Template_RedirectFunction(funcName, redirectFuncName) {
		// Both blank, or both explicit func names that match
		if (funcName.toLowerCase() == redirectFuncName.toLowerCase())
			return this.CreateError("Couldn't set function \"%s\" to redirect to \"%s\"; same function.", funcName, redirectFuncName);

		const f = this.Sub_GetFuncTemplateByName("Template_RedirectFunction", funcName),
			f2 = this.Sub_GetFuncTemplateByName("Template_RedirectFunction", redirectFuncName);
		if (!f || !f2)
			return;

		// Pulled running func with blank, and it matches the explicit func name
		if (f.nameL == f2.nameL)
			return this.CreateError("Couldn't set function \"%s\" to redirect to \"%s\"; same function.", funcName, redirectFuncName);

		// If you decide to allow anonymous function template redirection, modify the if() in VariableFunction
		f.redirectFunc = f2.name;
		f.redirectFuncPtr = f2;
	};
	Template_Loop(loopName) {
		if (loopName.length == 0) {
			return this.CreateError("%s: Empty loop name not allowed.", "Template_Loop");
		}
		if (this.curLoopName.length != 0) {
			return this.CreateError("%s: Can't run two internal loops at once; already running loop" +
				" \"%s\", can't run loop \"%s\".", "Template_Loop", this.curLoopName, loopName);
		}
	
		this.internalLoopIndex = 0;
		for (const f of this.globals.functionTemplates) {
			this.curFuncTemplateLoop = f;
			this.Runtime.GenerateEvent(7);
			++this.internalLoopIndex;
		}
	
		this.curFuncTemplateLoop = null;
		this.internalLoopIndex = -1;
	};
	DelayedFunctions_Loop(loopName) {
		if (loopName.length == 0) {
			return this.CreateError("%s: Empty loop name not allowed.", "DelayedFunctions_Loop");
		}
		if (this.curLoopName.length != 0) {
			return this.CreateError("%s: Can't run two internal loops at once; already running " + 
				"loop \"%s\", can't run loop \"%s\".", "DelayedFunctions_Loop", this.curLoopName, loopName);
		}
	
		this.internalLoopIndex = 0;
		for (const f of this.globals.pendingFuncs) {
			this.curDelayedFuncLoop = f;
			this.Runtime.GenerateEvent(8);
			++this.internalLoopIndex;
		}
	
		this.curDelayedFuncLoop = null;
		this.internalLoopIndex = -1;
	};
	DelayedFunctions_CancelByPrefix(funcName) {
		if (this.globals.pendingFuncs.length == 0) {
			return;
		}
		if (funcName.length == 0) {
			return this.globals.pendingFuncs.length = 0;
		}
	
		const funcNameL = this.ToLower(funcName);
		this.globals.pendingFuncs = this.globals.pendingFuncs.filter(f => f.funcToRun.funcTemplate.nameL.startsWith(funcNameL));
	};
	RunFunction_ActionDummy_Num(result) {
		// Ignore params, this action just allows user to run an expression's functions.
	};
	RunFunction_ActionDummy_String(result) {
		// Ignore params, this action just allows user to run an expression's functions.
	};
	RunFunction_Foreach_Num(_obj, _dummy) {
		// No expression executed when reading the "int dummy" used for this action, so we don't know what func to run foreach on
		if (this.foreachFuncToRun == null) {
			return this.CreateError("%s: Did not find any expression-function that was run in the foreach action.", "RunFunction_Foreach_Num");
		}

		// We store temp copies of the current foreach function, so a Foreach expression
		// can run a Foreach itself without corrupting.
		const funcToRun = this.foreachFuncToRun;
		this.foreachFuncToRun = null;

		// Ignore the RunObject * passed and look up OIList index. This is necessary due to the passed object being
		// a qualifier, which we can't tell from reading the RunObject *. However, the qualifier OI number is
		// hard-coded as a variable in the event sheet, and you can't modify which object is in a parameter live,
		// so we can read it back from the event sheet directly.
		const  oil = this.Runtime.GetOIListIndexFromObjectParam(0);

		this.globals.runningFuncs.push(funcToRun);
		this.Sub_RunPendingForeachFunc(oil, funcToRun);
		this.globals.runningFuncs.splice(this.globals.runningFuncs.length - 1);

		// The RunObject * passed to Foreach_Num ^ points to the first instance of the selected objects
		// this action has in its parameters.
		// If you want your action to be called repeatedly, once for each object instance,
		// you just run as if the RunObject * is the only instance this action is being run on, and the
		// Fusion runtime will cycle through each selected object, changing the RunObject * passed by the event.
		// Otherwise, you cancel this behaviour, and loop the selection yourself.
		// 
		// We want fine-tuned control over loop behaviour - qualifier <-> singular, and stopping loops
		// midway - so we do it ourselves.
		// We run this at end of the action, after foreach, as the generated events may cause the cancel
		// to be overwritten otherwise.
		this.Runtime.CancelRepeatingObjectAction();
	};
	RunFunction_Foreach_String(_obj, _dummy) {
		// No expression executed when loading the "_dummy" used for this action, we don't know what func to run foreach on
		if (this.foreachFuncToRun == null) {
			return this.CreateError("%s: Did not find any expression-function that was run in the foreach action.", "RunFunction_Foreach_String");
		}

		// We store temp copies, so a Foreach expression's can run a Foreach itself without corrupting
		const funcToRun = this.foreachFuncToRun;
		this.foreachFuncToRun = null;
		const oil = this.Runtime.GetOIListIndexFromObjectParam(0);

		this.globals.runningFuncs.push(funcToRun);
		this.Sub_RunPendingForeachFunc(oil, funcToRun);
		this.globals.runningFuncs.splice(this.globals.runningFuncs.length - 1);

		// For an explanation of this flag, see RunFunction_Foreach_Num()
		this.Runtime.CancelRepeatingObjectAction();
	};
	RunFunction_Delayed_Num_MS(_timeFirst, _numRepeats, _timeSubsequent, _crossFrames, _funcDummy) {
		// This action does nothing. Actual handling of this parameters happens when funcDummy is being evaluated, and VariableFunction() is called.
		// The delayed function made by this action is handled under handleRunObject().
	};
	RunFunction_Delayed_String_MS(_timeFirst, _numRepeats, _timeSubsequent, _crossFrames, _funcDummy) {
		// This action does nothing. Actual handling of this parameters happens when funcDummy is being evaluated, and VariableFunction() is called.
		// The delayed function made by this action is handled under handleRunObject().
	};
	RunFunction_Delayed_Num_Ticks(_timeFirst, _numRepeats, _timeSubsequent, _crossFrames, _funcDummy) {
		// This action does nothing. Actual handling of this parameters happens when funcDummy is being evaluated, and VariableFunction() is called.
		// The delayed function made by this action is handled under handleRunObject().
	};
	RunFunction_Delayed_String_Ticks(_timeFirst, _numRepeats, _timeSubsequent, _crossFrames, _funcDummy) {
		// This action does nothing. Actual handling of this parameters happens when funcDummy is being evaluated, and VariableFunction() is called.
		// The delayed function made by this action is handled under handleRunObject().
	};
	RunFunction_Script(script) {
		// TODO: Create If, Else etc sort of functions, + operators, nested funcs, etc
		let funcCallMatcher = new RegExp(String.raw`([^\s(]+)\s*\(((?:\s*(?:[^",]+|(?:"(?:(?=(?:\\?)).)*?"))?,\s*)*(?:\s*(?:[^",]+|(?:"(?:(?=(?:\\?)).)*?"))))?\)`);
		let funcCallBreakdown = funcCallMatcher.exec(script);
		if (funcCallBreakdown == null) {
			return this.CreateError("%s: Function script \"%s\" not parseable.", "", script);
		}
	
		let funcName = funcCallBreakdown[1], redirectedFrom = "";
		let funcNameL = this.ToLower(funcName);
		// Make sure it's not a KRFuncXXX() function name. That would work, but the script engine
		// (used in "run text as script") will find it ambiguous.
		const isKRFunc = new RegExp("(k?r?f?)func(?:[fis]*)(?:\\$?)");
		let funcNameIsParam = false, repeatCountExists = false, keepSelection = false;
		let isKRFuncCallBreakdown = isKRFunc.exec(funcNameL);
		if (isKRFuncCallBreakdown != null)
		{
			const prefix = isKRFuncCallBreakdown[1];
			const params = isKRFuncCallBreakdown[2];
			//const std::tstring paramTypes = isKRFuncCallBreakdown[3].str();
			if (prefix.indexOf('k') != -1) {
				keepSelection = true;
			}
			if (prefix.indexOf('r') != -1) {
				repeatCountExists = true;
			}
			// TODO: We currently ignore param types, we could instead use it as a reference for what
			// an otherwise ambiguous parameter is, e.g. int -> float. But with allowing f suffix, that
			// seems unnecessary.
			funcNameIsParam = true;
		}
	
		let info = "Script run of " + funcName;
	
		let values = [];
		let repeatCount = 0;
		/** @type DarkScript_FunctionTemplate */
		let funcTemplate;
	
		const FindFuncOrDie = () => {
			// Match by name
			const funcTemplateIt = this.globals.functionTemplates.find(f => { return f.nameL == funcNameL; });
	
			if (funcTemplateIt == null) {
				if (this.funcsMustHaveTemplate) {
					return this.CreateError("%s: Function script uses function name \"%s\", which has no template.", "RunFunction_Script", funcName), false;
				}
	
				funcTemplate = new DarkScript_FunctionTemplate(this, funcName, DarkScript_Expected.Either, DarkScript_Expected.Either, false, DarkScript_Type.Any);
				let name = 'a';
				// allow 6 params, we'll delete excess ones later
				for (let i = 0; i < 6; ++i, name = String.fromCharCode(name.charCodeAt(0) + 1)) {
					// Note the third part gives variable names a, b, c
					funcTemplate.params.push(new DarkScript_Param(name, DarkScript_Type.Any));
				}
				this.lastReturn = new DarkScript_Value(DarkScript_Type.Any);
				funcTemplate.isAnonymous = true;
			}
			else {
				// Disabling takes priority over possible redirect
				if (!funcTemplate.isEnabled) {
					this.lastReturn = funcTemplate.defaultReturnValue;
					if (funcTemplate.defaultReturnValue.type == DarkScript_Type.Any) {
						this.CreateError("%s: Function script uses function name \"%s\", which is set to disabled, and has no default return value.", "RunFunction_Script", funcName);
					}
					console.debug(`Script function "${funcName}" is disabled; returning default return value to lastReturn.`);
					return false;
				}
	
				if (funcTemplateIt.redirectFunc.length == 0) {
					funcTemplate = funcTemplateIt;
				}
				else {
					funcTemplate = funcTemplateIt.redirectFuncPtr;
					console.debug("Script redirecting from function \"%s\" to \"%s\".", funcName, funcTemplateIt.redirectFunc);
					redirectedFrom = funcName;
					funcName = funcTemplate.name;
					funcNameL = funcTemplate.nameL;
	
					// Disabling the redirected function
					if (!funcTemplate.isEnabled) {
						this.lastReturn = funcTemplate.defaultReturnValue;
						if (funcTemplate.defaultReturnValue.type == DarkScript_Type.Any) {
							this.CreateError("%s: Function script uses function name \"%s\", which was redirected to function \"%s\", which is set to disabled, and has no default return value.",
								"RunFunction_Script", redirectedFrom, funcName);
						}
						console.debug("Redirected script function \"%s\" is disabled; returning default return value to lastReturn.", funcName);
						return false;
					}
				}
	
				if (funcTemplate.delaying == DarkScript_Expected.Always) {
					return this.CreateError("%s: Function script uses function name \"%s\", which is expected to be called delayed only.", "RunFunction_Script", funcName), false;
				}
				if (funcTemplate.repeating == DarkScript_Expected.Always) {
					return this.CreateError("%s: Function script uses function name \"%s\", which is expected to be called repeating only.", "RunFunction_Script", funcName), false;
				}
			}
	
			return true;
		};
	
		if (!funcNameIsParam && !FindFuncOrDie()) {
			return;
		}
		if (funcTemplate == null)
			return; // shut up intellisense
	
		if (funcCallBreakdown[2] != null) {
			let writeTo = new DarkScript_Value(DarkScript_Type.Any);
			// Regex is dumb and for repeating groups, only the last one will be considered;
			// so we re-run a regex on the parameter list, looping all matches.
			// .NET regex does capture repeating groups nicely, but no other regex engines support it.
			const paramListParser = new RegExp(String.raw`\s*([^",]+|(?:"(?:(?=(?:\\?)).)*?"))\s*,`);
			const paramList = funcCallBreakdown[2] + ',';
	
			// We backtrack 1 because regex is 1-based, but C++ params vector is 0-based. We use this to skip KR func parameters.
			const funcParamIndex = 1 + (funcNameIsParam ? (repeatCountExists ? 2 : 1) : 0);
			let j = 0;
			let paramListResults = paramListParser.exec(paramList);
			if (paramListResults == null) {
				return this.CreateError("Couldn't run function \"%s\" from script, param value at index %zu was empty or whitespace.",
					j, funcName.length == 0 ? "<unknown>" : funcName, j - 1);
			}
			for (const i of paramListResults) {
				++j;
				let valueTextToParse = i[1];
				if (valueTextToParse.length == 0) {
					return this.CreateError("Couldn't run function \"%s\" from script, param value at index %zu was empty or whitespace.",
						j, funcName.length == 0 ? "<unknown>" : funcName, j - 1);
				}
	
				// Func type of run, where instead of Bob("a",2), you run Func("Bob", "a", 2),
				// or possibly RFunc("Bob", repeat count, "a", 2)
				if (funcNameIsParam) {
					// func name, 1st param
					if (j == 1) {
						if (valueTextToParse[0] != '"' || valueTextToParse[valueTextToParse.length - 1] != '"' || valueTextToParse.length <= 2) {
							return this.CreateError("Couldn't run KR-function script,  function name argument [%s] is not in quotes, or not a valid function name.", j, valueTextToParse);
						}
						funcName = valueTextToParse.substring(1, valueTextToParse.length - 2);
						funcNameL = this.ToLower(funcName);
						if (/[\"\r\n',\\]/.test(funcName)) {
							return this.CreateError("Couldn't run KR-function script, function name argument \"%s\" uses forbidden characters.", valueTextToParse);
						}
						if (!FindFuncOrDie()) {
							return; // error reported already
						}
						continue;
					}
					// repeat count, 2nd param
					else if (j == 2 && repeatCountExists) {
						const d = parseInt(valueTextToParse);
						// Returns 0 and endPos should be set to start pos if failed to convert
						// If out of bounds of long, should return ERANGE
						// If 64-bit long, make sure value within is in bounds of 32-bit long
						if (d == null) {
							return this.CreateError("%s: Couldn't run function %s from script, has invalid repeat count \"%s\".",
								"RunFunction_Script", funcName, valueTextToParse);
						}
						repeatCount = d;
						continue;
					}
				}
				// This works for anonymous parameters as well, as we pre-init params to size 6 above
				if (values.length >= funcTemplate.params.length) {
					return this.CreateError("Couldn't run function %s from script; only expects %zu arguments, but script has extra passed; " +
						"starting from index %zu, [%.20s].",
						funcName, funcTemplate.params.length, i, j, i.substring(0, 21));
				}
	
				if (!this.Sub_ParseParamValue(info, valueTextToParse,
					funcTemplate.params[j - funcParamIndex], j - funcParamIndex, (t) => writeTo = t)) {
					return; // error already reported by Sub_ParseParamValue
				}
				values.push(writeTo);
			}
			if (funcNameIsParam && j < (repeatCountExists ? 2 : 1)) { // no params supplied, but we need them because it's KR style
				return this.CreateError("KR-function argument list is incomplete; expected function name%s.", repeatCountExists ? ", repeat count" : "");
			}
		}
		else if (funcNameIsParam) { // no params supplied, but we need them because it's KR style
			return this.CreateError("KR-function argument list is incomplete; expected function name%s.", repeatCountExists ? ", repeat count" : "");
		}
	
		const numPassedExpFuncParams = values.length;
		if (!funcTemplate.isAnonymous) {
			// No default value, and none provided
			while (values.length < funcTemplate.params.length) {
				if (funcTemplate.params[values.length].defaultVal.type == DarkScript_Type.Any) {
					return this.CreateError("Couldn't run function %s from script; didn't pass a value for parameter \"%s\" (index %zu), and no default value in the template.",
						funcName, funcTemplate.params[values.length].name, values.length);
				}
				values.push(funcTemplate.params[values.length - 1].defaultVal);
			}
		}
		
		const runningFunc = new DarkScript_RunningFunction(funcTemplate, true, repeatCount - 1);
		for (let i = 0; i < values.length; ++i) {
			runningFunc.paramValues[i] = values[i];
		}
		runningFunc.numPassedParams = numPassedExpFuncParams;
		runningFunc.keepObjectSelection = keepSelection;
		runningFunc.isVoidRun = true;
		runningFunc.redirectedFromFunctionName = redirectedFrom;
	
		/** @type DarkScript_FusionSelectedObjectListCache[] */
		let selObjList = [];
		this.evt_SaveSelectedObjects(selObjList);
		runningFunc.runLocation = this.Sub_GetLocation(27);
		console.assert(this.$actionFuncs[27] == this.RunFunction_Script);
		this.ExecuteFunction(null, runningFunc);
		this.evt_RestoreSelectedObjects(selObjList, true);
	};
	/**
	 * 
	 * @param {string} loopName Fusion dev's DarkScript loop name
	 * @param {number} includeInherited Whether to include inherited scoped variables, or just topmost
	 * @returns void
	 */
	RunningFunc_ScopedVar_Loop(loopName, includeInherited) {
		if (loopName.length == 0) {
			return this.CreateError("%s: Empty loop name not allowed.", "RunningFunc_ScopedVar_Loop");
		}
		if (this.curLoopName.length != 0) {
			return this.CreateError("%s: Can't run two internal loops at once; already running loop \"%s\", can't run loop \"%s\".", "RunningFunc_ScopedVar_Loop", this.curLoopName, loopName);
		}
		if ((includeInherited & 1) != includeInherited) {
			return this.CreateError("%s: Can't run scoped variables loop; \"include inherited\" parameter was %d, not 0 or 1.", "RunningFunc_ScopedVar_Loop", includeInherited);
		}
	
		const rf = this.Sub_GetRunningFunc("RunningFunc_ScopedVar_Loop", "");
		if (!rf) {
			return;
		}
	
		this.internalLoopIndex = 0;
		for (const s of this.globals.scopedVars) {
			if (includeInherited == 0 && s.level != this.globals.runningFuncs.length) {
				continue;
			}
			this.curScopedVarLoop = s;
			this.Runtime.GenerateEvent(10);
			++this.internalLoopIndex;
		}
		this.curScopedVarLoop = null;
		this.internalLoopIndex = -1;
	};
	RunningFunc_SetReturnI(value) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_SetReturnI", "");
		if (!rf) {
			return;
		}

		let val = rf.returnValue;
		// TODO: Complain or ignore delayed func return types?
		// Delayed func return types will be Any.
		if (val.type != DarkScript_Type.Integer) {
			let typeName = "no return value";
			if (val.type != DarkScript_Type.Any) {
				typeName = this.TypeToString(val.type) + " return type";
			}
			return this.CreateError("Can't return type %s from function %s, expected %s.",
				"integer", rf.funcTemplate.name, typeName);
		}

		val.data = value;
		val.dataSize = 4;
		val.type = DarkScript_Type.Integer;
	};
	RunningFunc_SetReturnF(value) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_SetReturnF", "");
		if (!rf) {
			return;
		}

		let val = rf.returnValue;
		// TODO: Complain or ignore delayed func return types?
		// Delayed func return types will be Any.
		if (val.type != DarkScript_Type.Float) {
			let typeName = "no return value";
			if (val.type != DarkScript_Type.Any) {
				typeName = this.TypeToString(val.type) + " return type";
			}
			return this.CreateError("Can't return type %s from function %s, expected %s.",
				"float", rf.funcTemplate.name, typeName);
		}

		val.data = value;
		val.dataSize = 4;
		val.type = DarkScript_Type.Float;
	};
	RunningFunc_SetReturnS(value) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_SetReturnS", "");
		if (!rf) {
			return;
		}

		let val = rf.returnValue;
		// TODO: Complain or ignore delayed func return types?
		// Delayed func return types will be Any.
		if (val.type != DarkScript_Type.String) {
			let typeName = "no return value";
			if (val.type != DarkScript_Type.Any) {
				typeName = this.TypeToString(val.type) + " return type";
			}
			return this.CreateError("Can't return type %s from function %s, expected %s.",
				"string", rf.funcTemplate.name, typeName);
		}

		val.data = value;
		val.dataSize = value.length + 1;
		val.type = DarkScript_Type.String;
	};
	RunningFunc_ScopedVar_SetI(paramName, newVal) {
		/** @type DarkScript_ScopedVar */
		let param = null;
		let val = this.Sub_CheckScopedVarAvail("RunningFunc_ScopedVar_SetI", paramName, DarkScript_Expected.Either, false, (p) => param = p);
		if (!val) {
			this.globals.scopedVars.push(new DarkScript_ScopedVar(paramName, DarkScript_Type.Integer, true, this.globals.runningFuncs.length));
			param = this.globals.scopedVars[this.globals.scopedVars.length - 1];
			val = this.globals.scopedVars[this.globals.scopedVars.length - 1].defaultVal;
		}

		// Check the type is int, or convertible to int
		if (param.type != DarkScript_Type.Integer && param.type != DarkScript_Type.Any) {
			return this.CreateError("%s: param/scoped var %s does not accept integer type.",
				"RunningFunc_ScopedVar_SetI", param.name);
		}

		val.data = newVal;
		val.dataSize = 4; // sizeof(newVal);
		val.type = DarkScript_Type.Integer;
	};
	RunningFunc_ScopedVar_SetF(paramName, newVal) {
		/** @type DarkScript_ScopedVar */
		let param = null;
		let val = this.Sub_CheckScopedVarAvail("RunningFunc_ScopedVar_SetF", paramName, DarkScript_Expected.Either, false, (p) => param = p);
		if (!val) {
			this.globals.scopedVars.push(new DarkScript_ScopedVar(paramName, DarkScript_Type.Float, true, this.globals.runningFuncs.length));
			param = this.globals.scopedVars[this.globals.scopedVars.length - 1];
			val = this.globals.scopedVars[this.globals.scopedVars.length - 1].defaultVal;
		}

		// Check the type is float, or convertible to float
		if (param.type != DarkScript_Type.Float && param.type != DarkScript_Type.Any) {
			return this.CreateError("%s: param/scoped var %s does not accept float type.",
				"RunningFunc_ScopedVar_SetF", param.name);
		}

		val.data = newVal;
		val.dataSize = 4; // sizeof(newVal);
		val.type = DarkScript_Type.Float;
	};
	RunningFunc_ScopedVar_SetS(paramName, newVal) {
		/** @type DarkScript_ScopedVar */
		let param = null;
		let val = this.Sub_CheckScopedVarAvail("RunningFunc_ScopedVar_SetS", paramName, DarkScript_Expected.Either, false, (p) => param = p);
		if (!val) {
			this.globals.scopedVars.push(new DarkScript_ScopedVar(paramName, DarkScript_Type.String, true, this.globals.runningFuncs.length));
			param = this.globals.scopedVars[this.globals.scopedVars.length - 1];
			val = this.globals.scopedVars[this.globals.scopedVars.length - 1].defaultVal;
		}

		// Check the type is string, or convertible to string
		if (param.type != DarkScript_Type.String && param.type != DarkScript_Type.Any) {
			return this.CreateError("%s: param/scoped var %s does not accept string type.",
				"RunningFunc_ScopedVar_SetS", param.name);
		}

		val.data = newVal;
		val.dataSize = newVal.length + 1;
		val.type = DarkScript_Type.String;
	};
	RunningFunc_Params_Loop(loopName, includeNonPassed) {
		if (loopName.length == 0) {
			return this.CreateError("%s: Empty loop name not allowed.", "RunningFunc_Params_Loop");
		}
		if (this.curLoopName.length != 0) {
			return this.CreateError("%s: Can't run two internal loops at once; already running loop \"%s\", can't run loop \"%s\".", "RunningFunc_Params_Loop", this.curLoopName, loopName);
		}
		if ((includeNonPassed & 1) != includeNonPassed) {
			return this.CreateError("%s: Can't run params loop; \"include non-passed\" parameter was %d, not 0 or 1.", "RunningFunc_Params_Loop", includeNonPassed);
		}
	
		const rf = this.Sub_GetRunningFunc("RunningFunc_Params_Loop", "");
		if (!rf) {
			return;
		}
	
		let i = 0;
		for (const p of rf.funcTemplate.params) {
			if (includeNonPassed == 0 && i++ > rf.numPassedParams) {
				break;
			}
			this.curParamLoop = p;
			this.Runtime.GenerateEvent(9);
			++this.internalLoopIndex;
		}
		this.curParamLoop = null;
		this.internalLoopIndex = -1;
	};
	RunningFunc_StopFunction(cancelCurrentIteration, cancelNextIterations, cancelForeach) {
		if ((cancelCurrentIteration & 1) != cancelCurrentIteration) {
			return this.CreateError("Use of StopFunction with an incorrect \"cancel current iteration\" parameter %i. Must be 0 or 1.", cancelCurrentIteration);
		}
		if ((cancelNextIterations & 1) != cancelNextIterations) {
			return this.CreateError("Use of StopFunction with an incorrect \"cancel next iterations\" parameter %i. Must be 0 or 1.", cancelNextIterations);
		}
		if ((cancelForeach & 1) != cancelForeach) {
			return this.CreateError("Use of StopFunction with an incorrect \"cancel foreach loop\" parameter %i. Must be 0 or 1.", cancelForeach);
		}
	
		const cancelCurrentB = cancelCurrentIteration != 0;
		const cancelNextB = cancelNextIterations != 0;
		const cancelForeachB = cancelForeach != 0;
		if (!cancelCurrentB && !cancelNextB && !cancelForeachB) {
			return this.CreateError("Use of StopFunction with all cancel parameters as 0. This will have no effect.");
		}
	
		const rf = this.Sub_GetRunningFunc("RunningFunc_StopFunction", "");
		if (!rf) {
			return;
		}
	
		if (cancelCurrentB) {
			rf.currentIterationTriggering = false; // stops instantly in On Function event; next On Function check returns false
		}
		if (cancelNextB) {
			rf.nextRepeatIterationTriggering = false; // stops after all On Function conds for this event loop finishes
		}
		if (cancelForeachB) {
			rf.foreachTriggering = false;
		}
	};
	RunningFunc_ChangeRepeatSetting(newRepeatIndex, newRepeatCount, ignoreExistingCancel) {
		if ((ignoreExistingCancel & 1) != ignoreExistingCancel) {
			return this.CreateError("Use of ChangeRepeatSetting with an incorrect \"ignore existing cancel\" parameter %i. Must be 0 or 1.", ignoreExistingCancel);
		}
	
		if (newRepeatIndex < 0) {
			return this.CreateError("Use of ChangeRepeatSetting with a new repeat index of %i; must be more or equal to 0.", newRepeatIndex);
		}
		if (newRepeatCount < 1) {
			return this.CreateError("Use of ChangeRepeatSetting with a new repeat count of %i; must be more or equal to 1.", newRepeatCount);
		}
		if (newRepeatIndex >= newRepeatCount) {
			return this.CreateError("Use of ChangeRepeatSetting with a new repeat index of %i, higher than new repeat count %i.", newRepeatIndex, newRepeatCount);
		}
	
		const rf = this.Sub_GetRunningFunc("RunningFunc_ChangeRepeatSetting", "");
		if (!rf || !rf.active) {
			return;
		}
	
		rf.index = newRepeatIndex;
		rf.numRepeats = newRepeatCount;
		if (!rf.nextRepeatIterationTriggering && ignoreExistingCancel == 0)
			rf.nextRepeatIterationTriggering = true;
	};
	RunningFunc_Abort(error, funcToUnwindTo) {
		if (error.length == 0) {
			return this.CreateError("Can't abort with an empty reason.");
		}
	
		const rf = this.Sub_GetRunningFunc("RunningFunc_Abort", "");
		if (!rf) {
			return;
		}
		if (!rf.active) {
			return this.CreateError("Can't abort twice. Second reason discarded.");
		}
	
		let unwindToRunningFuncIndex = 0;
		if (funcToUnwindTo.length != 0) {
			const funcToUnwindToL = this.ToLower(funcToUnwindTo);
			const q = this.globals.runningFuncs.findLastIndex(f => { return f.funcTemplate.nameL == funcToUnwindToL; });
			if (q == -1) {
				return this.CreateError("Aborting function \"%s\" to function \"%s\" failed; unwind function \"%s\" not in call stack.", rf.funcTemplate.name, funcToUnwindTo, funcToUnwindTo);
			}
			unwindToRunningFuncIndex = q;
			console.assert(unwindToRunningFuncIndex >= 0);
		}
	
		for (let i = this.globals.runningFuncs.length - 1; i >= unwindToRunningFuncIndex; --i) {
			const fa = this.globals.runningFuncs[i];
			fa.abortReason = error;
			fa.currentIterationTriggering = false;
			fa.nextRepeatIterationTriggering = false;
			fa.active = false;
	
			if (fa.returnValue.type == DarkScript_Type.Any) {
				fa.returnValue = fa.funcTemplate.defaultReturnValue;
				// No default return
				if (fa.returnValue.type == DarkScript_Type.Any) {
					this.CreateError("Warning: Aborted function \"%s\" from function \"%s\", Fusion event line %d, with no default return and no set return value.",
						fa.funcTemplate.name, rf.funcTemplate.name, darkEdif.GetCurrentFusionEventNum(this));
				}
			}
		}
	};
	Logging_SetLevel(funcNames, logLevel) {
		this.CreateError("Logging_SetLevel not implemented");
	};
	Template_ImportFromAnotherFrame(funcName, globalIDToImportFrom) {
		// I can't imagine a scenario where a function is already running and then needs to be imported.
		if (funcName.length == 0) {
			return this.CreateError("Can't import a blank function name.");
		}
		if (globalIDToImportFrom.length == 0) {
			return this.CreateError("Can't import from DarkScript with global ID \"\". A local import doesn't make sense.");
		}

		// Match by name
		const nameL = this.ToLower(funcName);
		if (this.globals.functionTemplates.find(f => f.nameL == nameL) != null) {
			return this.CreateError("Can't import function \"%s\" from global ID \"%s\"; a function template with that name already exists.", funcName, globalIDToImportFrom);
		}

		const key = "DarkScript"+ globalIDToImportFrom;
		const gd = this.Runtime.ReadGlobal(key);
		if (gd == null) {
			return this.CreateError("Couldn't import function template \"%s\" from global ID \"%s\", no matching extension with that global ID found.", funcName, globalIDToImportFrom);
		}
		const ft = gd.functionTemplates.find(f => f.nameL == nameL);
		if (ft == null)
			return this.CreateError("Can't import function \"%s\" from global ID \"%s\"; a function template with that name already exists.", funcName, globalIDToImportFrom);
		this.globals.functionTemplates.push(ft);
	};
	

	// ======================================================================================================
	// Conditions
	// ======================================================================================================
	
	AlwaysTrue() {
		return true;
	};
	LoopNameMatch(loopName) {
		// lazy man's case insensitive compare
		// TODO: curLoopName can be stored lowercase'd
		return this.curLoopName.toLowerCase() == loopName.toLowerCase();
	};

	// OnError() -> AlwaysTrue()
	
	OnFunction(name) {
		// We're running this in a dummy fashion; this may be used in future to make a dependency/call tree
		if (!this.selfAwareness) {
			console.info("OnFunction(\"%s\" %p) on event %i. Self aware = 0. Exiting.", name, name, darkEdif.GetCurrentFusionEventNum(this));
			return false;
		}
		console.assert(this.globals.runningFuncs.length != 0);
	
		let nameR = this.ToLower(name);
		let lowest = this.globals.runningFuncs[this.globals.runningFuncs.length - 1];
		console.info("OnFunction(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.",
			name, darkEdif.GetCurrentFusionEventNum(this), lowest.funcTemplate.name, lowest.currentIterationTriggering ? 1 : 0);
	
		if (lowest.funcTemplate.nameL != nameR || !lowest.currentIterationTriggering) {
			return false;
		}
		if (lowest.keepObjectSelection) {
			this.evt_RestoreSelectedObjects(lowest.selectedObjects, true);
		}
		lowest.eventWasHandled = true;
		return true;
	};
	OnForeachFunction(name, objOiList) {
		// We're running this in a dummy fashion; this may be used in future to make a dependency/call tree
		if (!this.selfAwareness) {
			console.info("OnForeachFunction(\"%s\") on event %i. Self aware = 0. Exiting.", name, darkEdif.GetCurrentFusionEventNum(this));
			return false;
		}
		console.assert(this.globals.runningFuncs.length != 0);
		let nameR = this.ToLower(name);
		let lowest = this.globals.runningFuncs[this.globals.runningFuncs.length - 1];
		console.info("OnForeachFunction(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.",
			name, darkEdif.GetCurrentFusionEventNum(this), lowest.funcTemplate.name, lowest.currentIterationTriggering ? 1 : 0);
	
		if (lowest.funcTemplate.nameL != nameR || !lowest.currentIterationTriggering) {
			return false;
		}
	
		// Since it's a foreach
		let ro = this.Runtime.RunObjPtrFromFixed(lowest.currentForeachObjFV);
		console.assert(ro != null);
		const ho = ro.get_rHo();
		const actionExptOi = lowest.currentForeachOil;
	
		// For conditions, object parameters are passed as EventParam *, a ParamObject *, to exts;
		// but DarkEdif reads the OIList num from it and passes that instead, as an int.
		const conditionExptOi = objOiList;
	
		// Oi doesn't match, so the condition's selected object does not match the Foreach action.
		// If an oi >= 0, was on object, otherwise qualifier.
		// If a singular condition does not match the singular action, or both are qualifiers and don't match,
		// then we don't trigger.
		if (conditionExptOi != actionExptOi && ((((conditionExptOi & 0x8000) != 0) == ((actionExptOi & 0x8000) != 0)) ||
			// action is actionExptOi < 0, so action was run on qualifier. oi >= 0, condition is a singular.
			((actionExptOi & 0x8000) != 0 && !this.allowQualifierToTriggerSingularForeach) ||
			// actionExptOi >= 0, so action was run on singular. oi < 0, condition is a qualifier.
			((actionExptOi & 0x8000) == 0 && !this.allowSingularToTriggerQualifierForeach))) {
			console.info("OnForeachFunction(\"%s\") on event %i. Oi %i does not match expected Oi %i.",
				name, darkEdif.GetCurrentFusionEventNum(this), conditionExptOi, actionExptOi);
	
			// If we just return false from a qualifier event, Fusion crashes.
			// We have to deselect all the qualifier entries, then return false.
			for (const oi of new darkEdif.QualifierOIListIterator(this.rhPtr, actionExptOi, darkEdif.Selection.All)) {
				oi.SelectNone(this.rhPtr);
			}
	
			return false;
		}
	
		// Condition selected a singular object
		if ((conditionExptOi & 0xF000) == 0) {
			// It doesn't match, so we can assume actionExptOi is a qualifier - or the earlier if would've killed it
			if (conditionExptOi != actionExptOi) {
				let poil = this.rhPtr.GetOIListByIndex(conditionExptOi);
				if (poil.get_Oi() != ho.get_Oi())
				{
					console.info("OnForeachFunction(\"%s\") on event %i. Expected FV is %i, Number = %i; does not equal passed FV is %i, number %i. Exiting.",
						name, darkEdif.GetCurrentFusionEventNum(this),
						lowest.currentForeachObjFV, (lowest.currentForeachObjFV & 0xFFFF),
						ho.GetFixedValue(), ho.get_Number());
	
					return false;
				}
				console.info("OnForeachFunction(\"%s\") on event %i. Fall through. Continuing...",
					name, darkEdif.GetCurrentFusionEventNum(this));
			}
	
			console.info("OnForeachFunction(\"%s\") on event %i. Expected FV is %i, Number = %i, Oi %i; equals passed FV is %i, number %i. Continuing...",
				name, darkEdif.GetCurrentFusionEventNum(this),
				lowest.currentForeachObjFV, (lowest.currentForeachObjFV & 0xFFFF),
				ho.get_Oi(), ho.GetFixedValue(), ho.get_Number());
	
			// TODO: we may not need to select none to cancel this event.
			// However, it does seem that in some OR scenarios, not deselecting everything causes the event to proceed
			// as if all objects matched the selection.
			if (!lowest.currentIterationTriggering) {
				this.ObjectSelection.selectNone(conditionExptOi);
			}
			else {
				this.ObjectSelection.selectOneObject(ro);
			}
		}
		else { // condition has qualifier
			// Qualifier matches actionExptOi, or actionExptOi is a singular;
			// if actExptOi was not singular and mismatched, the earlier if would've killed it
			// We need to check the foreach's Oi is within the condition's qualifier Oi list
			let found = false;
	
			// For each object in qualifier
			for (const poil of new darkEdif.QualifierOIListIterator(this.rhPtr, conditionExptOi, darkEdif.Selection.All)) {
				poil.SelectNone(this.rhPtr);
	
				if (found) {
					continue;
				}
	
				if (ho.get_Oi() == poil.get_Oi()) {
					if (lowest.currentIterationTriggering) {
						this.ObjectSelection.selectOneObject(ro);
					}
					found = true;
	
					console.info("Found OI match: ro FV %i, ro OI %hi, its poil %p, name %s, matches poil OI %hi, %p, name %s.",
						ho.GetFixedValue(), ho.get_Oi(), ho.get_OiList(), ho.get_OiList().get_name(),
						poil.get_Oi(), poil, poil.get_name());
	
					// don't break loop, we want to select none for all objects in qualifier,
					// or it'll keep the ones we haven't selected none for as implicitly all selected
				}
				else {
					console.info("Haven't found OI match yet: ro FV %i, ro OI %hi, its poil %p, name %s, does not match poil OI %hi, %p, name %s.",
						ho.GetFixedValue(), ho.get_Oi(), ho.get_OiList(), ho.get_OiList().get_name(),
						poil.get_Oi(), poil, poil.get_name());
				}
			}
	
			if (!found) {
				console.info("OnForeachFunction(\"%s\") on event %i. Couldn't find FV %i (OI %hi) in qualifier %hi's OI list. Exiting.",
					name, darkEdif.GetCurrentFusionEventNum(this), lowest.currentForeachObjFV, ho.get_Oi(), conditionExptOi);
				return false;
			}
		}
	
		if (this['DebugMode']) {
			let numSel = this.ObjectSelection.getNumberOfSelected(actionExptOi);
		
			console.info(`OnForeachFunction("${name}") on event ${darkEdif.GetCurrentFusionEventNum(this)}. ` +
				`Current FV = ${lowest.currentForeachObjFV}; number ${lowest.currentForeachObjFV & 0xFFFF}. ` +
				`Event count ${this.rhPtr.GetRH2EventCount()}. Checking selection count: ${numSel}. Triggering.`);
		
			// Pre-restore any objects selected, if needed
			if (lowest.keepObjectSelection) {
				//assert(false);
				this.evt_RestoreSelectedObjects(lowest.selectedObjects, true);
			}
		}
	
		// ObjectSelection.SelectOneObject(ro);
		lowest.eventWasHandled = true;
		return lowest.currentIterationTriggering;
	};
	OnFunctionAborted(name) {
		// We're not aware
		if (!this.selfAwareness) {
			console.debug("OnFunctionAborted(\"%s\") on event %i. Self aware = 0. Exiting.", name, darkEdif.GetCurrentFusionEventNum(this));
			return false;
		}
		console.assert(this.globals.runningFuncs.length != 0 && !this.globals.runningFuncs[this.globals.runningFuncs.length - 1].active);
		const nameR = this.ToLower(name);
		const lowest = this.globals.runningFuncs[this.globals.runningFuncs.length - 1];
		console.debug("OnFunctionAborted(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.",
			name, darkEdif.GetCurrentFusionEventNum(this), lowest.funcTemplate.name, lowest.currentIterationTriggering ? 1 : 0);
		// We can assume the function has aborted if this is running.
		if (lowest.funcTemplate.nameL != nameR) {
			return false;
		}
		lowest.abortWasHandled = true;
		return true;
	};
	IsRunningFuncStillActive() {
		const rf = this.Sub_GetRunningFunc("IsRunningFuncStillActive", "");
		return rf && rf.currentIterationTriggering;
	};

	DoesFunctionHaveTemplate(name) {
		const nameR = this.ToLower(name);
		return this.globals.functionTemplates.find(f => f.nameL == nameR);
	};
	IsFunctionInCallStack(name) {
		const nameR = this.ToLower(name);
		return this.globals.runningFuncs.find(f => f.funcTemplate.nameL == nameR);
	};
	IsLastRepeatOfFunction(name) {
		const rf = this.Sub_GetRunningFunc("IsLastRepeatOfFunction", name);
		return rf && (rf.index >= rf.numRepeats || !rf.nextRepeatIterationTriggering);
	};

	// OnFuncTemplateLoop -> LoopNameMatch
	// OnPendingFunctionLoop -> LoopNameMatch
	// OnFuncParamLoop -> LoopNameMatch
	// OnScopedVarLoop -> LoopNameMatch

	Logging_OnAnyFunction() {
		// We're not aware
		if (!this.selfAwareness) {
			console.debug("Logging_OnAnyFunction() on event %i. Self aware = 0. Exiting.", darkEdif.GetCurrentFusionEventNum(this));
			return false;
		}
	
		console.debug("Logging_OnAnyFunction() on event %i. Self aware = 1. Triggering.", darkEdif.GetCurrentFusionEventNum(this));
		return true;
	};
	Logging_OnAnyFunctionCompletedOK() {
		// We're not aware
		if (!this.selfAwareness) {
			console.debug("Logging_OnAnyFunctionCompletedOK() on event %i. Self aware = 0. Exiting.", darkEdif.GetCurrentFusionEventNum(this));
			return false;
		}
	
		console.debug("Logging_OnAnyFunctionCompletedOK() on event %i. Self aware = 1. Triggering.", darkEdif.GetCurrentFusionEventNum(this));
		return true;
	};
	Logging_OnAnyFunctionAborted() {
		// We're not aware
		if (!this.selfAwareness) {
			console.debug("Logging_OnAnyFunctionAborted() on event %i. Self aware = 0. Exiting.", darkEdif.GetCurrentFusionEventNum(this));
			return false;
		}
	
		console.debug("Logging_OnAnyFunctionAborted() on event %i. Self aware = 1. Triggering.", darkEdif.GetCurrentFusionEventNum(this));
		return true;
	};

	// =============================
	// Expressions
	// =============================

	Logging_GetDarkScriptError() {
		return this.curError;
	};
	Logging_GetAbortReason() {
		// TODO: Shouldn't this be the called func's abort reason, not this one?
		const rf = this.Sub_GetRunningFunc("Logging_GetAbortReason", "");
		return rf ? rf.abortReason : "";
	};
	Logging_GetCurrentLog(clearLog) {
		const log = this.curLog;
		if (clearLog == 1) {
			this.curLog = "";
		}
		return log;
	};
	RunningFunc_GetRepeatIndex(funcNameOrBlank) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_GetRepeatIndex", funcNameOrBlank);
		return rf ? rf.index : -1;
	};
	RunningFunc_GetNumRepeatsLeft(funcNameOrBlank) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_GetNumRepeatsLeft", funcNameOrBlank);
		// Exclude current iteration: and yes, this calculation is correct
		return rf ? rf.numRepeats - rf.index : -1;
	};
	RunningFunc_GetNumRepeatsTotal(funcNameOrBlank) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_GetNumRepeatsTotal", funcNameOrBlank);
		return rf ? rf.numRepeats : -1;
	};
	RunningFunc_ForeachFV(funcNameOrBlank) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_ForeachFV", funcNameOrBlank);
		return rf ? rf.currentForeachObjFV : -1;
	};
	RunningFunc_NumParamsPassed(funcNameOrBlank) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_NumParamsPassed", funcNameOrBlank);
		return rf ? rf.numPassedParams : -1;
	};
	RunningFunc_ScopedVar_GetI(scopedVarName) {
		const val = this.Sub_CheckScopedVarAvail("RunningFunc_ScopedVar_GetI", scopedVarName, DarkScript_Expected.Either, true);
		return !val ? 0 : this.Sub_GetValAsInteger(val);
	};
	RunningFunc_ScopedVar_GetF(scopedVarName) {
		const val = this.Sub_CheckScopedVarAvail("RunningFunc_ScopedVar_GetF", scopedVarName, DarkScript_Expected.Either, true);
		return !val ? 0.0 : this.Sub_GetValAsFloat(val);
	};
	RunningFunc_ScopedVar_GetS(scopedVarName) {
		const val = this.Sub_CheckScopedVarAvail("RunningFunc_ScopedVar_GetS", scopedVarName, DarkScript_Expected.Either, true);
		return !val ? "" : this.Sub_GetValAsString(val);
	};
	RunningFunc_GetParamValueByIndexI(paramIndex) {
		const val = this.Sub_CheckParamAvail("RunningFunc_GetParamValueByIndexI", paramIndex);
		return !val ? 0 : this.Sub_GetValAsInteger(val);
	};
	RunningFunc_GetParamValueByIndexF(paramIndex) {
		const val = this.Sub_CheckParamAvail("RunningFunc_GetParamValueByIndexF", paramIndex);
		return !val ? 0 : this.Sub_GetValAsFloat(val);
	};
	RunningFunc_GetParamValueByIndexS(paramIndex) {
		const val = this.Sub_CheckParamAvail("RunningFunc_GetParamValueByIndexS", paramIndex);
		return !val ? 0 : this.Sub_GetValAsString(val);
	};
	RunningFunc_GetAllParamsAsText(funcNameOrBlank, separator, annotate) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_GetAllParamsAsText", funcNameOrBlank);
		if (!rf) {
			return "";
		}

		let i = 0;
		let output = "";
		for (const p of rf.paramValues)
		{
			if (!annotate) {
				output += p.data + separator;
			}
			else {
				output += rf.funcTemplate.params[i].name + " = ";
				if (p.type == DarkScript_Type.Integer) {
					output += p.data.integer + separator;
				}
				else if (p.type == DarkScript_Type.Float) {
					output += p.data.decimal + 'f' + separator;
				}
				else {
					let escapeMe = p.data;
					escapeMe = this.Sub_ReplaceAllString(escapeMe, "\r", "");
					escapeMe = this.Sub_ReplaceAllString(escapeMe, "\n", "\\n");
					escapeMe = this.Sub_ReplaceAllString(escapeMe, "\t", "\\t");
					escapeMe = this.Sub_ReplaceAllString(escapeMe, "\"", "\\\"");
					escapeMe = this.Sub_ReplaceAllString(escapeMe, "\\", "\\\\");

					output += `"${escapeMe}"${separator}`;
				}
			}
		}
		let outputStr = output;
		if (outputStr.length != 0) {
			outputStr = outputStr.slice(0, outputStr.length - separator.length);
		}

		return outputStr;
	};
	RunningFunc_GetCallStack(mostRecentAtBottom, rewindCount) {
		if (this.globals.runningFuncs.length == 0) {
			return this.CreateError("Couldn't get call stack; no functions running."), "";
		}
		if (rewindCount == 0 || rewindCount < -1) {
			return this.CreateError("Couldn't get call stack; rewind count %i is invalid.", rewindCount), "";
		}
		if ((mostRecentAtBottom & 1) != mostRecentAtBottom) {
			return this.CreateError("Couldn't get call stack; most recent at bottom param %i is invalid, must be 0 or 1.", mostRecentAtBottom), "";
		}
	
		if (rewindCount == -1) {
			rewindCount = this.globals.runningFuncs.length;
		}
	
		const nl = "\n";
		let str = "";
		let j = 0;
		if (mostRecentAtBottom == 0) {
			for (let i = this.globals.runningFuncs.length - 1; i != -1 && j < rewindCount; --i, ++j) {
				const k = this.globals.runningFuncs[i];
				str += `${k.funcTemplate.name} called from ${k.runLocation}${nl}`;
			}
		}
		else {
			for (let i = 0; i != -1 && j < rewindCount; ++i, ++j) {
				const k = this.globals.runningFuncs[i];
				str += `${k.funcTemplate.name} called from ${k.runLocation}${nl}`;
			}
		}
		str = str.slice(0, str.length - nl.length);
		return str;
	};
	RunningFunc_GetCalledFuncName(funcNameOrBlank) {
		const rf = this.Sub_GetRunningFunc("RunningFunc_GetCalledFuncName", funcNameOrBlank);
		if (!rf) {
			return "";
		}
		if (rf.redirectedFromFunctionName.length != 0) {
			return rf.redirectedFromFunctionName;
		}
		return rf.funcTemplate.name;
	};
	InternalLoop_GetIndex() {
		if (this.curLoopName.length == 0) {
			return this.CreateError("Couldn't read internal loop index: no internal loop running."), -1;
		}
		return this.internalLoopIndex;
	};
	InternalLoop_GetVarName() {
		if (this.curLoopName.length == 0) {
			return this.CreateError("Couldn't read internal loop param/scoped var name: no internal loop running."), "";
		}
		if (this.curParamLoop == null && this.curScopedVarLoop == null) {
			return this.CreateError("Couldn't read internal loop param/scoped var name: internal loop \"%s\" is not a param/scoped var loop.", this.curLoopName), "";
		}
		return this.curParamLoop ? this.curParamLoop.name : this.curScopedVarLoop.name;
	};
	InternalLoop_GetVarType() {
		if (this.curLoopName.length == 0) {
			return this.CreateError("Couldn't read internal loop param/scoped var type: no internal loop running."), "";
		}
		if (this.curParamLoop == null && this.curScopedVarLoop == null) {
			return this.CreateError("Couldn't read internal loop param/scoped var type: internal loop \"%s\" is not a param/scoped var loop.", this.curLoopName), "";
		}
		return this.TypeToString(this.curParamLoop ? this.curParamLoop.defaultVal.type : this.curScopedVarLoop.defaultVal.type);
	};
	FuncTemplate_GetFunctionName() {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_GetFunctionName", "");
		return ft ? ft.name : "";
	};
	FuncTemplate_GetNumRequiredParams(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_GetNumRequiredParams", funcNameOrBlank);
		if (!ft) {
			return -1;
		}
		// Type of Any is unset default value, so is required param
		return ft.params.filter(p => p.defaultVal.type == DarkScript_Type.Any).length;
	};
	FuncTemplate_GetNumPossibleParams(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_GetNumPossibleParams", funcNameOrBlank);
		if (!ft) {
			return -1;
		}
		return ft.params.length;
	};
	FuncTemplate_GetReturnType(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_GetReturnType", funcNameOrBlank);
		return ft ? this.TypeToString(ft.defaultReturnValue.type) : "";
	};
	FuncTemplate_ShouldRepeat(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ShouldRepeat", funcNameOrBlank);
		return ft ? ft.repeating : -1;
	};
	FuncTemplate_ShouldBeDelayed(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ShouldBeDelayed", funcNameOrBlank);
		return ft ? ft.delaying : -1;
	};
	FuncTemplate_RecursionAllowed(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_RecursionAllowed", funcNameOrBlank);
		return ft ? (ft.recursiveAllowed ? 1 : 0) : -1;
	};
	FuncTemplate_IsEnabled(funcNameOrBlank) {
		// can be false for running func if disabled while running
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_IsEnabled", funcNameOrBlank);
		return ft ? (ft.isEnabled ? 1 : 0) : -1;
	};
	FuncTemplate_GetRedirectFuncName(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_GetRedirectFuncName", funcNameOrBlank);
		return ft ? ft.redirectFunc : "";
	};
	FuncTemplate_GlobalID(funcNameOrBlank) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_GlobalID", funcNameOrBlank);
		return ft ? ft.globalID : "";
	};
	FuncTemplate_ParamNameByIndex(funcNameOrBlank, paramIndex) {
		/** @type DarkScript_FunctionTemplate | null */
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ParamNameByIndex", funcNameOrBlank);
		const p = this.Sub_GetTemplateParam("FuncTemplate_ParamNameByIndex", ft, paramIndex);
		return p ? p.name : "";
	};
	FuncTemplate_ParamTypeByIndex(funcNameOrBlank, paramIndex) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ParamTypeByIndex", funcNameOrBlank);
		const p = this.Sub_GetTemplateParam("FuncTemplate_ParamTypeByIndex", ft, paramIndex);
		return p ? this.TypeToString(p.type) : "";
	};
	FuncTemplate_ParamDefaultValByIndex(funcNameOrBlank, paramIndex) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ParamDefaultValByIndex", funcNameOrBlank);
		const p = this.Sub_GetTemplateParam("FuncTemplate_ParamDefaultValByIndex", ft, paramIndex);
		return p ? this.Sub_GetValAsString(p.defaultVal) : "";
	};
	FuncTemplate_ParamIndexByName(funcNameOrBlank, paramName) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ParamIndexByName", funcNameOrBlank);
		const p = this.Sub_GetTemplateParam("FuncTemplate_ParamIndexByName", ft, paramName);
		if (p == null) {
			return -1;
		}
		const idx = ft.params.findIndex(q => q == p);
		if (idx == -1) {
			throw Error("Param not found after it was found?");
		}
		return idx;
	};
	FuncTemplate_ParamTypeByName(funcNameOrBlank, paramName) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ParamTypeByName", funcNameOrBlank);
		const p = this.Sub_GetTemplateParam("FuncTemplate_ParamTypeByName", ft, paramName);
		return p ? this.TypeToString(p.defaultVal.type) : "";
	};
	FuncTemplate_ParamDefaultValByName(funcNameOrBlank, paramName) {
		const ft = this.Sub_GetFuncTemplateByName("FuncTemplate_ParamDefaultValByName", funcNameOrBlank);
		const p = this.Sub_GetTemplateParam("FuncTemplate_ParamDefaultValByName", ft, paramName);
		return p ? this.Sub_GetValAsString(p.defaultVal) : "";
	};
	LastReturn_AsInt() {
		return this.Sub_GetValAsInteger(this.lastReturn);
	};
	LastReturn_AsFloat() {
		return this.Sub_GetValAsFloat(this.lastReturn);
	};
	LastReturn_AsString() {
		return this.Sub_GetValAsString(this.lastReturn);
	};
	LastReturn_Type() {
		if (this.lastReturn.dataSize == 0) {
			return "void";
		}
		return this.TypeToString(this.lastReturn.type);
	};
	TestFunc(_param) {
		throw Error("Unimplemented");
	};

	// No comma for the last function : the Google compiler
	// we use for final projects does not accept it
};
