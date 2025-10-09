/*
	This Lacewing Blue Client Fusion extension UWP port is copyright © 2025 by Darkwire Software.
	Any redistribution is expressly prohibited outside of the official channels.
	It is expressly forbidden to acquire these files from GitHub for free for personal or commercial use;
	in other words, if there is a fee to acquire or use this port, any developers must pay for it and only
	use it for their own projects.
	It is in good faith, and to help new Fusion developers, that this file is uploaded to GitHub as is.

	This file is otherwise available under the MIT license.

	Modification for purposes of tuning to your own UWP application is permitted, but must retain this notice and not be redistributed,
	outside of its (hopefully minified) presence inside your UWP app's source code.
*/

/// <reference path="https://cdn.jsdelivr.net/gh/inexorabletash/text-encoding@master/lib/encoding.min.js" />
/// <reference path="https://cdn.jsdelivr.net/gh/imaya/zlib.js@master/bin/zlib.min.js" />
/* global console, darkEdif, Zlib, window, alert, GraphemeSplitter, CRunExtension, FinalizationRegistry, CServices, document, navigator,
	XMLHttpRequest, URL, Blob, TextEncoder, TextDecoder, WebSocket, setTimeout, clearTimeout */
/* jslint esversion: 6, sub: true */

// This is strict, but that can be assumed
//"use strict";

// Main differences between Windows and UWP:
// 1) UDP is not used, but is faked by part of the variant of the messages.
//    UWP JS does not use UDP; it could use it if it was UWP C# or UWP C++, but it's UWP JS.
// 2) No liblacewing event pump, checking the sockets and handling messages.
//    Sockets are maintained by the UWP app anyway. However, there is some dummy code to more closely mirror Windows,
//    on the minor chance that there is some issue with the Windows code and the fix needs replicating here.
// 3) Not multithreaded, for now at least. While it's possible to use WebWorkers with WebSockets, this adds an extra
//    layer of overhead, and event copying which, frankly, we don't need since there's no pump and thus any desync
//    is only from us queuing instead of immediately responding.
// 4) Web standards demand all disconnects and connects have no helpful identifying information. This is to prevent
//    malicious scripts from reading LAN connections' errors codes and figuring out local network addresses,
//    e.g. by trying to WebSocket into all LAN IPs and based on error return and speed, deduce whether a device is
//    available there.
//    However, browsers often give more information in the developer console.
// 5) Due to lack of DNS in browsers, and thus no IP returns, the Host IP expression is now a 1:1 of what the user
//    put in, leaving names unresolved.
//    It's possible the server can work out its remote IP and tell the client, but that's basically cosmetic,
//    and since WebSockets starts over HTTP which has a Host header, allowing one server IP to host multiple servers,
//    it may just confuse the issue as you could report the IP, connect directly to it again with ws://ip/, then due
//    to a different Host header or redirect, be sent to a different WebSocket server.
// 6) Blue client/channel name simplification does not match Blue Windows. Some effort has been made to make it
//    similar, but without a full UTF8PROC library, it's not possible to implement all the simplification
//    without largely reinventing the wheel, and that will add a lot of bulk to the ext.
// 7) UWP JS and Windows do not share protocols. WebSocket and raw socket are too different.
//    UWP C# and UWP C++ can use raw sockets, but UWP JS, that Fusion's UWP exporter uses, is browser-like.
//    You can connect from a HTTP page to WS:// connection, or WSS:// connection.
//    You CANNOT connect from a HTTPS page to a WS:// connection, just a WSS:// connection. Browsers will block it.
//    Get a free TLS/SSL certificate! Insecure connection is not recommended anyway - proxy servers may kill insecure connections.
//
//    Normally, Blue will match the type of page and guess the port - if HTTPS page, connection will be WSS, port 443.
//    However, it is possible to override this behaviour by connecting explicitly to "ws://path.com:port/".

/**
 * String.prototype.replaceAll() polyfill
 * https://gomakethings.com/how-to-replace-a-section-of-a-string-with-another-one-with-vanilla-js/
 * @author Chris Ferdinandi
 * @license MIT
 */
if (!String.prototype.replaceAll) {
	String.prototype.replaceAll = function(str, newStr){
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

// External scripts to load - mostly text encoding
var blueCliToScriptLoad = [];

/** @constructor */
function CRunBluewing_Client() {
	/// <summary> Constructor of Fusion object. </summary>
	CRunExtension.call(this);

	// DarkEdif SDK exts should have these four variables defined.
	// We need this[] and window[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
	this['ExtensionVersion'] = 105; // To match C++ version
	this['SDKVersion'] = 19;
	this['DebugMode'] = false;
	this['ExtensionName'] = 'Bluewing Client';

	// Can't find DarkEdif wrapper
	if (!window.hasOwnProperty('darkEdif')) {
		throw "a wobbly";
	}
	window['darkEdif'].checkSupportsSDKVersion(this.SDKVersion);

	// If text encoding or Zlib is missing, load from external files into the page's head tag.
	// Note their loading time is not instant.
	// It is recommended you insert them into the head tag yourself, from your local website.
	// Note: In UWP, these were added to default.js to load automatically.
	let scriptEmbedOff = blueCliToScriptLoad.length > 0;
	if (typeof window['TextEncoder'] === 'undefined' || typeof window['Zlib'] === 'undefined' || window['Zlib']['RawDeflate'] === 'undefined') {
		blueCliToScriptLoad.push(this);
		// First ext inserts script tags, all other instances don't, they just add to blueCliToScriptLoad
		if (blueCliToScriptLoad.length == 1) {
			// Load new JS if not present - TextEncoder should be in most browsers, though
			if (typeof window['TextEncoder'] === 'undefined') {
				let script2 = document.createElement('script');
				script2.type = 'text/javascript';
				script2.src = 'js/runtime/libs/encoding.min.js';
				// This is blocked by Microsoft's security settings, understandably.
				//script2.src = 'https://cdn.jsdelivr.net/gh/inexorabletash/text-encoding@master/lib/encoding.min.js';
				script2.text = '/* Loaded TextEncoding library automatically from a secure ' +
					'cloud connection by request of Bluewing. Consider downloading the file and including it in your app manually.*/';
				document.head.appendChild(script2);
			}

			// Likewise, load ZLIB, which probably won't be in the browsers
			if (typeof window['Zlib'] === 'undefined' || typeof window['Zlib']['RawDeflate'] === 'undefined') {
				let script3 = document.createElement('script');
				script3.type = 'text/javascript';
				script3.src = 'js/runtime/libs/zlib.min.js';
				//script3.src = 'ms-appx-web://cdn.jsdelivr.net/gh/imaya/zlib.js@master/bin/zlib.min.js';
				script3.text = '/* Loaded Zlib compression library automatically from a secure ' +
					'cloud connection by request of Bluewing. Consider downloading the file and including it in your app manually. */';
				document.head.appendChild(script3);
			}
		}
	}


	/// <field name="CLEAR_EVTNUM" type="Number"> An Event ID that actually indicates
	///		objects that should be marked as closed. Specifying peer or channel will close it. </field>
	this.CLEAR_EVTNUM = -1;

	// Grapheme splitters (for UTF8_VisibleCharCount)
	this.intlSegmenter = null;
	this.graphemeSplitter = null;

	// We need window instead of Intl direct workarounds because HTML5 Final Project minifies and breaks the names otherwise
	// this.intlSegmenter['example']() not this.intlSegmenter.example()

	// Intl.Segmenter - everywhere except Firefox
	if (window['Intl'] != null && window['Intl']['Segmenter'] != null) {
		this.intlSegmenter = new window['Intl']['Segmenter']();
	}
	// Alternative library: https://github.com/orling/grapheme-splitter/blob/master/index.js
	// We don't load this ourselves; we'll let the Fusion dev include it manually if they need it
	// Note: In UWP, this was added to default.js to load automatically.
	else if (window['GraphemeSplitter'] != null) {
		this.graphemeSplitter = new window['GraphemeSplitter']();
	}

	// we don't have these yet, because we don't have edPtr and its Fusion object property data, until CreateRunObject is called
	this.isGlobal = null;
	this.globals = null;
	this.selChannel = null;
	this.selPeer = null;
	this.threadData = new BluewingClient_EventToRun(null, {});

	// Called from outside of ext, so protect function names from minifier by using this['x'] instead of this.x
	this['GetSendMsg'] = function() {
		return this.globals.sendMsg;
	};
	this['SetSendMsg'] = function(newArr) {
		// should pass Uint8Array
		this.globals.sendMsg = newArr;
	};
	this['GetRecvMsg'] = function() {
		return this.threadData.recvMsg;
	};

	// Variables of Extension - not the GlobalData or Blue detail, that's under this.globals and this.globals.client
	this.Check_Subchannel = function (subChannel, func) {
		if (subChannel == null || subChannel > 255 || subChannel < 0) {
			this.CreateError("Error: " + func + " was called with an invalid subchannel; it must be " +
				"a number between 0 and 255, inclusively.");
			return false;
		}
		return true;
	};
	this.Check_NumMsgValue = function (numMsg, func) {
		if (numMsg == null) {
			this.CreateError("Error: " + func + " was called with a null parameter");
			return false;
		}
		if (isNaN(numMsg) || numMsg > 0x7FFFFFFF || numMsg < -0x7FFFFFFE) {
			this.CreateError("Error: " + func + " was called with an integer that couldn't be converted to an int32");
			return false;
		}
		return true;
	};
	this.Check_TextMsgValue = function (textMsg, func, isUDPMsg) {
		if (textMsg == null) {
			this.CreateError("Error: " + func + " was called with a null parameter");
			return false;
		}
		if (isUDPMsg) {
			let byteLength = this.textEncoder.encode(textMsg).byteLength;
			if (byteLength > this.globals.maxUDPSize) {
				this.CreateError("Error: " + func + " was called with text too large (" + byteLength + " bytes)");
				return false;
			}
		}

		return true;
	};
	this.Check_ChannelSelection = function (func) {
		if (this.selChannel == null) {
			this.CreateError("Error: " + func + " was called without a channel being selected");
			return false;
		}
		if (this.selChannel.isClosed) {
			this.CreateError("Error: " + func + " was called with a closed channel");
			return false;
		}
		return true;
	};
	this.Check_PeerSelection = function (func) {
		if (this.selPeer == null) {
			this.CreateError(func + " was called without a peer being selected");
			return false;
		}
		if (this.selPeer.isClosed) {
			this.CreateError(func + " was called with a closed peer");
			return false;
		}
		return true;
	};
	this.Check_UnlockedSendMsg = function (func) {
		if (this.globals.sendMsgLocked) {
			this.CreateError(func + " was called while the binary blob is being loaded in background.");
			return false;
		}
		return true;
	};
	this.Check_BinaryUDPSized = function (func) {
		if (this.globals.sendMsg.byteLength > this.globals.maxUDPSize) {
			this.CreateError(func + " was called with binary too large (" + this.globals.sendMsg.byteLength + " bytes).");
			return false;
		}
		return true;
	};
	this.CreateEvent = function (ids, dv) {
		this.globals.eventsToRun.push(new BluewingClient_EventToRun(ids, dv));
		if (this.globals.eventsToRun.length == 1) {
			this.ho.reHandle();
		}
	};
	this.CreateError = function (error) {
		/// <summary> Generates an error event with the given text. </summary>
		/// <param name="err" type="String" mayBeNull="false"> Error text. String only, cannot be null. </param>
		if (darkEdif.getCurrentFusionEventNumber(this) != -1) {
			error = "[Fusion event #" + darkEdif.getCurrentFusionEventNumber(this) + "] " + error;
		}
		else {
			error = "[handler] " + error;
		}

		this.CreateEvent([0], { errorStr: error });
	};

	// ============================================================================
	// Lacewing responders
	// ============================================================================
	this.LacewingCall_OnError = function (client, error) {
		/// <summary> Run when an error occurs. </summary>
		/// <param name="client" type="Bluewing_Client" mayBeNull="false">
		///		Client that this occurs in. Should always be the same for this.</param>
		/// <param name="error" type="String" mayBeNull="false">
		///		The error as text. </param>

		darkEdif.consoleLog(this, "Bluewing error: " + error);

		// Note use of Lacewing::Error was scrapped; error is just a String.
		this.CreateEvent([0], { errorStr: error });
	};
	this.LacewingCall_OnConnect = function (client) {
		this.CreateEvent([1], {});
	};
	this.LacewingCall_OnConnectDenied = function (client, denyReason) {
		this.serverAddr = client.serverAddr;
		this.serverPort = client.serverPort;
		this.CreateEvent([2, this.CLEAR_EVTNUM], { denyReason: denyReason });
	};
	this.LacewingCall_OnDisconnect = function (client) {
		this.CreateEvent([3, this.CLEAR_EVTNUM], {});
	};
	this.LacewingCall_OnChannelListReceived = function (client) {
		this.CreateEvent([26], {});
	};
	this.LacewingCall_OnJoinChannel = function (client, channel) {
		this.CreateEvent([4], { channel: channel });
	};
	this.LacewingCall_OnJoinChannelDenied = function (client, deniedChannelName, denyReason) {
		this.CreateEvent([5], { denyReason: denyReason, deniedChannelName: deniedChannelName });
	};
	this.LacewingCall_OnLeaveChannel = function (client, channel) {
		this.CreateEvent([43, this.CLEAR_EVTNUM], { channel: channel });
	};
	this.LacewingCall_OnLeaveChannelDenied = function (client, channel, denyReason) {
		this.CreateEvent([44], { channel: channel, denyReason: denyReason });
	};
	this.LacewingCall_OnNameSet = function (client) {
		this.CreateEvent([6], {});
	};
	this.LacewingCall_OnNameDenied = function (client, deniedName, denyReason) {
		this.CreateEvent([7], { denyReason: denyReason, deniedClientName: deniedName });
	};
	this.LacewingCall_OnNameChanged = function (client, oldName) {
		this.previousName = oldName;
		this.CreateEvent([53], {});
	};
	this.LacewingCall_OnPeerConnect = function (client, channel, peer) {
		this.CreateEvent([10], { channel: channel, peer: peer });
	};
	this.LacewingCall_OnPeerDisconnect = function (client, channel, peer) {
		this.CreateEvent([11, this.CLEAR_EVTNUM], { channel: channel, peer: peer });
	};
	this.LacewingCall_OnPeerNameChanged = function (client, channel, peer) {
		// Although peer name is changed on all channels at once, it's still
		// reported for each shared channel
		this.CreateEvent([45], { channel: channel, peer: peer });
	};
	this.LacewingCall_OnPeerMessage = function (client, fromPeer, viaChannel,
		blasted, subChannel, msg, size, variant) {
		/// <summary> On message from peer to peer (via channel). </summary>
		/// <param name="client" type="Bluewing_Client" mayBeNull="false">
		///		The Bluewing client variable. Ignored in this function. </param>
		/// <param name="fromPeer" type="Bluewing_Peer" mayBeNull="false"> The sending peer. </param>
		/// <param name="viaChannel" type="Bluewing_Channel" mayBeNull="false">
		///		The channel the peer sent the message on. </param>
		/// <param name="blasted" type="Boolean" mayBeNull="false">
		///		True if (pseudo) UDP, false if TCP. </param>
		/// <param name="subChannel" type="Number" mayBeNull="false"> The subchannel the message
		///		was sent on, range of 0-255 (Uint8). </param>
		/// <param name="msg" type="ArrayBuffer" mayBeNull="false"> The received message. </param>
		/// <param name="size" type="Number" mayBeNull="false"> Size of msg, kept for templating. </param>
		/// <param name="variant" type="Number" mayBeNull="false"> Variant of msg, 0-16, but
		///		only 0-2 are used in Lacewing Relay protocol. </param>
		const intArr = this.$GetEventNumsForMsg(blasted, variant, 52, 49, 39, 40, 41, 36, 37, 38);
		if (intArr == null) {
			return;
		}

		this.CreateEvent(intArr, { channel: viaChannel, peer: fromPeer, msg: msg, subChannel: subChannel });
	};
	this.LacewingCall_OnChannelMessage = function (client, fromPeer, onChannel,
		blasted, subChannel, msg, size, variant) {
		/// <summary> On message from peer to peer (via channel). </summary>
		/// <param name="client" type="Bluewing_Client" mayBeNull="false">
		///		The Bluewing client variable. Ignored in this function. </param>
		/// <param name="fromPeer" type="Bluewing_Peer" mayBeNull="false"> The sending peer. </param>
		/// <param name="onChannel" type="Bluewing_Channel" mayBeNull="false">
		///		The channel the peer sent the message on. </param>
		/// <param name="blasted" type="Boolean" mayBeNull="false">
		///		True if (pseudo) UDP, false if TCP. </param>
		/// <param name="subChannel" type="Number" mayBeNull="false"> The subchannel the message
		///		was sent on, range of 0-255 (Uint8). </param>
		/// <param name="msg" type="ArrayBuffer" mayBeNull="false"> The received message. </param>
		/// <param name="size" type="Number" mayBeNull="false"> Size of msg, kept for templating. </param>
		/// <param name="variant" type="Number" mayBeNull="false"> Variant of msg, 0-16, but
		///		only 0-2 are used in Lacewing Relay protocol. </param>
		const intArr = this.$GetEventNumsForMsg(blasted, variant, 51, 48, 22, 23, 35, 9, 16, 33);
		if (intArr == null) {
			return;
		}

		this.CreateEvent(intArr, { channel: onChannel, peer: fromPeer, msg: msg, subChannel: subChannel });
	};
	this.LacewingCall_OnServerMessage = function (client,
		blasted, subChannel, msg, size, variant) {
		/// <summary> On message from peer to peer (via channel). </summary>
		/// <param name="client" type="Bluewing_Client" mayBeNull="false">
		///		The Bluewing client variable. Ignored in this function. </param>
		/// <param name="blasted" type="Boolean" mayBeNull="false">
		///		True if (pseudo) UDP, false if TCP. </param>
		/// <param name="subChannel" type="Number" mayBeNull="false"> The subchannel the message
		///		was sent on, range of 0-255 (Uint8). </param>
		/// <param name="msg" type="ArrayBuffer" mayBeNull="false"> The received message. </param>
		/// <param name="size" type="Number" mayBeNull="false"> Size of msg, kept for templating. </param>
		/// <param name="variant" type="Number" mayBeNull="false"> Variant of msg, 0-16, but
		///		only 0-2 are used in Lacewing Relay protocol. </param>
		const intArr = this.$GetEventNumsForMsg(blasted, variant, 50, 47, 20, 21, 34, 8, 15, 32);
		if (intArr == null) {
			return;
		}

		this.CreateEvent(intArr, { msg: msg, subChannel: subChannel });
	};
	this.LacewingCall_OnServerChannelMessage = function (client, toChannel,
		blasted, subChannel, msg, size, variant) {
		/// <summary> On message from peer to peer (via channel). </summary>
		/// <param name="client" type="Bluewing_Client" mayBeNull="false">
		///		The Bluewing client variable. Ignored in this function. </param>
		/// <param name="toChannel" type="Bluewing_Channel" mayBeNull="false">
		///		The channel the peer sent the message on. </param>
		/// <param name="blasted" type="Boolean" mayBeNull="false">
		///		True if (pseudo) UDP, false if TCP. </param>
		/// <param name="subChannel" type="Number" mayBeNull="false"> The subchannel the message
		///		was sent on, range of 0-255 (Uint8). </param>
		/// <param name="msg" type="ArrayBuffer" mayBeNull="false"> The received message. </param>
		/// <param name="size" type="Number" mayBeNull="false"> Size of msg, kept for templating. </param>
		/// <param name="variant" type="Number" mayBeNull="false"> Variant of msg, 0-16, but
		///		only 0-2 are used in Lacewing Relay protocol. </param>
		const intArr = this.$GetEventNumsForMsg(blasted, variant, 72, 68, 69, 70, 71, 65, 66, 67);
		if (intArr == null) {
			return;
		}

		this.CreateEvent(intArr, { msg: msg, subChannel: subChannel });
	};

	// ======================================================================================================
	// Actions
	// ======================================================================================================
	this.Action_Replaced_Connect = function (host, port) {
		const die = "Bluewing error: Your Connect actions need to be recreated.\r\n" +
			"This is probably due to parameter changes.";
		this.CreateError(die);
	};
	this.Action_Disconnect = function () {
		/// <summary> Disconnects the Bluewing protocol. </summary>
		if (this.globals.client.isClosed) {
			this.CreateError("Can't disconnect, not connected to anything");
			return;
		}

		this.globals.client.Disconnect();
	};
	this.Action_SetName = function (newName) {
		/// <summary> Sets a new name. </summary>
		/// <param name="newName" type="String"> Text, the new name. Blank is disallowed. </param>
		if (newName == null || newName == "") {
			this.CreateError("SetName was called with an invalid name; cannot be blank, null, or undefined");
			return;
		}
		if (!this.globals.client.isConnected) {
			this.CreateError("SetName was called before a connection was created");
			return;
		}
		this.globals.client.SetName(newName);
	};
	this.Action_Replaced_JoinChannel = function (channelName, hideChannel) {
		const die = "Bluewing error: Your Join Channel actions need to be recreated.\r\n" +
			"This is probably due to parameter changes.";
		this.CreateError(die);
	};
	this.Action_LeaveChannel = function () {
		if (this.selChannel == null || this.selChannel.isClosed) {
			return;
		}
		this.selChannel.LeaveChannel();
	};
	this.Action_SendTextToServer = function (subChannel, textToSend) {
		/// <summary> Sends text to the server. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="textToSend" type="String"> Text to send. Can be blank. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Send Text to Server") ||
			!this.Check_TextMsgValue(textToSend, "Send Text to Server", false)) {
			return;
		}
		this.globals.client.SendMsg(subChannel, this.$StringToArrayBuffer(textToSend), 0);
	};
	this.Action_SendTextToChannel = function (subChannel, textToSend) {
		/// <summary> Sends text to the currently selected peer via the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="textToSend" type="String"> Text to send. Can be blank. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Send Text to Channel") ||
			!this.Check_TextMsgValue(textToSend, "Send Text to Channel", false) ||
			!this.Check_ChannelSelection("Send Text to Channel")) {
			return;
		}
		this.selChannel.SendMsg(subChannel, this.$StringToArrayBuffer(textToSend), 0);
	};
	this.Action_SendTextToPeer = function (subChannel, textToSend) {
		/// <summary> Sends text to the currently selected peer via the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="textToSend" type="String"> Text to send. Can be blank. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Send Text to Peer") ||
			!this.Check_TextMsgValue(textToSend, "Send Text to Peer", false) ||
			!this.Check_PeerSelection("Send Text to Peer")) {
			return;
		}
		this.selPeer.SendMsg(subChannel, this.$StringToArrayBuffer(textToSend), 0);
	};
	this.Action_SendNumberToServer = function (subChannel, numToSend) {
		/// <summary> Sends a number message to the server. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="numToSend" type="Number"> Number to send. Must fit in an Int32. </param>
		subChannel = ~~subChannel;
		numToSend = ~~numToSend;
		if (!this.Check_Subchannel(subChannel, "Send Number to Server") ||
			!this.Check_NumMsgValue(numToSend, "Send Number to Server")) {
			return;
		}
		this.globals.client.SendMsg(subChannel, this.$NumberToArrayBuffer(numToSend), 1);
	};
	this.Action_SendNumberToChannel = function (subChannel, numToSend) {
		/// <summary> Sends a number message to the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="numToSend" type="Number"> Number to send. Must fit in an Int32. </param>
		subChannel = ~~subChannel;
		numToSend = ~~numToSend;
		if (!this.Check_Subchannel(subChannel, "Send Number to Channel") ||
			!this.Check_NumMsgValue(numToSend, "Send Number to Channel") ||
			!this.Check_ChannelSelection("Send Number to Channel")) {
			return;
		}
		this.selChannel.SendMsg(subChannel, this.$NumberToArrayBuffer(numToSend), 1);
	};
	this.Action_SendNumberToPeer = function (subChannel, numToSend) {
		/// <summary> Sends a number message to the currently selected peer via the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="numToSend" type="Number"> Number to send. Must fit in an Int32. </param>
		subChannel = ~~subChannel;
		numToSend = ~~numToSend;
		if (!this.Check_Subchannel(subChannel, "Send Number to Peer") ||
			!this.Check_NumMsgValue(numToSend, "Send Number to Peer") ||
			!this.Check_PeerSelection("Send Number to Peer")) {
			return;
		}
		this.selPeer.SendMsg(subChannel, this.$NumberToArrayBuffer(numToSend), 1);
	};
	this.Action_BlastTextToServer = function (subChannel, textToBlast) {
		/// <summary> Blasts text to the server. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="textToBlast" type="String"> Text to blast. Can be blank. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Blast Text to Server") ||
			!this.Check_TextMsgValue(textToBlast, "Blast Text to Server", true)) {
			return;
		}
		this.globals.client.BlastMsg(subChannel, this.$StringToArrayBuffer(textToBlast), 0);
	};
	this.Action_BlastTextToChannel = function (subChannel, textToBlast) {
		/// <summary> Blasts text to the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="textToBlast" type="String"> Text to blast. Can be blank. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Blast Text to Channel") ||
			!this.Check_TextMsgValue(textToBlast, "Blast Text to Channel", true) ||
			!this.Check_ChannelSelection("Blast Text to Channel")) {
			return;
		}
		this.selChannel.BlastMsg(subChannel, this.$StringToArrayBuffer(textToBlast), 0);
	};
	this.Action_BlastTextToPeer = function (subChannel, textToBlast) {
		/// <summary> Blasts text to the currently selected peer. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="textToBlast" type="String"> Text to blast. Can be blank. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Blast Text to Peer") ||
			!this.Check_TextMsgValue(textToBlast, "Blast Text to Peer", true) ||
			!this.Check_PeerSelection("Blast Text to Peer")) {
			return;
		}
		this.selPeer.BlastMsg(subChannel, this.$StringToArrayBuffer(textToBlast), 0);
	};
	this.Action_BlastNumberToServer = function (subChannel, numToBlast) {
		/// <summary> Blasts a number message to the server. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="numToBlast" type="Number"> Number to blast. Must fit in an Int32. </param>
		subChannel = ~~subChannel;
		numToBlast = ~~numToBlast;
		if (!this.Check_Subchannel(subChannel, "Blast Number to Server") ||
			!this.Check_NumMsgValue(numToBlast, "Blast Number to Server")) {
			return;
		}
		this.globals.client.BlastMsg(subChannel, this.$NumberToArrayBuffer(numToBlast), 1);
	};
	this.Action_BlastNumberToChannel = function (subChannel, numToBlast) {
		/// <summary> Blasts a number message to the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="numToBlast" type="Number"> Number to blast. Must fit in an Int32. </param>
		subChannel = ~~subChannel;
		numToBlast = ~~numToBlast;
		if (!this.Check_Subchannel(subChannel, "Blast Number to Channel") ||
			!this.Check_NumMsgValue(numToBlast, "Blast Number to Channel") ||
			!this.Check_ChannelSelection("Blast Number to Channel")) {
			return;
		}
		this.selChannel.BlastMsg(subChannel, this.$NumberToArrayBuffer(numToBlast), 1);
	};
	this.Action_BlastNumberToPeer = function (subChannel, numToBlast) {
		/// <summary> Blasts a number message to the currently selected peer via the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		/// <param name="numToBlast" type="Number"> Number to blast. Must fit in an Int32. </param>
		subChannel = ~~subChannel;
		numToBlast = ~~numToBlast;
		if (!this.Check_Subchannel(subChannel, "Blast Number to Peer") ||
			!this.Check_NumMsgValue(numToBlast, "Blast Number to Peer") ||
			!this.Check_PeerSelection("Blast Number to Peer")) {
			return;
		}
		this.selPeer.BlastMsg(subChannel, this.$NumberToArrayBuffer(numToBlast), 1);
	};
	this.Action_SelectChannelByName = function (channelName) {
		if (channelName == null) {
			this.CreateError("A null channel name was passed to Select Channel With Name");
			return;
		}
		const channelnameSimplified = this.globals.client.SimplifyName(channelName);
		const channel = this.globals.client.channelList.find(function(c) { return c.nameSimplified == channelnameSimplified; });
		if (channel == null || channel.isClosed) {
			this.CreateError("Could not select channel using name \"" + channelName + "\", not joined to it");
			return;
		}
		this.selChannel = channel;
	};
	this.Action_ReplacedNoParams = function () {
		this.CreateError("An unknown/deprecated action ID was called. Please check all your Bluewing events for " +
			"undefined actions.");
	};
	this.Action_LoopClientChannels = function () {
		const origSelChannel = this.selChannel;
		const origSelPeer = this.selPeer;
		const origLoopName = this.loopName;

		for (let ch of this.globals.client.channelList) {
			this.selChannel = ch;
			this.selPeer = null;
			this.loopName = "";
			this.ho.generateEvent(14);
		}
		this.selChannel = origSelChannel;
		this.selPeer = origSelPeer;
		this.loopName = "";
		this.ho.generateEvent(18);

		this.loopName = origLoopName;
	};
	this.Action_SelectPeerOnChannelByName = function (peerName) {
		if (peerName == null || peerName == "") {
			this.CreateError("Error: Select Peer On Channel By Name was called with a null parameter/blank name");
		}
		else if (this.selChannel == null) {
			this.CreateError("Error: Select Peer On Channel By Name was called without a channel being selected");
		}
		else {
			const peerNameSimplified = this.globals.client.SimplifyName(peerName);
			const peer = this.selChannel.peerList.find(function(p) { return p.nameSimplified == peerNameSimplified; });
			if (peer == null || peer.isClosed) {
				this.CreateError("Peer with name \"" + peerName + "\" was not found on channel \"" +
					this.selChannel.name + "\".");
				return;
			}

			this.selPeer = peer;
		}
	};
	this.Action_SelectPeerOnChannelByID = function (peerID) {
		peerID = ~~peerID;
		if (peerID < 0 || peerID > 65535 || peerID == null) {
			this.CreateError("Could not select peer on channel, ID is not within range 0-65535");
		}
		else if (this.selChannel == null) {
			this.CreateError("Error: Select Peer On Channel By ID was called without a channel being selected");
		}
		else if (this.selChannel.isClosed) {
			this.CreateError("Error: Select Peer On Channel By ID was called with a closed channel");
		}
		else {
			const peer = this.selChannel.getPeer(peerID);
			if (peer == null || peer.isClosed) {
				this.CreateError("Peer with ID " + peerID + " was not found on channel \"" +
					this.selChannel.name + "\"");
				return;
			}

			this.selPeer = peer;
		}
	};
	this.Action_LoopPeersOnChannel = function () {
		// Store selected channel
		if (this.selChannel == null) {
			this.CreateError("Error: Loop Peers On Channel was called without a channel being selected");
			return;
		}
		const origSelChannel = this.selChannel;
		const origSelPeer = this.selPeer;
		const origLoopName = this.loopName;
		for (let peer of origSelChannel.peerList) {
			this.selChannel = origSelChannel;
			this.selPeer = peer;
			this.loopName = "";
			this.ho.generateEvent(13);
		}

		this.selChannel = origSelChannel;
		this.selPeer = origSelPeer;
		this.loopName = "";
		this.ho.generateEvent(17);

		this.loopName = origLoopName;
	};
	this.Action_RequestChannelList = function () {
		this.globals.client.ListChannels();
	};
	this.Action_LoopListedChannels = function () {
		const origChannelListing = this.threadData.channelListing;
		const origLoopName = this.loopName;

		for (let chLst of this.globals.client.channelListing) {
			this.threadData.channelListing = chLst;
			this.loopName = "";
			this.ho.generateEvent(27);
		}
		this.threadData.channelListing = origChannelListing;
		this.loopName = "";
		this.ho.generateEvent(28);

		this.loopName = origLoopName;
	};
	this.Action_SendBinaryToServer = function (subChannel) {
		/// <summary> Sends the binary to the server. </summary>
		/// <param name="subChannel"> An 8-bit unsigned number, 0-255. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Send Binary to Server") ||
			!this.Check_UnlockedSendMsg("Send Binary to Server")) {
			return;
		}
		this.globals.client.SendMsg(subChannel, this.globals.sendMsg, 2);
		this.$ClearBinaryIfSetTo();
	};
	this.Action_SendBinaryToChannel = function (subChannel) {
		/// <summary> Sends the binary to the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Send Binary to Channel") ||
			!this.Check_ChannelSelection("Send Binary to Channel") ||
			!this.Check_UnlockedSendMsg("Send Binary to Channel")) {
			return;
		}
		this.selChannel.SendMsg(subChannel, this.globals.sendMsg, 2);
		this.$ClearBinaryIfSetTo();
	};
	this.Action_SendBinaryToPeer = function (subChannel) {
		/// <summary> Sends the binary to the currently selected peer via the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Send Binary to Peer") ||
			!this.Check_PeerSelection("Send Binary to Peer") ||
			!this.Check_UnlockedSendMsg("Send Binary to Peer")) {
			return;
		}
		this.selPeer.SendMsg(subChannel, this.globals.sendMsg, 2);
		this.$ClearBinaryIfSetTo();
	};
	this.Action_BlastBinaryToServer = function (subChannel) {
		/// <summary> Blasts the binary to the server. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Blast Binary to Server") ||
			!this.Check_UnlockedSendMsg("Blast Binary to Server") ||
			!this.Check_BinaryUDPSized("Blast Binary to Server")) {
			return;
		}
		this.globals.client.BlastMsg(subChannel, this.globals.sendMsg, 2);
		this.$ClearBinaryIfSetTo();
	};
	this.Action_BlastBinaryToChannel = function (subChannel) {
		/// <summary> Blasts binary to the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Blast Binary to Channel") ||
			!this.Check_ChannelSelection("Blast Binary to Channel") ||
			!this.Check_UnlockedSendMsg("Blast Binary to Channel") ||
			!this.Check_BinaryUDPSized("Blast Binary to Channel")) {
			return;
		}
		this.selChannel.BlastMsg(subChannel, this.globals.sendMsg, 2);
		this.$ClearBinaryIfSetTo();
	};
	this.Action_BlastBinaryToPeer = function (subChannel) {
		/// <summary> Blasts binary to the currently selected peer via the currently selected channel. </summary>
		/// <param name="subChannel" type="Number"> An 8-bit unsigned number, 0-255. </param>
		subChannel = ~~subChannel;
		if (!this.Check_Subchannel(subChannel, "Blast Binary to Peer") ||
			!this.Check_PeerSelection("Blast Binary to Peer") ||
			!this.Check_UnlockedSendMsg("Blast Binary to Peer") ||
			!this.Check_BinaryUDPSized("Blast Binary to Peer")) {
			return;
		}
		this.selPeer.BlastMsg(subChannel, this.globals.sendMsg, 2);
		this.$ClearBinaryIfSetTo();
	};
	this.Action_AddASCIITextByte = function (theByte) {
		if (theByte == null || theByte.constructor.name != "String") {
			this.CreateError("Adding byte to binary failed: string supplied was null or not a string");
			return;
		}
		if (theByte == "") {
			this.CreateError("Adding byte to binary failed: string supplied was blank");
			return;
		}

		const next = theByte.charCodeAt(1);
		if (!isNaN(next)) {
			this.CreateError("Adding byte to binary failed: byte supplied was part of a string, not a single byte");
			return;
		}
		let b = theByte.charCodeAt(0);
		b = isNaN(b) || theByte.length == 0 ? -1 : b;
		if (b > 128 || b < 0) {
			this.CreateError("Adding byte to binary failed: the string was a non-ASCII character");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}
		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + 1);
		newMsg.set(this.globals.sendMsg, 0);
		newMsg.set([b], this.globals.sendMsg.byteLength);
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddByteInt = function (theByte) {
		const b = parseInt(theByte, 10);
		if (b == null || isNaN(b)) {
			this.CreateError("Adding integer to binary failed: integer supplied was null or not a integer");
			return;
		}
		if (b > 255 || b < -127) {
			this.CreateError("Error: integer out of bounds");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}

		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + 1);
		newMsg.set(this.globals.sendMsg, 0);
		const view = new DataView(newMsg.buffer);
		if (b < 0) {
			view.setInt8(this.globals.sendMsg.byteLength, b);
		}
		else {
			view.setUint8(this.globals.sendMsg.byteLength, b);
		}
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddShort = function (theShort) {
		const s = parseInt(theShort, 10);
		if (s == null || isNaN(s)) {
			this.CreateError("Adding short to binary failed: short supplied was null or not a short");
			return;
		}
		if (s > 65535 || s < -32768) {
			this.CreateError("Error: Short out of bounds");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}

		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + 2);
		newMsg.set(this.globals.sendMsg, 0);
		const view = new DataView(newMsg.buffer);
		if (s < 0) {
			view.setInt16(this.globals.sendMsg.byteLength, s, true);
		}
		else {
			view.setUint16(this.globals.sendMsg.byteLength, s, true);
		}
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddInt = function (theInt) {
		const i = parseInt(theInt, 10);
		if (i == null || isNaN(i)) {
			this.CreateError("Adding integer to binary failed: integer supplied was null or not a integer");
			return;
		}
		// Note parseInt is not Int32, it's a weirder JS one that has longer ranges.
		if (i > 4294967295 || i < -2147483648) {
			this.CreateError("Error: Integer out of bounds");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}

		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + 4);
		newMsg.set(this.globals.sendMsg, 0);
		const view = new DataView(newMsg.buffer);
		if (i < 0) {
			view.setInt32(this.globals.sendMsg.byteLength, i, true);
		}
		else {
			view.setUint32(this.globals.sendMsg.byteLength, i, true);
		}
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddFloat = function (theFloat) {
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}
		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + 4);
		newMsg.set(this.globals.sendMsg, 0);
		new DataView(newMsg.buffer).setFloat32(this.globals.sendMsg.byteLength, theFloat, true);
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddStringWithoutNull = function (string) {
		if (string == null || string.constructor.name != "String") {
			this.CreateError("Adding string without null failed: pointer was null");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}
		const stringAsArrBuff = this.$StringToArrayBuffer(string);

		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + stringAsArrBuff.byteLength);
		newMsg.set(this.globals.sendMsg, 0);
		newMsg.set(stringAsArrBuff, this.globals.sendMsg.byteLength);
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddString = function (string) {
		if (string == null || string.constructor.name != "String") {
			this.CreateError("Adding string without null failed: pointer was null");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}
		const stringAsArrBuff = this.$StringToArrayBuffer(string);

		const newMsg = new Uint8Array(this.globals.sendMsg.byteLength + stringAsArrBuff.byteLength + 1);
		newMsg.set(this.globals.sendMsg, 0);
		newMsg.set(stringAsArrBuff, this.globals.sendMsg.byteLength);
		this.globals.sendMsg = newMsg;
	};
	this.Action_AddBinary = function (address, size) {
		if (address.constructor.name != "String") {
			this.CreateError("Add binary failed: Memory address (blob) type " + address.constructor.name + " unexpected");
			return;
		}
		if (this.globals.sendMsgLocked) {
			this.CreateError("Adding to binary message failed; message is locked due to loading a blob into it");
			return;
		}

		size = ~~size;
		if (size == 0) {
			return;
		}

		if (size < 0 || size > 0xFFFFFFFE) {
			this.CreateError("Add binary failed: Size < 0");
			return;
		}

		const self = this;

		const xhr = new XMLHttpRequest();
		xhr.open('GET', address, true);
		xhr.responseType = 'arraybuffer';
		xhr.onload = function(e) {
			self.globals.sendMsgLocked = false;
			if (this.status == 200) {
				const arrBuff = this.response;
				size = Math.min(arrBuff.byteLength, size);
				const newMsg = new Uint8Array(self.globals.sendMsg.byteLength + size + 1);
				newMsg.set(self.globals.sendMsg, 0);
				newMsg.set(new Uint8Array(arrBuff, 0, size), self.globals.sendMsg.byteLength);
				self.globals.sendMsg = newMsg;
			}
			else {
				self.CreateError("Couldn't add binary; loading from \"" + address + "\" failed with code " + this.status);
			}
		};
		xhr.send();
	};
	this.Action_ClearBinaryToSend = function () {
		if (this.globals.sendMsgLocked) {
			this.CreateError("Clearing to-send binary message failed; message is locked due to loading a blob into it");
			return;
		}
		this.globals.sendMsg = new Uint8Array(0);
	};
	// FileReader.readAsArrayBuffer
	// There's a FileReader/FileWriter API, but only Chrome and some Opera version
	// support it. W3C have discontinued the spec and said no one should use it... so yeah.
	this.Action_SaveReceivedBinaryToFile = function (position, size, fileName) {
		if (position < 0) {
			this.CreateError("Cannot save received binary; Position less than 0");
		}
		else if (size <= 0) {
			this.CreateError("Cannot save received binary; Size equal or less than 0");
		}
		else if (fileName.constructor.name != "String" || !fileName || fileName[0] == '\0') {
			this.CreateError("Cannot save received binary; filename is invalid");
		}
		else if (this.threadData.recvMsg.byteLength - size <= 0) {
			this.CreateError("Cannot save received binary; Message is too small");
		}
		else {
			this.CreateError("File loading/saving by path is not available in UWP. Please " +
						 "contact Blue developer if this is needed");
		}
	};
	this.Action_AppendReceivedBinaryToFile = function (position, size, fileName) {
		if (position < 0) {
			this.CreateError("Cannot append received binary; Position less than 0");
		}
		else if (size <= 0) {
			this.CreateError("Cannot append received binary; Size equal or less than 0");
		}
		else if (fileName.constructor.name != "String" || fileName == "") {
			this.CreateError("Cannot append received binary; filename is invalid");
		}
		else if (this.threadData.recvMsg.size - size <= 0) {
			this.CreateError("Cannot append received binary; Message is too small");
		}
		else {
			this.CreateError("File loading/saving by path is not available in UWP. Please " +
						 "contact Blue developer if this is needed");
		}
	};
	this.Action_AddFileToBinary = function (fileName) {
		if (fileName.constructor.name != "String" || fileName == "") {
			this.CreateError("Cannot add file to send binary; filename is invalid");
		}
		else {
			this.CreateError("File loading/saving by path is not available in UWP. Please " +
						 "contact Blue developer if this is needed");
		}
	};
	this.Action_SelectChannelMaster = function () {
		if (this.selChannel == null) {
			this.CreateError("Could not select channel master: No channel selected");
		}
		else {
			const stored = this.selPeer;
			if (this.selChannel.master) {
				this.selPeer = this.selChannel.master;
			}
			else { // Restore if no channel master found
				this.selPeer = stored;
			}
		}
	};
	this.Action_JoinChannel = function (channelName, hidden, closeAutomatically) {
		if (channelName == null || channelName.constructor.name != "String" || channelName == "") {
			this.CreateError("Cannot join channel: invalid channel name supplied");
		}
		else {
			this.globals.client.JoinChannel(channelName, hidden != 0, closeAutomatically != 0);
		}
	};
	this.Action_CompressSendBinary = function () {
		// plain = Array.<number> or Uint8Array
		const deflate = new Zlib['Deflate'](this.globals.sendMsg);
		const compressed = deflate['compress'](); // returns Uint8Array
		const count = this.globals.sendMsg.byteLength;
		this.globals.sendMsg = new Uint8Array(4 + compressed.byteLength);
		new DataView(this.globals.sendMsg.buffer, 0, 4).setUint32(0, count, true);
		this.globals.sendMsg.set(compressed, 4);
	};
	this.Action_DecompressReceivedBinary = function () {
		if (this.threadData.recvMsg.byteLength < 12) {
			this.CreateError("Cannot decompress message; too small.");
			return;
		}
		// compressed = Array.<number> or Uint8Array
		try {
			const inflate = new Zlib['Inflate'](new Uint8Array(this.threadData.recvMsg, 4));
			const plain = inflate['decompress'](); // returns Uint8Array (presumably)
			const origCursorIndex = this.threadData.recvMsg.cursorIndex;
			this.threadData.recvMsg = new ArrayBuffer(plain.byteLength);
			this.threadData.recvMsg.cursorIndex = origCursorIndex;
			new Uint8Array(this.threadData.recvMsg).set(plain);
		}
		catch (e) {
			this.CreateError("Couldn't decompression; decompression returned error " + e + ".");
		}
	};
	this.Action_MoveReceivedBinaryCursor = function (newPos) {
		if (~~newPos !== newPos) {
			this.CreateError("Can't move received binary cursor; unexpected type");
			return;
		}
		if (newPos < 0 || newPos > this.threadData.recvMsg.byteLength) {
			this.CreateError("Can't move received binary cursor; new cursor index " + newPos + " is smaller than 0 " +
							"or beyond the end of the recived message (in this case " + this.threadData.recvMsg.byteLength + ")");
			return;
		}
		this.threadData.recvMsg.cursorIndex = newPos;
	};
	this.Action_LoopListedChannelsWithLoopName = function (loopName) {
		if (loopName == null || loopName.constructor.name != 'String') {
			throw "Loop name must be non-null string.";
		}
		if (loopName == "") {
			this.CreateError("Cannot loop listed channels: invalid loop name \"\" supplied.");
			return;
		}

		const origChannelListing = this.threadData.channelListing;
		const origLoopName = this.loopName;

		for (let chLst of this.globals.client.channelListing) {
			this.threadData.channelListing = chLst;
			this.loopName = loopName;
			this.ho.generateEvent(59);
		}
		this.threadData.channelListing = origChannelListing;
		this.loopName = loopName;
		this.ho.generateEvent(60);

		this.loopName = origLoopName;
	};
	this.Action_LoopClientChannelsWithLoopName = function (loopName) {
		if (loopName == null || loopName.constructor.name != 'String') {
			throw "Loop name must be non-null string.";
		}
		if (loopName == "") {
			this.CreateError("Cannot loop joined channels: invalid loop name \"\" supplied.");
			return;
		}

		const origSelChannel = this.selChannel;
		const origSelPeer = this.selPeer;
		const origLoopName = this.loopName;
		for (let ch of this.globals.client.channelList) {
			this.selChannel = ch;
			this.selPeer = null;
			this.loopName = loopName;
			this.ho.generateEvent(63);
		}
		this.selChannel = origSelChannel;
		this.selPeer = origSelPeer;
		this.loopName = loopName;
		this.ho.generateEvent(64);

		this.loopName = origLoopName;
	};
	this.Action_LoopPeersOnChannelWithLoopName = function (loopName) {
		if (loopName == null || loopName.constructor.name != 'String') {
			throw "Loop name must be non-null string.";
		}
		if (loopName == "") {
			this.CreateError("Cannot loop peers on channel: invalid loop name \"\" supplied.");
			return;
		}
		if (this.selChannel == null) {
			this.CreateError("Cannot loop peers on channel: no channel currently selected.");
			return;
		}

		const origSelChannel = this.selChannel;
		const origSelPeer = this.selPeer;
		const origLoopName = this.loopName;
		for (let peer of origSelChannel.peerList) {
			this.selChannel = origSelChannel;
			this.selPeer = peer;
			this.loopName = loopName;
			this.ho.generateEvent(61);
		}
		this.selChannel = origSelChannel;
		this.selPeer = origSelPeer;
		this.loopName = loopName;
		this.ho.generateEvent(62);

		this.loopName = origLoopName;
	};
	this.Action_Connect = function (serverAddr) {
		if (serverAddr == null || serverAddr.constructor.name != 'String') {
			throw "Server address must be non-null string.";
		}

		if (serverAddr == "") {
			this.CreateError("Can't connect to a blank server");
		}
		//else if (typeof io == 'undefined') {
		//	this.connectAddressGiven = serverAddr;
		// }
		else {
			this.globals.client.Connect(serverAddr);

			// Copy out, as the event queue's delayed handling will mean these serverXX variables
			// will be empty by the time On Disconnect/On Connect Denied runs
			this.globals.hostIP = this.globals.client.serverAddr;
			this.globals.hostPort = this.globals.client.serverPort;
		}
	};
	this.Action_ResizeBinaryToSend = function (newSize) {
		if (~~newSize !== newSize) {
			this.CreateError("Can't move received binary cursor; unexpected type");
			return;
		}
		if (newSize < 0) {
			this.CreateError("Can't resize binary-to-send to a negative size");
			return;
		}
		const newArr = new Uint8Array(newSize),
			len = this.globals.sendMsg.byteLength,
			resizeLen = len > newSize ? newSize : len;
		newArr.set(new Uint8Array(this.globals.sendMsg.buffer, 0, resizeLen));

		this.globals.sendMsg = newArr;
	};
	this.Action_SetDestroySetting = function (enabled) {
		if (enabled > 1 || enabled < 0) {
			return this.CreateError("Invalid setting passed to SetDestroySetting, expecting 0 or 1.");
		}
		this.globals.fullDeleteEnabled = enabled != 0;
	};
	this.Action_SetLocalPortForHolePunch = function (port) {
		if (port < 1 || port > 0xFFFF) {
			return this.CreateError("Invalid port passed to SetLocalPortForHolePunch, expecting 1 through 65535, got " + port + ".");
		}
		this.CreateError("Setting local port is not available in UWP.");
	};

	// ======================================================================================================
	// Conditions
	// ======================================================================================================

	this.$LoopNameTest = function (curName, func) {
		if (curName == null || curName.constructor.name != 'String') {
			throw func + " condition's parameter must be non-null string.";
		}
		return this.loopName == curName;
	};
	this.Condition_OnSentTextMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Text Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentTextMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Text Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentNumberMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Number Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentNumberMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Number Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedTextMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Text Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedNumberMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Number Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedTextMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Text Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedNumberMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Number Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentBinaryMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Binary Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentBinaryMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Binary Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedBinaryMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Binary Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedBinaryMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Binary Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentTextMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Text Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentNumberMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Number Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentBinaryMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Binary Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedTextMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Text Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedNumberMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Number Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedBinaryMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Binary Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_IsConnected = function () {
		return this.globals.client.isConnectApproved;
	};
	this.Condition_OnAnySentMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Sent Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnySentMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Sent Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnySentMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Sent Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnyBlastedMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Blasted Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnyBlastedMessageFromChannel = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Blasted Message From Channel") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnyBlastedMessageFromPeer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Blasted Message From Peer") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_ClientHasAName = function () {
		return this.globals.client.me.name != "";
	};
	this.Condition_AlwaysFalse = function () {
		return false;
	};
	this.Condition_AlwaysTrue = function () {
		return true;
	};
	this.Condition_MandatoryTriggeredEvent = function () {
		this.lastMandatoryEventWasChecked = true;
		return true;
	};
	this.Condition_SelectedPeerIsChannelMaster = function () {
		if (!this.globals.client.isConnectApproved) {
			this.CreateError("Peer is Channel Master condition failed; client not connected");
			return;
		}
		if (this.selChannel == null) {
			this.CreateError("Peer is Channel Master condition failed; no channel selected");
			return;
		}
		if (this.selPeer == null) {
			this.CreateError("Peer is Channel Master condition failed; no peer selected");
			return;
		}

		return this.selChannel.master.id == this.selPeer.id;
	};
	this.Condition_YouAreChannelMaster = function () {
		if (!this.globals.client.isConnectApproved) {
			this.CreateError("You Are Channel Master condition failed; client not connected");
			return;
		}
		if (this.selChannel == null) {
			this.CreateError("You Are Channel Master condition failed; no channel selected");
			return;
		}

		return this.selChannel.master.id == this.globals.client.me.id;
	};
	this.Condition_OnChannelListLoopWithName = function (curName) {
		return this.$LoopNameTest(curName, "Channel listing loop with name");
	};
	this.Condition_OnChannelListLoopWithNameFinished = function (curName) {
		return this.$LoopNameTest(curName, "Channel listing loop with name finished");
	};
	this.Condition_OnPeerLoopWithName = function (curName) {
		return this.$LoopNameTest(curName, "Peer loop with name");
	};
	this.Condition_OnPeerLoopWithNameFinished = function (curName) {
		return this.$LoopNameTest(curName, "Peer loop with name finished");
	};
	this.Condition_OnClientChannelLoopWithName = function (curName) {
		return this.$LoopNameTest(curName, "Client joined-channel loop with name");
	};
	this.Condition_OnClientChannelLoopWithNameFinished = function (curName) {
		return this.$LoopNameTest(curName, "Client joined-channel loop with name finished");
	};
	this.Condition_OnSentTextChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Channel Text Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentNumberChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Channel Number Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnSentBinaryChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Sent Channel Binary Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnySentChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Sent Channel Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedTextChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Channel Text Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedNumberChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Channel Number Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnBlastedBinaryChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Blasted Channel Binary Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_OnAnyBlastedChannelMessageFromServer = function (subchannel) {
		return subchannel == -1 || (this.Check_Subchannel(subchannel, "On Any Blasted Channel Message From Server") &&
			this.threadData.recvSubChannel == subchannel);
	};
	this.Condition_IsJoinedToChannel = function (channelName) {
		return this.globals.client.isConnectApproved && this.globals.client.me.name != "" &&
			this.globals.client.channelList.findIndex(function (e) { return e.name == channelName; }) !== -1;
	};
	this.Condition_IsPeerOnChannel_Name = function (peerName, channelName) {
		if (!this.globals.client.isConnectApproved || this.globals.client.me.name == "") {
			return false;
		}
		const ch = channelName == "" ? this.selChannel : this.globals.client.channelList.find(function (e) { return e.name == channelName; });
		if (ch == null) {
			return false;
		}
		return ch.peerList.find(function (e) { return e.name == peerName; }) != null;
	};
	this.Condition_IsPeerOnChannel_ID = function (peerID, channelName) {
		if (!this.globals.client.isConnectApproved || this.globals.client.me.name == "" || (~~peerID !== peerID)) {
			return false;
		}
		const ch = channelName == "" ? this.selChannel : this.globals.client.channelList.find(function (e) { return e.name == channelName; });
		if (ch == null) {
			return false;
		}
		return ch.getPeer(peerID) != null;
	};

	// =============================
	// Expressions
	// =============================

	this.Expression_Error = function () {
		return this.threadData.error;
	};
	this.Expression_ReplacedExprNoParams = function () {
		return "";
	};
	this.Expression_Self_Name = function () {
		return this.globals.client.me.name;
	};
	this.Expression_Self_ChannelCount = function () {
		return this.globals.client.channelList.length;
	};
	this.Expression_Peer_Name = function () {
		return this.selPeer != null ? this.selPeer.name : "";
	};
	this.Expression_ReceivedStr = function () {
		return this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength == 0 ? "" : this.textDecoder.decode(this.threadData.recvMsg);
	};
	this.Expression_ReceivedInt = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength !== 4) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength !== 4) {
			this.CreateError("Received() was used on a message that is not a number message");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getInt32(0, true);
	};
	this.Expression_Subchannel = function () {
		return this.threadData.recvSubChannel;
	};
	this.Expression_Peer_ID = function () {
		return this.selPeer != null ? this.selPeer.id : -1;
	};
	this.Expression_Channel_Name = function () {
		if (this.selChannel != null) {
			return this.selChannel.name;
		}
		// If channel join was denied (condition 5), selChannel will be null, and the denied channel name will be in deniedChannelName
		if (this.threadData.idList != null && this.threadData.idList[0] == 5) {
			return this.threadData.deniedChannelName;
		}
		return "";
	};
	this.Expression_Channel_PeerCount = function () {
		return this.selChannel != null ? this.selChannel.peerList.length : -1;
	};
	this.Expression_ChannelListing_Name = function () {
		return this.threadData.channelListing != null ? this.threadData.channelListing.name : "";
	};
	this.Expression_ChannelListing_PeerCount = function () {
		return this.threadData.channelListing != null ? this.threadData.channelListing.peerCount : -1;
	};
	this.Expression_Self_ID = function () {
		return this.globals.client.me.id;
	};
	this.Expression_ASCIIStrByte = function (index) {
		if (this.threadData.recvMsg == null && this.threadData.recvMsg.byteLength - index < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return "";
		}
		else if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return "";
		}

		// Max bytecount of a utf-8 char is 4 bytes (internally clamped to end of recvMsg)
		const msg = this.threadData.recvMsg.slice(index, 4);
		const msg2 = this.textDecoder.decode(msg);
		return msg2.charAt(0);
	};
	this.Expression_UnsignedByte = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getUint8(index);
	};
	this.Expression_SignedByte = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getInt8(index);
	};
	this.Expression_UnsignedShort = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 2) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getUint16(index, true);
	};
	this.Expression_SignedShort = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 2) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getInt16(index, true);
	};
	this.Expression_UnsignedInteger = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 4) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		const readFrom = new DataView(this.threadData.recvMsg);
		return readFrom.getUint32(index, true);
	};
	this.Expression_SignedInteger = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 4) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getInt32(index, true);
	};
	this.Expression_Float = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0.0;
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return 0.0;
		}
		else if (this.threadData.recvMsg.byteLength - index < 4) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0.0;
		}

		return new DataView(this.threadData.recvMsg).getFloat32(index, true);
	};
	this.Expression_StringWithSize = function (index, stringSize) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return "";
		}
		else if (stringSize < 0) {
			this.CreateError("Could not read from received binary, string size to read less than 0");
			return "";
		}
		else if (this.threadData.recvMsg.byteLength - index < stringSize) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return "";
		}

		return this.$GetRecvMsgSubString(index, stringSize, false);
	};
	this.Expression_String = function (index) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		if (index < 0) {
			this.CreateError("Could not read from received binary, index less than 0");
			return "";
		}
		else if (this.threadData.recvMsg.byteLength - index < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return "";
		}

		return this.$GetRecvMsgSubString(index, -1, false);
	};

	this.Expression_ReceivedBinarySize = function () {
		return this.threadData.recvMsg == null ? -1 : this.threadData.recvMsg.byteLength;
	};
	this.Expression_Lacewing_Version = function () {
		const has64 = navigator.userAgent.indexOf('WOW64')>-1 || navigator.userAgent.indexOf('Win64')>-1 ||
			(navigator.platform && navigator.platform != 'Win32');
		return "liblacewing 0.5.4 (UWP " + (has64 ? "x64, 64" : "x86, 32") + "-bit) / Bluewing reimpl b" + this['ExtensionVersion'];
	};
	this.Expression_SendBinarySize = function () {
		return this.globals.sendMsg.byteLength;
	};
	this.Expression_Self_PreviousName = function () {
		return this.globals.client.me.previousName;
	};
	this.Expression_Peer_PreviousName = function () {
		return this.selPeer != null ? this.selPeer.previousName : "";
	};
	this.Expression_DenyReason = function () {
		return this.threadData.denyReason || "";
	};
	this.Expression_HostIP = function () {
		return this.globals.hostIP || "";
	};
	this.Expression_HostPort = function () {
		return this.globals.hostPort || -1;
	};
	this.Expression_ReplacedExprNoParams = function () {
		return "";
	};
	this.Expression_WelcomeMessage = function () {
		return this.globals.client.welcomeMessage;
	};
	this.Expression_ReceivedBinaryAddress = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		return URL.createObjectURL(new Blob( [ this.threadData.recvMsg ] ));
	};
	this.Expression_CursorASCIIStrByte = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return "";
		}

		const retByte = new Uint8Array(this.threadData.recvMsg)[this.threadData.recvMsg.cursorIndex++];
		// Is ASCII: < 0x7f, is printable: > 0x1f, != 0x7f
		if (retByte > 0x1f && retByte < 0x7f) {
			return String.fromCharCode(retByte);
		}
		this.CreateError("Could not read from received binary, character is not a printable ASCII character.");
		return "";
	};
	this.Expression_CursorUnsignedByte = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getUint8(this.threadData.recvMsg.cursorIndex++);
	};
	this.Expression_CursorSignedByte = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		return new DataView(this.threadData.recvMsg).getInt8(this.threadData.recvMsg.cursorIndex++);
	};
	this.Expression_CursorUnsignedShort = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 2) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		const readFrom = new DataView(this.threadData.recvMsg);
		this.threadData.recvMsg.cursorIndex += 2;
		return readFrom.getUint16(this.threadData.recvMsg.cursorIndex - 2, true);
	};
	this.Expression_CursorSignedShort = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 2) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		const readFrom = new DataView(this.threadData.recvMsg);
		this.threadData.recvMsg.cursorIndex += 2;
		return readFrom.getInt16(this.threadData.recvMsg.cursorIndex - 2, true);
	};
	this.Expression_CursorUnsignedInteger = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 4) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		const readFrom = new DataView(this.threadData.recvMsg);
		this.threadData.recvMsg.cursorIndex += 4;
		return readFrom.getUint32(this.threadData.recvMsg.cursorIndex - 4, true);
	};
	this.Expression_CursorSignedInteger = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 4) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		const readFrom = new DataView(this.threadData.recvMsg);
		this.threadData.recvMsg.cursorIndex += 4;
		return readFrom.getInt32(this.threadData.recvMsg.cursorIndex - 4, true);
	};
	this.Expression_CursorFloat = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return 0;
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 4) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return 0;
		}

		const readFrom = new DataView(this.threadData.recvMsg);
		this.threadData.recvMsg.cursorIndex += 4;
		return readFrom.getFloat32(this.threadData.recvMsg.cursorIndex - 4, true);
	};
	this.Expression_CursorStringWithSize = function (stringSize) {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		if (stringSize < 0) {
			this.CreateError("Could not read from received binary, string size to read less than 0");
			return "";
		}
		else if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < stringSize) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return "";
		}

		return this.$GetRecvMsgSubString(this.threadData.recvMsg.cursorIndex, stringSize, true);
	};
	this.Expression_CursorString = function () {
		if (this.threadData.recvMsg == null || this.threadData.recvMsg.byteLength === 0) {
			return "";
		}
		if (this.threadData.recvMsg.byteLength - this.threadData.recvMsg.cursorIndex < 1) {
			this.CreateError("Could not read from received binary, message is smaller than variable to be read. Check your index");
			return "";
		}

		return this.$GetRecvMsgSubString(this.threadData.recvMsg.cursorIndex, -1, true);
	};
	this.Expression_SendBinaryAddress = function () {
		this.CreateError("Send binary address is not usable in UWP. See help file to read and modify the binary to send.");
		return 0;
	};
	this.Expression_DumpMessage = function () {
		this.CreateError("Dumping binary messages not supported in UWP Bluewing, please " +
						 "contact developer if this is needed");
		return "";
	};
	this.Expression_ChannelListing_ChannelCount = function () {
		return this.globals.client.channelListing.length;
	};
	this.Expression_ConvToUTF8_GetVisibleCharCount = function (str) {
		// We don't convert to UTF8, because JS is strictly UTF-16, and more importantly Unicode graphemes will be
		// the same length regardless of underlying Unicode bit length, because they're the same display.
		if (str.length == 0) {
			return 0;
		}

		// Intl.Segmenter - everywhere except Firefox
		if (this.intlSegmenter != null) {
			return Array.from(this.intlSegmenter['segment'](str)).length;
		}

		// User-included splitter
		if (this.graphemeSplitter != null) {
			return this.graphemeSplitter['countGraphemes'](str);
		}

		// No grapheme splitter; exit with error.
		this.CreateError("Can't get visible character count: browser has no grapheme splitter functionality. " +
			"Firefox notably doesn't support this. Consider including the orling/grapheme-splitter library in your webpage.");
		return -1;
	};
	this.Expression_ConvToUTF8_GetCompleteCodePointCount = function (str) {
		let i = 0;
		for (let c of str) { // iterator handles surrogate pairs, unlike str.length, which will register them separately
			++i;
		}
		return i;
	};
	this.Expression_ConvToUTF8_GetByteCount = function (str) {
		return this.textEncoder.encode(str).byteLength;
	};
	this.Expression_ConvToUTF8_TestAllowList = function (str) {
		this.CreateError("\"Convert to UTF-8 and test allowlist\" expression is not available in UWP port.");
		return "(not available in UWP)";
	};
	// =============================
	// Macros
	// =============================
	this.$ClearBinaryIfSetTo = function () {
		if (this.globals.automaticallyClearBinary) {
			this.globals.sendMsg = new ArrayBuffer(0);
		}
	};
	this.$StringToArrayBuffer = function (text) {
		return this.textEncoder.encode(text);
	};
	this.$NumberToArrayBuffer = function (number) {
		const writeTo = new DataView(new ArrayBuffer(4));
		writeTo.setInt32(0, number, true);
		return writeTo.buffer;
	};
	this.$GetEventNumsForMsg = function (isUDP, variant, ifUDP, ifTCP, ifUDPText, ifUDPNumber, ifUDPBinary,
		ifTCPText, ifTCPNumber, ifTCPBinary) {
		/// <summary> Internal function. Reads data.variant, returns an array of two Numbers,
		/// using the given ifXX params. </summary>
		/// <param name="data" type="Object"> Message data object. </param>
		/// <param name="ifUDP" type="Number"> First number to return if UDP. </param>
		/// <param name="ifTCP" type="Number"> First number to return if TCP. </param>
		/// <param name="ifUDPText" type="Number"> Second number to return if UDP and Variant is 0 (Text). </param>
		/// <param name="ifUDPNumber" type="Number"> Second number to return if UDP and Variant is 1 (Number). </param>
		/// <param name="ifUDPBinary" type="Number"> Second number to return if UDP and Variant is 2 (Binary). </param>
		/// <param name="ifTCPText" type="Number"> Second number to return if TCP and Variant is 0 (Text). </param>
		/// <param name="ifTCPNumber" type="Number"> Second number to return if TCP and Variant is 1 (Number). </param>
		/// <param name="ifTCPBinary" type="Number"> Second number to return if TCP and Variant is 2 (Binary). </param>
		/// <returns type="Array" elementType="Number"> Returns two numbers that correspond or null. </returns>
		let toRet;
		if (isUDP) {
			toRet = [ifUDPText, ifUDPNumber, ifUDPBinary];
		}
		else {
			toRet = [ifTCPText, ifTCPNumber, ifTCPBinary];
		}

		if (variant > 2 || variant < 0) {
			this.CreateError("An unrecognised variant '" + variant + "' was used in a received message");
			return null;
		}
		return [isUDP ? ifUDP : ifTCP, toRet[variant]];
	};
	// Returns 0 if OK. -1 if cut off UTF-8 at front, 1 if cut off at end
	this.$CheckForUTF8Cutoff = function(sv) {
		// Start char is invalid
		let res = this.$GetNumBytesInUTF8Char(sv);

		// 0 = empty string; we can't do anything, return it.
		// -1 = UTF-8 start char, but cut off string; we can't do anything, return it.
		// -2 = UTF-8 non-start char, so start char is cut off.
		if (res <= 0) {
			return res;
		}

		// We don't know the sizeInCodePoints of end char; we'll try for a 1 byte-char at very end, and work backwards and up to max UTF-8 sizeInCodePoints, 4 bytes.
		for (let i = 0, j = (sv.byteLength < 4 ? sv.byteLength : 4); i < j; ++i) {
			// Cut off a char; go backwards
			res = this.$GetNumBytesInUTF8Char(sv.slice(0, sv.byteLength - i));
			if (res == -2) {
				continue;
			}

			// Otherwise, we hit the last start char in the string

			// But it's been cut off; invalid UTF-8
			// 0 = empty string; we can't do anything, return it.
			// -1 = UTF-8 start char, but cut off string; we can't do anything, return it.
			// -2 = UTF-8 non-start char
			return res == -1 ? 1 : 0;
		}

		// Never found a start char; 5-byte/6-byte nonstandard UTF-8?
		return 1;
	};
	// UTF-8 uses a bitmask to determine how many bytes are in the item.
	// Note that this does not verify the ending characters other than a size check; but any TString converter will throw them out.
	this.$GetNumBytesInUTF8Char = function(sv) {
		if (sv.byteLength == 0) {
			return 0;
		}
		if (sv.constructor.name != "Uint8Array") {
			throw "No!";
		}

		const c = sv[0];
		// ASCII/UTF-8 1-byte char
		if (c <= 0x7F) {
			return 1;
		}

		// 2-byte indicator
		if (c >= 0xC2 && c <= 0xDF) {
			return sv.byteLength >= 2 ? 2 : -1;
		}

		// 3-byte indicator
		if (c >= 0xE0 && c <= 0xEF) {
			return sv.byteLength >= 3 ? 3 : -1;
		}

		// 4-byte indicator
		if (c >= 0xF0) {
			return sv.byteLength >= 4 ? 4 : -1;
		}

		// Non-first character in multibyte sequence; user is reading too far ahead
		if (c >= 0x80 && c <= 0xBF) {
			return -2;
		}

		// Note 5-byte and 6-byte variants are theoretically possible but were removed by UTF-8 standard.

		return -1;
	};
	// Called as a subfunction to read string at given position of received binary. If sizeInCodePoints is -1, will expect a null
	// terminator. The isCursorExpression is used for error messages.
	this.$GetRecvMsgSubString = function(recvMsgStartIndex, sizeInCodePoints, isCursorExpression) {
		// User requested empty size, let 'em have it
		if (sizeInCodePoints == 0) {
			return "";
		}

		if (recvMsgStartIndex < 0) {
			this.CreateError("Could not read from received binary, index less than 0.");
			return "";
		}
		if (recvMsgStartIndex > this.threadData.recvMsg.byteLength) {
			this.CreateError("Could not read from received binary, index " + recvMsgStartIndex + " is outside range of 0 to " +
				Math.max(0, this.threadData.recvMsg.byteLength - 1) + ".");
			return "";
		}

		if (sizeInCodePoints < -1) {
			this.CreateError("Could not read string with size " + sizeInCodePoints + "; size is too low.");
			return "";
		}
		const fixedSize = sizeInCodePoints != -1;

		const maxSize = this.threadData.recvMsg.byteLength - recvMsgStartIndex;
		let actualStringSizeBytes = new Uint8Array(this.threadData.recvMsg, recvMsgStartIndex, maxSize).indexOf(0);
		if (actualStringSizeBytes == -1) {
			actualStringSizeBytes = maxSize;
		}
		if (fixedSize) {
			// Size too small - we assumed every char was 1-byte for this, so it's way under
			if (sizeInCodePoints != -1 && sizeInCodePoints > maxSize) {
				this.CreateError("Could not read string with size " + sizeInCodePoints + " at " + (isCursorExpression ? "cursor's " : "") +
					"start index " + recvMsgStartIndex + ", only " + maxSize + " possible characters in message.");
				return "";
			}

			// Null terminator found within string
			if (actualStringSizeBytes < sizeInCodePoints) {
				this.CreateError("Could not read string with size " + sizeInCodePoints + " at " + (isCursorExpression ? "cursor's " : "") +
					"start index " + recvMsgStartIndex + ", found null byte within at index " + (recvMsgStartIndex + actualStringSizeBytes) + ".");
				return "";
			}
		}
		// Not fixed size; if null terminator not found within remainder of text, then whoops.
		// We are expecting (actualStringSizeBytes < maxSize) if found.
		else if (actualStringSizeBytes == maxSize) {
			this.CreateError("Could not read null-terminated string from " + (isCursorExpression ? "cursor's " : "") + "start index " +
				recvMsgStartIndex + "; null terminator not found.");
			return "";
		}

		// To make sure user hasn't cut off the start/end UTF-8 char, we'll do a quick check
		const result = new Uint8Array(this.threadData.recvMsg, recvMsgStartIndex, actualStringSizeBytes);

		// Start char is invalid
		if (this.$GetNumBytesInUTF8Char(result) < 0) {
			this.CreateError("Could not read text from received binary, UTF-8 char was cut off at " +
				(isCursorExpression ? "the cursor's " : "") + "start index " + this.threadData.recvMsg.cursorIndex + ".");
			return "";
		}

		// We have the entire received message in result, we need to trim it to sizeInCodePoints

		// We don't know the sizeInCodePoints of end char; we'll try for a 1 byte-char at very end, and work backwards and up to max UTF-8 sizeInCodePoints, 4 bytes.
		for (let codePointIndex = 0, numBytesRead = 0, byteIndex = 0, remainder = result.byteLength; ; ) {
			let numBytes = this.$GetNumBytesInUTF8Char(result.slice(byteIndex, byteIndex + Math.min(remainder, 4)));

			// We checked for -2 in start char in previous if(), so the string isn't starting too early.
			// So, a -2 in middle of the string means it's a malformed UTF-8.
			// We'll catch both -1 and -2 as malformed UTF-8 errors.
			if (numBytes < 0) {
				this.CreateError("Could not read text from received binary, UTF-8 was malformed at index " + (recvMsgStartIndex + byteIndex) +
					" (attempted to read " + byteIndex + " chars from " + (isCursorExpression ? "the cursor's " : "") + "start index " + recvMsgStartIndex + ").");
				return "";
			}

			// otherwise, valid char
			++byteIndex;

			// loop all 2nd+ bytes used by it, validate 'em as UTF-8 continuations
			for (let i = 1; i < numBytes; ++i) {
				const c = result[byteIndex++];
				if (c < 0x80 || c > 0xBF) {
					numBytes = -1;
					this.CreateError("Could not read text from received binary, UTF-8 was malformed at index " + (recvMsgStartIndex + byteIndex) +
						" (attempted to read " + byteIndex + " chars from " + (isCursorExpression ? "the cursor's " : "") + "start index " + recvMsgStartIndex + ").");
					return "";
				}
			}

			// Easier looping - note, numBytes == 0 for null terminator
			remainder -= numBytes;
			numBytesRead += numBytes;

			// Okay, we read a full character (aka a code point)
			++codePointIndex;

			// Either it's null byte-terminated and we're at the null; or got all the characters we need
			if ((!fixedSize && numBytes == 0) || (fixedSize && sizeInCodePoints == codePointIndex)) {
				// $U8StrValidate will do a more thorough investigation of characters, +1 for null terminator
				if (isCursorExpression) {
					this.threadData.recvMsg.cursorIndex += numBytesRead + (fixedSize ? 0 : 1);
				}

				const resStr = result.slice(0, numBytesRead);
				if (this.$U8StrValidate(resStr)) {
					return this.textDecoder.decode(resStr);
				}

				this.CreateError("Could not read text from received binary, UTF-8 was malformed at index " + (recvMsgStartIndex + byteIndex) +
					" (attempted to read " + byteIndex + " chars from " + (isCursorExpression ? "the cursor's " : "") + "start index " + recvMsgStartIndex + ").");
				return "";
			}

			// grab another character
			// continue;
		}
		// Shouldn't reach here
	};

	this.$U8StrValidate = function(str) {
		try {
			this.textDecoder.decode(str);
			return true;
		}
		catch(Error) { return false; }
	};

	// =============================
	// Function arrays
	// =============================

	this.$actionFuncs = [
	/* 0 */ this.Action_Replaced_Connect,
	/* 1 */ this.Action_Disconnect,
	/* 2 */ this.Action_SetName,
	/* 3 */ this.Action_Replaced_JoinChannel,
	/* 4 */ this.Action_LeaveChannel,
	/* 5 */ this.Action_SendTextToServer,
	/* 6 */ this.Action_SendTextToChannel,
	/* 7 */ this.Action_SendTextToPeer,
	/* 8 */ this.Action_SendNumberToServer,
	/* 9 */ this.Action_SendNumberToChannel,
	/* 10 */ this.Action_SendNumberToPeer,
	/* 11 */ this.Action_BlastTextToServer,
	/* 12 */ this.Action_BlastTextToChannel,
	/* 13 */ this.Action_BlastTextToPeer,
	/* 14 */ this.Action_BlastNumberToServer,
	/* 15 */ this.Action_BlastNumberToChannel,
	/* 16 */ this.Action_BlastNumberToPeer,
	/* 17 */ this.Action_SelectChannelByName,
	/* 18 */ this.Action_ReplacedNoParams,
	/* 19 */ this.Action_LoopClientChannels,
	/* 20 */ this.Action_SelectPeerOnChannelByName,
	/* 21 */ this.Action_SelectPeerOnChannelByID,
	/* 22 */ this.Action_LoopPeersOnChannel,
	/* 23 */ this.Action_ReplacedNoParams,
	/* 24 */ this.Action_ReplacedNoParams,
	/* 25 */ this.Action_ReplacedNoParams,
	/* 26 */ this.Action_ReplacedNoParams,
	/* 27 */ this.Action_ReplacedNoParams,
	/* 28 */ this.Action_ReplacedNoParams,
	/* 29 */ this.Action_ReplacedNoParams,
	/* 30 */ this.Action_RequestChannelList,
	/* 31 */ this.Action_LoopListedChannels,
	/* 32 */ this.Action_ReplacedNoParams,
	/* 33 */ this.Action_ReplacedNoParams,
	/* 34 */ this.Action_ReplacedNoParams,
	/* 35 */ this.Action_SendBinaryToServer,
	/* 36 */ this.Action_SendBinaryToChannel,
	/* 37 */ this.Action_SendBinaryToPeer,
	/* 38 */ this.Action_BlastBinaryToServer,
	/* 39 */ this.Action_BlastBinaryToChannel,
	/* 40 */ this.Action_BlastBinaryToPeer,
	/* 41 */ this.Action_AddASCIITextByte,
	/* 42 */ this.Action_AddByteInt,
	/* 43 */ this.Action_AddShort,
	/* 44 */ this.Action_AddInt,
	/* 45 */ this.Action_AddFloat,
	/* 46 */ this.Action_AddStringWithoutNull,
	/* 47 */ this.Action_AddString,
	/* 48 */ this.Action_AddBinary,
	/* 49 */ this.Action_ClearBinaryToSend,
	/* 50 */ this.Action_SaveReceivedBinaryToFile,
	/* 51 */ this.Action_AppendReceivedBinaryToFile,
	/* 52 */ this.Action_AddFileToBinary,
	/* 53 */ this.Action_ReplacedNoParams,
	/* 54 */ this.Action_ReplacedNoParams,
	/* 55 */ this.Action_ReplacedNoParams,
	/* 56 */ this.Action_ReplacedNoParams,
	/* 57 */ this.Action_ReplacedNoParams,
	/* 58 */ this.Action_ReplacedNoParams,
	/* 59 */ this.Action_ReplacedNoParams,
	/* 60 */ this.Action_ReplacedNoParams,
	/* 61 */ this.Action_ReplacedNoParams,
	/* 62 */ this.Action_ReplacedNoParams,
	/* 63 */ this.Action_ReplacedNoParams,
	/* 64 */ this.Action_SelectChannelMaster,
	/* 65 */ this.Action_JoinChannel,
	/* 66 */ this.Action_CompressSendBinary,
	/* 67 */ this.Action_DecompressReceivedBinary,
	/* 68 */ this.Action_MoveReceivedBinaryCursor,
	/* 69 */ this.Action_LoopListedChannelsWithLoopName,
	/* 70 */ this.Action_LoopClientChannelsWithLoopName,
	/* 71 */ this.Action_LoopPeersOnChannelWithLoopName,
	/* 72 */ this.Action_ReplacedNoParams,
	/* 73 */ this.Action_Connect,
	/* 74 */ this.Action_ResizeBinaryToSend,
	// Blue-only actions
	/* 75 */ this.Action_SetDestroySetting,
	/* 76 */ this.Action_SetLocalPortForHolePunch
	];
	this.$conditionFuncs = [
	/* 0 */ this.Condition_MandatoryTriggeredEvent, /* OnError */
	/* 1 */ this.Condition_AlwaysTrue, /* OnConnect */
	/* 2 */ this.Condition_MandatoryTriggeredEvent, /* OnConnectDenied */
	/* 3 */ this.Condition_MandatoryTriggeredEvent, /* OnDisconnect */
	/* 4 */ this.Condition_AlwaysTrue, /* OnChannelJoin */
	/* 5 */ this.Condition_MandatoryTriggeredEvent, /* OnChannelJoinDenied */
	/* 6 */ this.Condition_AlwaysTrue, /* OnNameSet */
	/* 7 */ this.Condition_MandatoryTriggeredEvent, /* OnNameDenied */
	/* 8 */ this.Condition_OnSentTextMessageFromServer,
	/* 9 */ this.Condition_OnSentTextMessageFromChannel,
	/* 10 */ this.Condition_AlwaysTrue, /* OnPeerConnect */
	/* 11 */ this.Condition_AlwaysTrue, /* OnPeerDisonnect */
	/* 12 */ this.Condition_AlwaysFalse, /* Replaced_OnChannelJoin */
	/* 13 */ this.Condition_AlwaysTrue, /* OnChannelPeerLoop */
	/* 14 */ this.Condition_AlwaysTrue, /* OnClientChannelLoop */
	/* 15 */ this.Condition_OnSentNumberMessageFromServer,
	/* 16 */ this.Condition_OnSentNumberMessageFromChannel,
	/* 17 */ this.Condition_AlwaysTrue, /* OnChannelPeerLoopFinished */
	/* 18 */ this.Condition_AlwaysTrue, /* OnClientChannelLoopFinished */
	/* 19 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 20 */ this.Condition_OnBlastedTextMessageFromServer,
	/* 21 */ this.Condition_OnBlastedNumberMessageFromServer,
	/* 22 */ this.Condition_OnBlastedTextMessageFromChannel,
	/* 23 */ this.Condition_OnBlastedNumberMessageFromChannel,
	/* 24 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 25 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 26 */ this.Condition_AlwaysTrue, /* OnChannelListReceived */
	/* 27 */ this.Condition_AlwaysTrue, /* OnChannelListLoop */
	/* 28 */ this.Condition_AlwaysTrue, /* OnChannelListLoopFinished */
	/* 29 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 30 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 31 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 32 */ this.Condition_OnSentBinaryMessageFromServer,
	/* 33 */ this.Condition_OnSentBinaryMessageFromChannel,
	/* 34 */ this.Condition_OnBlastedBinaryMessageFromServer,
	/* 35 */ this.Condition_OnBlastedBinaryMessageFromChannel,
	/* 36 */ this.Condition_OnSentTextMessageFromPeer,
	/* 37 */ this.Condition_OnSentNumberMessageFromPeer,
	/* 38 */ this.Condition_OnSentBinaryMessageFromPeer,
	/* 39 */ this.Condition_OnBlastedTextMessageFromPeer,
	/* 40 */ this.Condition_OnBlastedNumberMessageFromPeer,
	/* 41 */ this.Condition_OnBlastedBinaryMessageFromPeer,
	/* 42 */ this.Condition_IsConnected,
	/* 43 */ this.Condition_AlwaysTrue, /* OnChannelLeave */
	/* 44 */ this.Condition_MandatoryTriggeredEvent, /* OnChannelLeaveDenied */
	/* 45 */ this.Condition_AlwaysTrue, /* OnPeerChangedName */
	/* 46 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 47 */ this.Condition_OnAnySentMessageFromServer,
	/* 48 */ this.Condition_OnAnySentMessageFromChannel,
	/* 49 */ this.Condition_OnAnySentMessageFromPeer,
	/* 50 */ this.Condition_OnAnyBlastedMessageFromServer,
	/* 51 */ this.Condition_OnAnyBlastedMessageFromChannel,
	/* 52 */ this.Condition_OnAnyBlastedMessageFromPeer,
	/* 53 */ this.Condition_MandatoryTriggeredEvent, /* OnNameChanged */
	/* 54 */ this.Condition_ClientHasAName,
	/* 55 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 56 */ this.Condition_AlwaysFalse, /* ReplacedCondNoParams */
	/* 57 */ this.Condition_SelectedPeerIsChannelMaster,
	/* 58 */ this.Condition_YouAreChannelMaster,
	/* 59 */ this.Condition_OnChannelListLoopWithName,
	/* 60 */ this.Condition_OnChannelListLoopWithNameFinished,
	/* 61 */ this.Condition_OnPeerLoopWithName,
	/* 62 */ this.Condition_OnPeerLoopWithNameFinished,
	/* 63 */ this.Condition_OnClientChannelLoopWithName,
	/* 64 */ this.Condition_OnClientChannelLoopWithNameFinished,
	/* 65 */ this.Condition_OnSentTextChannelMessageFromServer,
	/* 66 */ this.Condition_OnSentNumberChannelMessageFromServer,
	/* 67 */ this.Condition_OnSentBinaryChannelMessageFromServer,
	/* 68 */ this.Condition_OnAnySentChannelMessageFromServer,
	/* 69 */ this.Condition_OnBlastedTextChannelMessageFromServer,
	/* 70 */ this.Condition_OnBlastedNumberChannelMessageFromServer,
	/* 71 */ this.Condition_OnBlastedBinaryChannelMessageFromServer,
	/* 72 */ this.Condition_OnAnyBlastedChannelMessageFromServer,
	// Added Blue-only conditions
	/* 73 */ this.Condition_IsJoinedToChannel,
	/* 74 */ this.Condition_IsPeerOnChannel_Name,
	/* 75 */ this.Condition_IsPeerOnChannel_ID

	// update getNumOfConditions function if you edit this!!!!
	];
	this.$expressionFuncs = [
	/* 0 */ this.Expression_Error,
	/* 1 */ this.Expression_ReplacedExprNoParams,
	/* 2 */ this.Expression_Self_Name,
	/* 3 */ this.Expression_Self_ChannelCount,
	/* 4 */ this.Expression_Peer_Name,
	/* 5 */ this.Expression_ReceivedStr,
	/* 6 */ this.Expression_ReceivedInt,
	/* 7 */ this.Expression_Subchannel,
	/* 8 */ this.Expression_Peer_ID,
	/* 9 */ this.Expression_Channel_Name,
	/* 10 */ this.Expression_Channel_PeerCount,
	/* 11 */ this.Expression_ReplacedExprNoParams,
	/* 12 */ this.Expression_ChannelListing_Name,
	/* 13 */ this.Expression_ChannelListing_PeerCount,
	/* 14 */ this.Expression_Self_ID,
	/* 15 */ this.Expression_ReplacedExprNoParams,
	/* 16 */ this.Expression_ReplacedExprNoParams,
	/* 17 */ this.Expression_ReplacedExprNoParams,
	/* 18 */ this.Expression_ReplacedExprNoParams,
	/* 19 */ this.Expression_ReplacedExprNoParams,
	/* 20 */ this.Expression_ASCIIStrByte,
	/* 21 */ this.Expression_UnsignedByte,
	/* 22 */ this.Expression_SignedByte,
	/* 23 */ this.Expression_UnsignedShort,
	/* 24 */ this.Expression_SignedShort,
	/* 25 */ this.Expression_UnsignedInteger,
	/* 26 */ this.Expression_SignedInteger,
	/* 27 */ this.Expression_Float,
	/* 28 */ this.Expression_StringWithSize,
	/* 29 */ this.Expression_String,
	/* 30 */ this.Expression_ReceivedBinarySize,
	/* 31 */ this.Expression_Lacewing_Version,
	/* 32 */ this.Expression_SendBinarySize,
	/* 33 */ this.Expression_Self_PreviousName,
	/* 34 */ this.Expression_Peer_PreviousName,
	/* 35 */ this.Expression_ReplacedExprNoParams,
	/* 36 */ this.Expression_ReplacedExprNoParams,
	/* 37 */ this.Expression_DenyReason,
	/* 38 */ this.Expression_HostIP,
	/* 39 */ this.Expression_HostPort,
	/* 40 */ this.Expression_ReplacedExprNoParams,
	/* 41 */ this.Expression_WelcomeMessage,
	/* 42 */ this.Expression_ReceivedBinaryAddress,
	/* 43 */ this.Expression_CursorASCIIStrByte,
	/* 44 */ this.Expression_CursorUnsignedByte,
	/* 45 */ this.Expression_CursorSignedByte,
	/* 46 */ this.Expression_CursorUnsignedShort,
	/* 47 */ this.Expression_CursorSignedShort,
	/* 48 */ this.Expression_CursorUnsignedInteger,
	/* 49 */ this.Expression_CursorSignedInteger,
	/* 50 */ this.Expression_CursorFloat,
	/* 51 */ this.Expression_CursorStringWithSize,
	/* 52 */ this.Expression_CursorString,
	/* 53 */ this.Expression_ReplacedExprNoParams,
	/* 54 */ this.Expression_SendBinaryAddress,
	// Added Blue-only expressions
	/* 55 */ this.Expression_DumpMessage,
	/* 56 */ this.Expression_ChannelListing_ChannelCount,
	/* 57 */ this.Expression_ConvToUTF8_GetCompleteCodePointCount,
	/* 58 */ this.Expression_ConvToUTF8_GetVisibleCharCount,
	/* 59 */ this.Expression_ConvToUTF8_GetByteCount,
	/* 60 */ this.Expression_ConvToUTF8_TestAllowList
	];
}
//
CRunBluewing_Client.prototype = {
	/// <summary> Prototype definition </summary>
	/// <description> This class is a sub-class of CRunExtension, by the mean of the
	/// CServices.extend function which copies all the properties of
	/// the parent class to the new class when it is created.
	/// As all the necessary functions are defined in the parent class,
	/// you only need to keep the ones that you actually need in your code. </description>

	getNumberOfConditions: function() {
		/// <summary> Returns the number of conditions </summary>
		/// <returns type="Number" isInteger="true"> Warning, if this number is not correct, the application _will_ crash</returns>
		return 76; // $conditionFuncs not available yet
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

		// Text encoders
		this.textEncoder = new TextEncoder(/* utf-8 assumed */);
		this.textDecoder = new TextDecoder('utf-8');

		// We have to read object properties manually, since C++ version doesn't use DarkEdif smart properties

		file.skipBytes(5); // there is padding to match Relay's offsets

		let edPtr = {
			automaticallyClearBinary : file.readAByte() !== 0,
			multiThreading : file.readAByte() !== 0,
			isGlobal : file.readAByte() !== 0,
			globalID : file.readBuffer(255),
			timeoutWarningEnabled : file.readAByte() !== 0,
			fullDeleteEnabled : file.readAByte() !== 0
			// As a side note, 256 padding bytes follow in edPtr
		};
		edPtr.globalID = new Uint8Array(edPtr.globalID); // Array to UInt8Array
		edPtr.globalID = edPtr.globalID.slice(0, edPtr.globalID.findIndex(function (c) { return c===0; })); // Trim to null UTF-8 character
		edPtr.globalID = this.textDecoder.decode(edPtr.globalID); // Decode UTF-8 to JS string (UTF-16, generally)

		this.isGlobal = edPtr.isGlobal;
		if (this.isGlobal) {
			let fromRuntime = darkEdif.getGlobalData(edPtr.globalID + "BlueClient");
			if (fromRuntime != null && fromRuntime.pendingDelete) {
				darkEdif.consoleLog(this, "Found old ID: deleting");
				fromRuntime.Delete();
				fromRuntime = null;
				darkEdif.setGlobalData(edPtr.globalID + "BlueClient", null);
			}
			if (fromRuntime == null) {
				this.globals = new BluewingClient_GlobalInfo(this, edPtr);
				darkEdif.setGlobalData(edPtr.globalID + "BlueClient", this.globals);
			}
			else { // Add this Extension to extsHoldingGlobals to inherit control later
				this.globals = fromRuntime;
				if (this.globals.lastDestroyedExtSelectedChannel != null) {
					this.selChannel = this.globals.lastDestroyedExtSelectedChannel;
					this.globals.lastDestroyedExtSelectedChannel = null;
				}
				if (this.globals.lastDestroyedExtSelectedPeer != null) {
					this.selPeer = this.globals.lastDestroyedExtSelectedPeer;
					this.globals.lastDestroyedExtSelectedPeer = null;
				}

				this.globals.extsHoldingGlobals.push(this);
				if (!this.globals.ext) {
					this.globals.client.ext = this.globals.ext = this;
					clearTimeout(this.globals.timeoutThread);
				}
			}
		}
		else {
			// non-global Blue Client
			this.globals = new BluewingClient_GlobalInfo(this, edPtr);
		}

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
		const mandatoryEvents = [
			{ name: "On Error", id: 0 },
			{ name: "On Connection Denied", id: 2 },
			{ name: "On Disconnect", id: 3 },
			{ name: "On Join Denied", id: 5 },
			{ name: "On Name Denied", id: 7 },
			{ name: "On Leave Denied", id: 44 },
			{ name: "On Name Changed", id: 53 },
		];
		const maxNumEventsPerEventLoop = 10;
		let remainingCount = 0;
		let runNextLoop = !this.globals.multiThreading;

		for (let maxTrig = 0; maxTrig < maxNumEventsPerEventLoop; ++maxTrig) {
			if (this.globals.eventsToRun.length == 0) {
				this.isOverloadWarningQueued = false;
				break;
			}
			const evtToRun = this.globals.eventsToRun.shift();
			remainingCount = this.globals.eventsToRun.length;

			const mandatoryEvent = mandatoryEvents.find(function (me) { return me.id == evtToRun.idList[0]; } );
			if (mandatoryEvent != null) {
				this.lastMandatoryEventWasChecked = false;
			}

			const id2 = evtToRun.idList[1]; // run second event, or clear data
			// Loop through every ext to trigger the events on it
			for (let i of this.globals.extsHoldingGlobals) {
				const origSelChannel = i.selChannel;
				const origSelPeer = i.selPeer;
				const origTData = i.threadData; // may not be needed
				i.threadData = evtToRun;
				i.selChannel = evtToRun.channel;
				i.selPeer = evtToRun.peer;
				i.ho.generateEvent(evtToRun.idList[0]); // run first event

				if (id2 !== this.CLEAR_EVTNUM && id2 != null) {
					i.ho.generateEvent(id2);
				}

				// Restore old selection - if there was a selection
				i.threadData = origTData;
				if (origSelChannel != null) {
					i.selChannel = origSelChannel;
				}
				if (origSelPeer != null) {
					i.selPeer = origSelPeer;
				}
			} // each ext

			if (id2 === this.CLEAR_EVTNUM) {
				// On disconnect, clear everything
				if (evtToRun.channel == null) {
					this.globals.hostIP = "";
					this.globals.hostPort = -1;
					this.globals.previousName = "";
					this.threadData.denyReason = "";
				}
				// No channel in evtToRun: full deselect of all channels/peers
				for (let i of this.globals.extsHoldingGlobals) {
					if (evtToRun.channel == null || i.selChannel == evtToRun.channel) {
						// If evtToRun.peer is not specified, it's channel leave; deselect both channel and peer selection
						// If evtToRun.peer is specified, it's peer leave; only deselect peer, if peer matches evtToRun.peer
						if (evtToRun.peer == null) {
							i.selChannel = null;
						}
						if (evtToRun.peer == null || evtToRun.peer == i.selPeer) {
							i.selPeer = null;
						}
					}
				}
			} // clear event check

			// On Error or other mandatory events were not processed by Fusion events. Shameful.
			if (!this.globals.lastMandatoryEventWasChecked) {
				let err = "Bluewing Client event occurred, but you have no \"Bluewing Client\" > " +
					mandatoryEvent.name + "\" event to handle it. That is BAD PRACTICE";

				// On Error has message
				if (mandatoryEvent.name == "On Error") {
					err += ". Error message:\n" + evtToRun.error;
				}
				// On Disconnect and On Name Changed has no text included
				else if (mandatoryEvent.name == "On Disconnect" || mandatoryEvent.name == "On Name Changed") {
					err += '.';
				}
				else {
					err += ". Deny reason:\n" + this.threadData.denyReason;
				}

				alert(err);
				this.globals.lastMandatoryEventWasChecked = true; // reset for next loop
			}
		} // max events triggered per fps

		if (!this.isOverloadWarningQueued && remainingCount > maxNumEventsPerEventLoop * 3) {
			// Create an error and move it to the front of the queue
			this.CreateError("You're receiving too many messages for the application to process. Max of " + maxNumEventsPerEventLoop +
				" events per event loop, currently " + this.globals.eventsToRun.length + " messages in queue.");
			const errEvt = this.globals.eventsToRun.pop();
			this.globals.eventsToRun.splice(0, 0, errEvt);
			this.isOverloadWarningQueued = true;
		}
		runNextLoop |= remainingCount > 0;
		return runNextLoop ? 0 : CRunExtension.REFLAG_ONESHOT; // Do not call next loop
	},
	destroyRunObject: function(bFast) {
		/// <summary> Destruction of the object.</summary>
		/// <description> Called when the object is actually destroyed. This will always be
		/// after the main game loop, and out of the actions processing : the
		/// destroy process is queued until the very end of the game loop. </description>
		/// <param name="bFast" type="Boolean"> Fast close. Usage unknown. </param>

		this.isDestroyed = true;
		darkEdif.consoleLog(this, "Was destroyed!");

		// Remove this Extension from liblacewing usage.
		const i = this.globals.extsHoldingGlobals.findIndex(function (e) { return e == this; }, this);
		const wasBegin = i == 0;
		this.globals.extsHoldingGlobals.splice(i, 1);

		this.threadData = new BluewingClient_EventToRun(null, {});

		// Shift secondary event management to other Extension, if any
		if (this.globals.extsHoldingGlobals.length > 0) {
			// Switch Handle ticking over to next Extension visible.
			if (wasBegin) {
				this.globals.client.ext = this.globals.ext = this.globals.extsHoldingGlobals[0];
				this.globals.ext.ho.reHandle();
			}
			// else, this extension wasn't even the main event handler (for Handle()/globals).
		}
		// Last instance of this object; if global and not full-delete-enabled, do not cleanup.
		// In single-threaded instances, this will cause a dirty timeout; the lower-level protocols,
		// e.g. TCP, will close the connection after a certain amount of not-responding.
		// In multi-threaded instances, messages will continue to be queued, and this will retain
		// the connection indefinitely.
		else {
			if (!this.globals.client.isConnected) {
				darkEdif.consoleLog(this, "Bluewing Client - Not connected, nothing important to retain, closing Globals info.\n");
			}
			else if (!this.isGlobal) {
				darkEdif.consoleLog(this, "Bluewing Client - Not global, closing Globals info.\n");
			}
			else if (this.globals.fullDeleteEnabled) {
				darkEdif.consoleLog(this, "Bluewing Client - Full delete enabled, closing Globals info.\n");
			}
			// This can never be true in UWP. Fusion runtime doesn't respond to end application requests (outside of subapps),
			// and runs no code when window is closed; it just instantly dies, not even calling the On Destroy actions.
			// else if (AppWasClosed == true) {
			//   darkEdif.consoleLog(this, "Bluewing Client - App was closed, closing Globals info.\n");
			// }
			else { // !globals.fullDeleteEnabled
				darkEdif.consoleLog(this, "Bluewing Client - Last instance dropped, and currently connected - " +
					"Globals will be retained until a Disconnect is called.\n");
				this.globals.client.ext = this.globals.ext = null;
				this.globals.lastDestroyedExtSelectedChannel = this.selChannel;
				this.globals.lastDestroyedExtSelectedPeer = this.selPeer;
				this.threadData = new BluewingClient_EventToRun(null, {});
				this.selPeer = null;
				this.selChannel = null;

				darkEdif.consoleLog(this, "Bluewing Client - Timeout thread started. If no instance has reclaimed ownership in 3 seconds, " +
					(this.globals.timeoutWarningEnabled ? "a warning message will be shown"
					: "the connection will terminate and all pending messages will be discarded") + ".\n");

				// Note the timeout thread does not delete globals. It can't, as Runtime.WriteGlobal() requires a valid Extension.
				// Instead, the thread marks it as pending delete, and in ReadGlobal in Extension ctor, it checks if it's
				// pending delete and deletes there.
				const self = this;
				this.globals.timeoutThread = setTimeout(
					function() {
						if (self.globals.ext == null) {
							if (self.globals.timeoutWarningEnabled) {
								alert("You did not reclaim the global Bluewing Client object in time.");
							}
							darkEdif.setGlobalData(self.globals.globalID + "BlueClient", null);
							self.globals.Delete();
							darkEdif.consoleLog(self, "Timeout thread erased unclaimed global Bluewing Client data.");
						}
					}, 3 * 1000
				);
				return;
			}
			const id = this.globals.globalID + "BlueClient";
			darkEdif.setGlobalData(id, null);

			// Due to the shared_ptr dtor potentially freeing ID, the deselection must be before globals delete
			this.selPeer = null;
			this.selChannel = null;

			this.globals.Delete();
			//delete globals; // Disconnects and closes event pump, deletes lock
			this.globals = null;
		}

		this.selPeer = null;
		this.selChannel = null;
	},

	condition: function(num, cnd) {
		/// <summary> Called when a condition of this object is tested. </summary>
		/// <param name="num" type="Number" integer="true"> The number of the condition; 0+. </param>
		/// <param name="cnd" type="CCndExtension"> a CCndExtension object, allowing you to retreive the parameters
		//			of the condition. </param>
		/// <returns type="Boolean"> True if the condition is currently true. </returns>

		const func = this.$conditionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised condition ID " + (~~num) + " was called in " + this['ExtensionName'] + ".";
		}

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
			throw "Unrecognised action ID " + (~~num) + " was called in " + this['ExtensionName'] + ".";
		}

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
			throw "Unrecognised expression ID " + (~~num) + " was called in " + this['ExtensionName'] + ".";
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

CServices.extend(CRunExtension, CRunBluewing_Client);

// Template code -> Bluewing specifics
/** @constructor */
function BluewingClient_GlobalInfo(ext, edPtr) {
	this.objEventPump = { tick: function () { return null; } };
	this.client = new Bluewing_Client(ext);
	// Check to make sure current non-ignorable triggered event was processed by Fusion events
	this.lastMandatoryEventWasChecked = true;

	// Server's IP address, set during connect; no port
	this.hostIP = "";
	this.hostPort = -1;

	// Binary message to send
	this.sendMsg = new Uint8Array(0);
	this.sendMsgLocked = false; // If loading a Blob, we'll need this

	// Previous name of this client
	this.previousName = "";
	// Last deny reason, set during Handle() when running deny events
	this.denyReason = "";
	// Clear binary when a binary message is sent/blasted?
	this.automaticallyClearBinary = edPtr.automaticallyClearBinary;

	this.multiThreading = edPtr.multiThreading;

	// This GlobalInfo global ID of extension, in UTF-8
	this.globalID = edPtr.globalID;
	// If in multithreading mode, the Lacewing message handler thread
	this.thread = { joinable: function() { return false; }, join: function() { } };
	// Current "owner" extension used to run events. Can be null, e.g. during frame switches
	this.ext = ext;
	// Thread handle; Thread checking whether a client extension has not regained control of connection in a reasonable time, i.e. slow frame transition
	this.timeoutThread = null;

	// Used to keep Fusion selection across frames
	this.lastDestroyedExtSelectedChannel = null;
	this.lastDestroyedExtSelectedPeer = null;

	// Queued conditions to trigger, with selected client/channel
	this.eventsToRun = [];

	// List of all extensions holding this Global ID
	this.extsHoldingGlobals = [ ext ];
	// If no Bluewing exists, fuss after a preset time period
	this.timeoutWarningEnabled = edPtr.timeoutWarningEnabled;
	// If no Bluewing exists after DestroyRunObject, clean up this GlobalInfo
	this.fullDeleteEnabled = edPtr.fullDeleteEnabled;
	// Used to determine if an error event happened in a Fusion event, e.g. user put in bad parameter.
	// Fusion code always runs in main thread, but errors can occur outside of user input.
	this.mainThreadID = 0;
	// Max size of a UDP message - good values are 1400 bytes for Ethernet MTU,
	// and 576 bytes for minimum IPv4 packet transmissible without fragmentation.
	// Another size of note is a bit under 16KiB, due to SSL record size + Lacewing headers.
	this.maxUDPSize = this.client.relay_max_udp_payload;

	// Due to Runtime.WriteGlobal() not working if there's no Extension,
	// or not working mid-frame transition, we'll have to just fake its deletion,
	// and not free memory until later.
	this.pendingDelete = false;
	this.MarkAsPendingDelete = function() {
		if (this.pendingDelete) {
			return;
		}

		// Let the memory be freed later by _client dtor.
		this.lastDestroyedExtSelectedChannel = null;
		this.lastDestroyedExtSelectedPeer = null;

		// Cleanup all usages of GlobalInfo
		if (!this.thread.joinable()) {
			this.objEventPump.tick();
		}

		this.client.isDead = true;

		if (this.client.isConnected || this.client.isConnecting) {
			this.client.Disconnect();
		}

		// Multithreading mode; wait for thread to end
		if (this.thread.joinable()) {
			this.thread.join();

			darkEdif.consoleLog(this.ext, "Lacewing loop thread should have ended.\n");
		}
		else { // single-threaded; tick so all pending events are parsed, like the eventloop exit
			const err = this.objEventPump.tick();
			if (err != null) {
				// No way to report it to Fusion; the last ext is being destroyed.
				darkEdif.consoleLog(this.ext, "Pump closed with error \"" + err + "\". Lacewing loop thread should have ended.\n");
			}
			darkEdif.consoleLog(this.ext, "Pump should be closed.\n");
			// yield
		}
		this.client = null;

	};
	this.Delete = function() {
		if (this.extsHoldingGlobals.length != 0) {
			alert("GlobalInfo dtor called prematurely.");
		}

		if (!this.pendingDelete) {
			this.MarkAsPendingDelete();
		}
	};
}
/** @constructor */
function BluewingClient_EventToRun(ids, data) {
	/// <summary> Creates a variable containing peer, channel, error etc, used by events. </summary>
	/// <param name="ids" type="Array" elementType="Number">
	///		Array of Fusion condition IDs to generate events for. </param>
	/// <param name="peer" type="Bluewing_Peer" mayBeNull="true">
	///		A Bluewing Peer Object that's relevant for these events. Can be null. </param>
	/// <param name="channel" type="Bluewing_Channel" mayBeNull="true">
	///		A Bluewing Channel Object that's relevant for these events. Can be null. </param>
	/// <param name="loopNameStr" type="String" mayBeNull="true">
	///		Name of the loop triggering these events. Can be null. </param>
	/// <param name="recvMsg" type="ArrayBuffer" mayBeNull="true">
	///		Received message that triggered these events. Can be null. </param>
	/// <param name="errorStr" type="String" mayBeNull="true">
	///		The name of the error. Can be null. See this.CreateError. </param>
	/// <param name="denyReason" type="String" mayBeNull="true">
	///		The reason the thing was denied. Can be null. </param>
	/// <param name="channelListing" type="Bluewing_ChannelListing" mayBeNull="true">
	///		The Channel Listing loop's current channel. </param>
	/// <param name="deniedChannelName" type="string" mayBeNull="true">
	///		Only used by ChannelJoin_Denied. </param>
	this.idList = ids || null;
	if (this.idList != null && this.idList.length == 0) {
		throw "DataQueue ctor was called, but an empty array was specified. Use null for dummies.";
	}
	this.peer = data.peer || null;
	this.channel = data.channel || null;
	this.recvMsg = data.msg || null;
	if (this.recvMsg != null) {
		this.recvMsg.cursorIndex = 0;
	}
	this.recvSubChannel = data.subChannel == null ? -1 : data.subChannel;
	this.loopName = data.loopNameStr || "";
	this.error = data.errorStr || "";
	this.denyReason = data.denyReason || "";
	this.channelListing = data.channelListing || null;
	this.deniedClientName = data.deniedClientName || ""; // Note: no current way to read the denied name in ext version 104
	this.deniedChannelName = data.deniedChannelName || "";
	if (data.deniedChannelName != null && this.channel != null) {
		throw "Invalid use of deniedChannelName with channel.";
	}
}
/** @constructor */
function Bluewing_ChannelListing(client, peerCount, name) {
	/// <summary> Creates a channel listing entry for Bluewing. </summary>
	/// <param name="peerCount" type="Number" integer="true"> The number of peers
	///		on this channel when the list was sent. </param>
	/// <param name="name"> The name of this channel. </param>
	this.peerCount = peerCount;
	this.name = name;
	this.nameSimplified = client.SimplifyName(name);
}
/// <var> Constructor for a Bluewing Channel. </var>
/** @constructor */
function Bluewing_Channel(client, id, name) {
	/// <summary> Constructor for a Bluewing Channel. No peers in list. </summary>
	/// <param name="client" type="Bluewing_Client"> The client containing the channel. </param>
	/// <param name="id" type="Number" integer="true"> The ID of the channel. Uint16. </param>
	/// <param name="name" type="String"> The name of the channel. Text. </param>

	/// <field name="client" type="Bluewing_Client"> The client containing the channel. </field>
	this.client = client;
	/// <field name="id" type="Number" integer="true"> The ID of the channel, UInt16. </field>
	this.id = id;
	/// <field name="name" type="String"> The name of the channel. </field>
	this.name = name;
	this.nameSimplified = client != null ? client.SimplifyName(name) : name;
	/// <field name="peerList" type="Array" elementType="Bluewing_Peer">
	///		List of all peers on this channel. </field>
	this.peerList = [];
	/// <field name="isClosed" type="Boolean"> If true, this channel is 'read-only' and
	///		messages cannot be sent to it. If false, it is free to use for all purposes.
	///		When set to true, all peers should be set to closed as well. </channel>
	this.isClosed = false;

	this.LeaveChannel = function () {
		/// <summary> Leaves the channel. </summary>
		if (this.isClosed) {
			return; // No errors.
		}

		// We'll close the channel on channel leave message
		const view = new DataView(new ArrayBuffer(4));
		view.setUint8(0, 0);
		view.setUint8(1, 3);
		view.setUint16(2, this.id, true);
		this.client.$SendRawTCPServerCmd(view.buffer);
	};

	this.BlastMsg = function (subChannel, msg, variant) {
		if (subChannel > 255 || subChannel < 0) {
			throw "Cannot send/blast message with subchannel less than 0 or greater than 255.";
		}
		const view = new DataView(new ArrayBuffer(4 + msg.byteLength));
		view.setUint8(0, 0x20 | variant);
		view.setUint8(1, subChannel);
		view.setUint16(2, this.id, true);
		new Uint8Array(view.buffer).set(new Uint8Array(msg), 4);
		this.client.$SendRawUDP(view.buffer);
	};
	this.SendMsg = function (subChannel, msg, variant) {
		if (subChannel > 255 || subChannel < 0) {
			throw "Cannot send/blast message with subchannel less than 0 or greater than 255.";
		}
		const view = new DataView(new ArrayBuffer(4 + msg.byteLength));
		view.setUint8(0, 0x20 | variant);
		view.setUint8(1, subChannel);
		view.setUint16(2, this.id, true);
		new Uint8Array(view.buffer).set(new Uint8Array(msg), 4);
		this.client.$SendRawTCP(view.buffer);
	};
	this.getPeer = function (peerID) {
		return this.peerList.find(function (f) { return f.id == peerID; });
	};
}
/** @constructor */
function Bluewing_Peer(channel, peerID, name) {
	/// <summary> Creates a Bluewing Peer on the given channel. </summary>
	/// <param name="channel" type="Bluewing_Channel"> The channel the peer can be communciated with on. </param>
	/// <param name="peerID" type="Number" integer="true"> The ID of the peer. Integer. </param>
	/// <param name="name" type="String"> The name of the peer. Text. </param>

	/// <field name="channel" type="Bluewing_Channel"> The channel this peer is on. A peer
	///		cannot be accessed without a channel as a go-between.  </field>
	this.channel = channel;
	/// <field name="peerID" type="Number" integer="true"> The ID number of the peer. UShort. </field>
	this.id = peerID;
	/// <field name="name" type="String"> The peer's current name. </field>
	this.name = name;
	/// <field name="name" type="String"> The peer's current name. </field>
	this.nameSimplified = this.channel == null ? name : this.channel.client.SimplifyName(name);
	/// <field name="previousName" type="String"> The last name or blank. </field>
	this.previousName = "";
	/// <field name="isClosed" type="Boolean"> If true, this peer is 'read-only' and messages cannot be sent to
	///		it. If false, it is free to use for all purposes. </field>
	this.isClosed = false;

	this.SendMsg = function (subChannel, userMsg, variant) {
		/// <summary> Sends a user-defined message via TCP to the peer. </summary>
		/// <param name="subChannel" type="Number"> The subchannel, 0-255. Uint8. </param>
		/// <param name="userMsg" type="ArrayBuffer"> The user message. </param>
		/// <param name="variant" type="Number"> The variant. 0-2 only, integer. </param>
		subChannel = ~~subChannel;

		if (subChannel == null || userMsg == null || variant == null ||
			subChannel > 255 || subChannel < 0 || variant > 2 || variant < 0 ||
			(variant == 1 && userMsg.byteLength != 4)) {
			// Serious error with client implementation, so die horrifically.
			alert("Argument out of range in Bluewing_Peer.SendMsg().");
			throw "Argument out of range in Bluewing_Peer.SendMsg().";
		}

		const view = new DataView(new ArrayBuffer(6 + userMsg.byteLength));
		view.setUint8(0, 0x30 | variant);
		view.setUint8(1, subChannel);
		view.setUint16(2, this.channel.id, true);
		view.setUint16(4, this.id, true);
		new Uint8Array(view.buffer).set(new Uint8Array(userMsg), 6);
		this.channel.client.$SendRawTCP(view.buffer);
	};
	this.BlastMsg = function (subChannel, userMsg, variant) {
		/// <summary> Blasts a user-defined message via UDP to the peer. </summary>
		/// <param name="subChannel" type="Number"> The subchannel, 0-255. Uint8. </param>
		/// <param name="userMsg" type="ArrayBuffer"> The user message. </param>
		/// <param name="variant" type="Number"> The variant. 0-2 only, integer. </param>
		subChannel = ~~subChannel;

		if (subChannel == null || userMsg == null || variant == null ||
			subChannel > 255 || subChannel < 0 || variant > 2 || variant < 0 ||
			(variant == 1 && userMsg.byteLength != 4)) {
			// Serious error with client implementation, so die horrifically.
			alert("Argument out of range in Bluewing_Peer.BlastMsg().");
			throw "Argument out of range in Bluewing_Peer.BlastMsg().";
		}

		const view = new DataView(new ArrayBuffer(6 + userMsg.byteLength));
		view.setUint8(0, 0x30 | variant);
		view.setUint8(1, subChannel);
		view.setUint16(2, this.channel.id, true);
		view.setUint16(4, this.id, true);
		new Uint8Array(view.buffer).set(new Uint8Array(userMsg), 6);
		this.channel.client.$SendRawUDP(view.buffer);
	};
}
/** @constructor */
function Bluewing_Client(blue) {
	/// <summary> Bluewing client creation. </summary>
	/// <param name="blue" type="CRunBluewing_Client"> The extension. </param>

	darkEdif.finalizer.register(this, "Bluewing_Client itself!");
	// Statuses of the socket
	this.isConnected = false; // connected on websocket
	this.isConnectApproved = false; // connect OK approved by Lacewing
	this.isConnecting = false;
	this.isClosed = true;
	// In case server is slow to close, this indicates the Bluewing_Client is dead and should be GC'd ASAP.
	this.isDead = false;
	/// <field name="ext" type="CRunBluewing_Client"> The outer Fusion extension. </field>
	this.ext = blue;

	/// <field name="encoder" type="TextEncoder"> Deals with encoding text
	///		from String to Uint8Array. </field>
	this.encoder = new TextEncoder(/* utf-8 assumed */);
	/// <field name="decoder" type="TextEncoder"> Deals with decoding text
	///		from Uint8Array to String. </field>
	this.decoder = new TextDecoder('utf-8');
	/// <field name="channelList" type="Array" elementType="Bluewing_Channel"> List of all the
	///		Bluewing_Channel Objects that this client is accessing. </field>
	this.channelList = [];
	/// <field name="channelList" type="Array" elementType="Bluewing_ChannelListing">
	///		The last Bluewing_ChannelListing list sent by the server. </field>
	this.channelListing = [];
	/// <field name="me" type="Bluewing_Peer"> The details of this client. Unset names are blank. </field>
	this.me = new Bluewing_Peer(null, -1, "");
	this.welcomeMessage = "";

	/// <field name="willReconnect" type="Boolean"> If true, disconnections will be ignored;
	///		only failed_reconnect events will cause Fusion's On Disconnect events. </field>
	this.willReconnect = true;

	/// <field name="ignoreDisconnectErrorHack" type="Boolean"> Workaround. If true, when disconnected unexpectedly, will not create error. </field>
	this.ignoreDisconnectErrorHack = false;

	/// <field name="comm" type="WebSocket"> The communication socket. </field>
	this.comm = null;

	/// <field name="connectAddressGiven" type="String"> The address last passed to Connect(). </field>
	this.connectAddressGiven = "";

	/// <field name="udpConnected" type="Boolean"> True if UDPWelcome was received. </field>
	this.udpConnected = false;

	// <field name="pong" type="ArrayBuffer"> Response message to a Ping. </field>
	this.pong = new Uint8Array([9 << 4]);

	// <field name="impl" type="ArrayBuffer"> Response message to a Implementation Request. </field>
	const implStr = this.encoder.encode("Bluewing UWP JS b" + this.ext['ExtensionVersion']);
	this.impl = new ArrayBuffer(implStr.length + 1);
	const impl2 = new Uint8Array(this.impl);
	impl2.set([10 << 4], 0);
	impl2.set(implStr, 1);

	// Maximum size of a UDP datagram is 65535, minus IP(v4/v6) header, minus Lacewing Relay UDP header,
	// with some extra margin, due to WebSocket psuedo.
	// It's probably fragmented beyond 1400 bytes anyway, due to Ethernet MTU.
	// UDP exceeding this size will be silently dropped.
	this.relay_max_udp_payload = 0xFFFF - 40 - 20;

	this.OnDisconnect = function () {
		/// <summary> Called when the server disconnects or fails to connect. </summary>

		// this.willReconnect can be assumed false.

		// If isConnectApproved, we've exchanged a Lacewing handshake: so we use 'on disconnect'.
		// Otherwise we use 'on connection denied'.
		if (this.ext != null) {
			// connection approved, so disconnect
			if (this.isConnectApproved) {
				this.ext.LacewingCall_OnDisconnect(this);
			}
			// Edge case where we get WebSocket OK, but no Lacewing response
			else if (this.isConnected) {
				this.ext.LacewingCall_OnConnectDenied(this, "Got a WebSocket connection, but couldn't use Bluewing.");
			}
			//else we never connected anyway; but a connection failed error should've been made in handleEvent() already
			//	this.ext.LacewingCall_OnError(this, "Failed to connect to WebSocket server.");
		}

		this.isConnected = this.isConnectApproved = this.isConnecting = false;
		this.isClosed = true;
		this.me.name = "";
		this.me.previousName = "";
		this.me.id = -1;
		this.channelList = [];
		this.channelListing = [];
		this.serverAddr = "";
		this.serverPort = -1;
		this.welcomeMessage = "";
	};
	this.handleEvent = function (evt) {
		 if (evt.type == 'message') {
			this.onMessage(false, evt.data);
		 }
		// Socket open - just connected on WebSocket level, it's raw socket, or possibly TLS-encrypted raw
		else if (evt.type == 'open') {
			this.isConnected = true;
			this.isConnecting = false;
			this.willReconnect = true;

			const protoRev = this.encoder.encode("revision 3");
			const u8arr = new Uint8Array(2 + protoRev.length);
			u8arr.set([0, 0], 0); // Request, Connect
			u8arr.set(protoRev, 2); // Protocol version
			this.$SendRawTCP(u8arr.buffer);
		}
		// WebSocket closed. Normally handled by close packet exchanged both ways; but could be from protocol failure or timeout (evt.wasClean == false).
		else if (evt.type == 'close') {
			// Unfortunately according to WebSocket spec, webpages can't get proper socket error details.
			// This is to prevent malicious JS probing the local network and detecting from timeout error vs. protocol error,
			// whether a device with that IP exists.
			// However, the console can show more information.
			if ((!evt.wasClean && evt.code != 1000) && !this.isDead && !this.ignoreDisconnectErrorHack) {
				this.ext.LacewingCall_OnError(this, "Connection failed with error code " + evt.code + ": " + (evt.reason || "generic socket failure"));
			}
			// This hack is described on another line.
			if (this.ignoreDisconnectErrorHack) {
				this.ignoreDisconnectErrorHack = false;
				darkEdif.consoleLog(this.ext, "Disconnect no-error hack was active; possible error ignored.");
			}
			darkEdif.consoleLog(this.ext, "Connection killed with code " + evt.code + ". Clean = " + evt.wasClean);

			this.OnDisconnect();
		}
		else if (evt.type == 'error') {
			// Console may have more info, but otherwise try WireShark
			if (!this.isDead) {
				this.ext.LacewingCall_OnError(this, evt.reason || "WebSocket error, browser dev console may have more detail.");
			}
		}
		else { // unknown event handler
			throw "a wobbly";
		}
	};
	this.CreateError = function (err) {
		/// <summary> Generates an error event with the given text. </summary>
		/// <param name="err" type="String"> Error text. String only, cannot be null. </param>
		if (err == null) {
			// Can't get worse than erroring while making an error.
			alert("this.CreateError() called with a null/undefined parameter.");
			throw "this.CreateError() called with a null/undefined parameter.";
		}
		if (this.ext != null) {
			this.ext.LacewingCall_OnError(this, err);
		}
	};


	this.SimplifyName = function (name) {
		// This is meant to imitate the Windows version, but since we lack the UTF8PROC library in JS,
		// we're skipping all the category checks and the big lookup tables UTF8PROC has.
		// So it's only going to be similar to Windows, not an exact match.

		// Lazy man's stripping of accents and marks - first expand accented letters to letters + accents (NFKD), then remove accents
		let nameSimplified = name.normalize("NFKD").replace(/[\u0300-\u036f]/g, "");
		// Lowercase it
		nameSimplified = nameSimplified.toLowerCase();
		// Back to smallest form (NFC)
		nameSimplified = nameSimplified.normalize();

		// All newlines to one type (NS)
		nameSimplified = nameSimplified.replace(/[\r\n\u0085\u2029\t\v\f]/g, "\u2028");
		// Remove control characters
		nameSimplified = nameSimplified.replace(/[\u0000-\u001F\u007F-\u009F]/g, "");

		// Bluewing additional:
		// Don't allow '0' to be confused with 'O', could happen with some fonts
		nameSimplified = nameSimplified.replace("0", "o");
		// Pipe '|', 1, and uppercase I (converted to 'i' by toLowerCase())
		nameSimplified = nameSimplified.replace(/[\|i1]/g, "l");
		// |\| to n
		nameSimplified = nameSimplified.replace("l\\l", "n");
		// \/ to V (but due to lowercase, v)
		nameSimplified = nameSimplified.replace("\\/", "v");
		// |\/| to m (due to earlier, |v| to m)
		nameSimplified = nameSimplified.replace("lvl", "m");
		// 5 and S are similar
		nameSimplified = nameSimplified.replace("5", "s");
		// ( with C
		nameSimplified = nameSimplified.replace("(", "c");
		// horizontal ellipsis (U+2026) to "..."
		nameSimplified = nameSimplified.replace("\u2026", "...");
		// vv to w, just in case
		nameSimplified = nameSimplified.replace("vv", "w");
		return nameSimplified;
	};
	this.onMessage = function (isUDP, rawMsg) {
		/// <summary> Message handler. Called internally. </summary>
		/// <param name="isUDP"> True if UDP, false if TCP.
		///		Protocol is indicated by the room aka namespace. </param>
		/// <param name="rawMsg" type="ArrayBuffer"> The full message, including all headers. </param>
		const fullMsg = new DataView(rawMsg);

		if (fullMsg.byteLength < 1) {
			this.CreateError("Received a useless zero-length message. Server may be broken.");
			return;
		}

		const firstByte = fullMsg.getUint8(0);
		const type = firstByte >> 4;
		const variant = firstByte & 7;

		// Psuedo-UDP
		isUDP = (firstByte & 8) != 0;

		if (type >= 5 && type <= 8) {
			this.CreateError("Received a message with type " + type + ", which are Object messages.\n" +
				"Lacewing Relay does not support Object messages, although the protocol specifies it.");
			return;
		}
		if ((type < 1 || type > 3) && variant > 2) {
			this.CreateError("Received a message with type " + type + " and variant non-zero");
			return;
		}
		if ((type >= 1 && type <= 3) && variant == 1 && fullMsg.byteLength != 2 + 4 + (type == 1 ? 0 : 4)) {
			this.CreateError("Received a 'number' variant message, but the message size was not as expected");
			return;
		}

		switch (type) {
			// User response.
			case 0: {
				if (fullMsg.byteLength < 2) {
					this.CreateError("Received a message of type Response, but the size " +
						"of response was unexpected. Expected 2+, got " + fullMsg.byteLength);
					return;
				}
				const responseType = fullMsg.getUint8(1);
				const success = fullMsg.getUint8(2) != 0;
				switch (responseType) {
					// Connect response
					case 0: {
						if (success) {
							this.me.id = fullMsg.getUint16(3, true);

							if (fullMsg.byteLength == 5) {
								this.welcomeMessage = "";
							}
							else {
								this.welcomeMessage = this.decoder.decode(rawMsg.slice(5));
							}

							// Might as well send implementation without the server asking; impl was added to Blue Server
							// before WebSocket was
							this.comm.send(this.impl);
							this.isConnectApproved = true;

							// There is no UDP, unless we're going to drag in WebRTC, or make a fake UDP like Socket.IO does.
							// The fake UDP basically makes a no-queue message system, where if something is being queued already,
							// it discards it. WebRTC does the same, giving a max time to pend before giving up, and max queue size.
							this.ext.LacewingCall_OnConnect(this);
						}
						else {
							const denyReason = this.decoder.decode(rawMsg.slice(3));
							this.ext.LacewingCall_OnConnectDenied(this, denyReason);

							this.ignoreDisconnectErrorHack = true;
							this.comm.close();
						}
						return;
					}
					// SetName response
					case 1: {
						const nameLength = fullMsg.getUint8(3);
						if (!success) {
							const name = this.decoder.decode(rawMsg.slice(4, 4 + nameLength));
							const denyReason2 = this.decoder.decode(rawMsg.slice(4 + nameLength));

							this.ext.LacewingCall_OnNameDenied(this, name, denyReason2);
						}
						else {
							const name2 = this.decoder.decode(rawMsg.slice(4, 4 + nameLength));

							if (this.me.name === "") {
								this.me.name = name2;
								this.ext.LacewingCall_OnNameSet(this);
							}
							else {
								this.me.previousName = this.me.name;
								this.me.name = name2;

								this.ext.LacewingCall_OnNameChanged(this, this.me.previousName);
							}
						}
						return;
					}
					// JoinChannel response
					case 2: {
						if (success) {
							let flags = fullMsg.getUint8(3);
							const nameLength3 = fullMsg.getUint8(4);
							const name3 = this.decoder.decode(rawMsg.slice(5, 5 + nameLength3));
							const channelID = fullMsg.getUint16(5 + nameLength3, true);

							if (this.getChannel(channelID) != null) {
								this.CreateError("Received a \"channel join success\" message for channel \"" +
									name3 + "\", when the channel had already been joined.");
								return;
							}
							if ((flags & 0xFE) != 0) {
								this.CreateError("Received a \"channel join success\" message with an unrecognised " +
									"flags variable. Expected 0 or 1, got " + flags + ".");
								// Not a serious error, just keep swimmin'
							}

							const channel = new Bluewing_Channel(this, channelID, name3);
							darkEdif.finalizer.register(channel, "Channel with name " + channel.name);

							if ((flags & 0x1) == 0x1) {
								channel.master = this.me;
							}

							// No peers
							if (fullMsg.byteLength != 7 + nameLength3) {
								let cursor = 7 + nameLength3, curPeer = null;

								while (cursor < fullMsg.byteLength) {
									curPeer = new Bluewing_Peer(channel,
										fullMsg.getUint16(cursor, true),
										this.decoder.decode(rawMsg.slice(cursor + 4, cursor + 4 + fullMsg.getUint8(cursor + 3))));
									darkEdif.finalizer.register(curPeer, "pre-existing Peer with name " + curPeer.name);
									channel.peerList.push(curPeer);

									// Flags indicate channel master or not.
									flags = fullMsg.getUint8(cursor + 2);

									// Dodgy flags? Keep on parsin'
									if ((flags & 0xFE) != 0) {
										this.CreateError("Unrecognised peer flags. Expected 0 or 1, got " + flags);
									}

									if ((flags & 1) == 1) {
										// Master already set, fuss
										if (channel.master != null) {
											this.CreateError("Multiple channel masters are not supported, but were sent by the server");
										}
										else {
											channel.master = curPeer;
										}
									}
									cursor += 4 + fullMsg.getUint8(cursor + 3);
								}
							}

							if (channel == null || channel.id == null || channel.name == null) {
								this.CreateError("Malformed Join Channel Success message received");
								return;
							}
							this.channelList.push(channel);

							this.ext.LacewingCall_OnJoinChannel(this, channel);
						}
						else {
							const nameLength4 = fullMsg.getUint8(3);
							const name4 = this.decoder.decode(rawMsg.slice(4, 4 + nameLength4)),
								denyReason3 = this.decoder.decode(rawMsg.slice(4 + nameLength4));

							if (name4 == null || denyReason3 == null) {
								this.CreateError("Malformed Join Channel Denied message received");
								return;
							}
							this.ext.LacewingCall_OnJoinChannelDenied(this, name4, denyReason3);
						}
						return;
					}
					// LeaveChannel response
					case 3: {
						const channelID2 = fullMsg.getUint16(3, true), channel2 = this.getChannel(channelID2);
						if (channel2 == null) {
							this.CreateError("Malformed Leave Channel " + (success ? "" : "Denied ") + "message; channel ID " +
								channelID2 + " has already been left, or was never joined in the first place");
							return;
						}

						if (success) {
							this.ext.LacewingCall_OnLeaveChannel(this, channel2);
							this.channelList = this.channelList.filter(function (c) { return c.id != channel2.id; });
						}
						else {
							const denyReason4 = this.decoder.decode(rawMsg.slice(5));
							this.ext.LacewingCall_OnLeaveChannelDenied(this, channel2, denyReason4);
						}
						return;
					}
					// ChannelList response
					case 4: {
						if (success) {
							this.channelListing = [];
							let cursor2 = 3, curChLst = null;
							while (cursor2 < fullMsg.byteLength) {
								curChLst = new Bluewing_ChannelListing(this, fullMsg.getUint16(cursor2, true),
									this.decoder.decode(rawMsg.slice(cursor2 + 3, cursor2 + 3 + fullMsg.getUint8(cursor2 + 2))));
								cursor2 += 3 + fullMsg.getUint8(cursor2 + 2);

								if (curChLst.name == null) {
									this.CreateError("Channel Listing Response message was malformed");
									return;
								}
								for (let i = 0; i < this.channelListing.length; ++i) {
									if (this.channelListing[i].nameSimplified == curChLst.nameSimplified) {
										this.CreateError("Channel Listing response contained duplicates");
										return;
									}
								}
								darkEdif.finalizer.register(curChLst, "ChannelListing with name " + curChLst.name);
								this.channelListing.push(curChLst);
							}

							this.ext.LacewingCall_OnChannelListReceived(this);
						}
						else {
							// No native Channel List Request Denied.
							let denyReason5 = "Server has disabled or does not support channel listing requests.";
							if (fullMsg.byteLength > 3) {
								denyReason5 = this.decoder.decode(rawMsg.slice(3));
							}
							this.CreateError("Channel list request denied. Reason \"" + denyReason5 + "\"");
						}
						return;
					}
					default: {
						this.CreateError("Received a message of type Response, but the type " +
							"of response was unexpected. Expected 0-4, got " + responseType);
						return;
					}
				}
				// doesn't reach here
			}
			// BinaryServerMessage
			case 1: {
				const subChannel = fullMsg.getUint8(1), msg = rawMsg.slice(2);
				this.ext.LacewingCall_OnServerMessage(this, isUDP, subChannel, msg, msg.byteLength, variant);
				return;
			}
			// BinaryChannelMessage, peer message
			case 2:
			case 3: {
				const subChannel2 = fullMsg.getUint8(1);
				const channelID3 = fullMsg.getUint16(2, true);
				const channel3 = this.getChannel(channelID3);
				if (channel3 == null) {
					this.CreateError("A " +(type == 2 ? "channel" : "peer") + " message on channel ID " + channelID3 +
						" was received, but client is not joined to it. Message ignored.");
					return;
				}

				const peerID = fullMsg.getUint16(4, true);
				const peer = channel3.getPeer(peerID);
				if (peer == null) {
					this.CreateError("A "+(type == 2 ? "channel" : "peer") + " message on channel \"" + channel3.name +
						"\" was received, but sending peer ID " + peerID + " is not joined to it.");
					return;
				}

				const msg3 = rawMsg.slice(6);
				if (type == 2) {
					this.ext.LacewingCall_OnChannelMessage(this, peer, channel3, isUDP, subChannel2, msg3, msg3.byteLength, variant);
				}
				else {
					this.ext.LacewingCall_OnPeerMessage(this, peer, channel3, isUDP, subChannel2, msg3, msg3.byteLength, variant);
				}
				return;
			}
			// Message from server to channel
			case 4: {
				const subChannel3 = fullMsg.getUint8(1);
				const channelID4 = fullMsg.getUint16(2, true);
				const channel4 = this.getChannel(channelID4);
				if (channel4 == null) {
					this.CreateError("A server -> channel message for channel ID " + channelID4 +
						" was received, but client is not joined to that channel. Message ignored.");
					return;
				}

				const msg4 = rawMsg.slice(4);
				this.ext.LacewingCall_OnServerChannelMessage(this, channel4, isUDP, subChannel3, msg4, msg4.byteLength, variant);
				return;
			}
			// Peer change
			case 9: {
				const channelID5 = fullMsg.getUint16(1, true);
				const peerID2 = fullMsg.getUint16(3, true);

				const channel5 = this.getChannel(channelID5);
				if (channel5 == null) {
					this.CreateError("A 'peer change' message for a peer on channel ID " + channelID5 +
						" was received, but client is not joined to that channel. Message ignored.");
					return;
				}

				let peer2 = channel5.getPeer(peerID2);
				const peerName = rawMsg.byteLength == 5 ? "" : this.decoder.decode(rawMsg.slice(6));
				const flags2 = rawMsg.byteLength == 5 ? 0 : fullMsg.getUint8(5);

				if ((flags2 & 0xFE) != 0) {
					this.CreateError("Unexpected flags in 'peer2 change message'; expected 0 or 1, got " + flags2);
					return;
				}
				// peer2 not in channel list: assume the message is 'peer2 connected to channel'
				if (peer2 == null) {
					peer2 = new Bluewing_Peer(channel5, peerID2, peerName);
					darkEdif.finalizer.register(peer2, "Peer with name " + peer2.name);

					channel5.peerList.push(peer2);
					if ((flags2 & 0x1) == 0x1) {
						// There's no event to inform the user that the channel master changed.
						//if (channel.master != null)

						channel5.master = peer2;
					}

					this.ext.LacewingCall_OnPeerConnect(this, channel5, peer2);
					return;
				}
				// else peer2 in list

				// peer2 disconnect (peerName and flags missing from message)
				if (rawMsg.byteLength == 5) {
					peer2.isClosed = true;
					this.ext.LacewingCall_OnPeerDisconnect(this, channel5, peer2);

					channel5.peerList = channel5.peerList.filter(function (f) { return f.id != peerID2; });
					if (channel5.master == peer2) {
						channel5.master = null;
					}
					return;
				}

				// Otherwise, peer2 name change.
				if (peer2.name != peerName) {
					peer2.previousName = peer2.name;
					peer2.name = peerName;
					peer2.nameSimplified = this.SimplifyName(peerName);
					this.ext.LacewingCall_OnPeerNameChanged(this, channel5, peer2);
					return;
				}

				// Otherwise, it has to be a master setting change...
				// but there's no events for that, nor server-side code,
				// bar in a custom server.
				// Since custom server deserves a custom client, of which this implementation is NOT,
				// Bluewing will thus error out, although it will still store the new master.

				// Master reset, currently no master.
				// Note it's the server's job to disconnect all peers if the
				// 'close when master leaves' flag was enabled for that channel.

				if ((flags2 & 0x1) == 0x1) {
					if (channel5.master == peer2) {
						this.CreateError("A 'peer change' message was sent, but no difference was noted. Message ignored");
						return;
					}
					// Already a master on this channel, and it's not this peer2
					channel5.master = peer2;
					if (channel5.master != null) {
						this.CreateError("A 'peer change' message was sent, which changed the master, but " +
							"there was already a master... switching, but there's no way to inform Fusion of that");
					}
					else {
						this.CreateError("A 'peer change' message added a peer as channel master, but there's no " +
							"events to inform Fusion of it. Switching");
					}
					return;
				}
				// Message peer2 is being removed from master
				else if (channel5.master == peer2) {
					channel5.master = null;
					this.CreateError("A 'peer change' message was sent, which removed the master from being master " +
						"... switching, but there's no way to inform Fusion of that.");
					return;
				}
				// Message peer2 is being removed from master status, but isn't master anyway
				else {
					this.CreateError("A 'peer change' message was sent, which removed the master, but " +
						"the peer that was being removed from master was already not a master... ignoring message.");
					return;
				}
			}
			// UDPWelcome
			case 10: {
				// The C++ protocol implements a wait for both TCP and UDP to connect before triggering On Connect.
				// We don't implement UDP connect yet, so this code shouldn't run.
				this.CreateError("Received UDPWelcome response to UDPHello, but not expecting UDP of any sort");
				return;
			}
			// Ping
			case 11: {
				// respond with Pong
				this.$SendRawTCPServerCmd(this.pong);
				return;
			}
			// Implementation response
			case 12: {
				this.$SendRawTCPServerCmd(this.impl);
				return;
			}
			// Unrecognised type.
			default: {
				this.CreateError("Received an unknown message type. Expected 0-12, got " + type);
			}
		}
	};
	this.Connect = function	(addr) {
		/// <summary> Connects to supplied addresss, which must be defined non-blank string. Called internally. </summary>
		/// <param name="isUDP"> True if UDP, false if TCP.
		///		Protocol is indicated by the room aka namespace. </param>
		/// <param name="rawMsg" type="ArrayBuffer"> The full message, including all headers. </param>

		if (!this.isClosed) {
			// Socket is NOT closed, or closing, which means it's open or connecting.
			// Since the user is attempting to connect, this is Not Allowed.
			this.CreateError("Couldn't start connection: Already " + (this.isConnecting ? "connecting" : "connected") + " to a server");
			return;
		}
		let socketSecure = "ws://";
		if (/wss?:\/\//i.test(addr)) {
			socketSecure = /wss?:\/\//i.exec(addr)[0].toLowerCase();
			addr = addr.substr(socketSecure.length);
			if (addr.length > 0 && addr[addr.length - 1] == '/') {
				addr = addr.substr(0, addr.length - 1);
			}
		}

		// TODO: IPv6 addresses?
		if (!/[a-zA-Z0-9-\.´]{4,}(?:\:[0-9]{1,5})?/.test(addr)) {
			this.CreateError("Connection address " + addr + " is poorly formatted");
			return;
		}

		const self = this;
		this.isClosed = false;
		this.isConnecting = true;
		this.connectAddressGiven = addr;

		// file and http: connect insecurely, for speed
		// https: connect securely
		// Note that browsers will reject a ws:// connection if the page is https://

		if (addr.indexOf(':') != -1) {
			const both = addr.split(':');
			this.serverAddr = both[0];
			this.serverPort = parseInt(both[1], 10);
			if (this.serverPort < 0 || this.serverPort > 65535) {
				this.CreateError("Invalid port given");
				this.isConnecting = false;
				this.isClosed = true;
				return;
			}
		}
		else { // No explicit port, guess from this page's protocol
			this.serverAddr = addr;
			this.serverPort = location.protocol === 'https:' ? 443 : 80;
		}

		if (this.serverPort === 443) {
			socketSecure = "wss://";
		}

		try {
			this.comm = new WebSocket(socketSecure + addr + "/", "bluewing");
			this.comm.binaryType = "arraybuffer";
			this.comm.addEventListener('open', this);
			this.comm.addEventListener('error', this);
			this.comm.addEventListener('close', this);
			this.comm.addEventListener('message', this);
		}
		catch (e) {
			do {
				// SecurityError are thrown when HTTPS page tries loading non-secure (ws://) connection
				// Retry on wss://, same port, and see how it goes
				if (e.type == "SecurityError" && socketSecure == "ws://") {
					this.CreateError("Cannot connect to insecure WebSocket port on a HTTPS webpage. Check help file. Will retry on secure.");
					try {
						this.comm = new WebSocket("wss://" + addr + "/", "bluewing");
						this.comm.binaryType = "arraybuffer";
						this.comm.addEventListener('open', this);
						this.comm.addEventListener('error', this);
						this.comm.addEventListener('close', this);
						this.comm.addEventListener('message', this);
						break;
					}
					catch (e2) {
						// fall through; original URL caused e, so e's error is more applicable to report over e2
					}
				}
				this.CreateError("Couldn't create connection: exception of type " + e.type + " occurred.");

				this.isConnecting = false;
				this.isClosed = true;
				return;
			// can't connect
			} while(false);
		}
	};
	this.Disconnect = function () {
		// If dead, the extension that held this Client has "deleted" it, but due to presence of the WebSocket it's still in RAM.
		// We'll clear out the handlers so garbage collector can get it faster
		if (this.isDead) {
			this.comm.removeEventListener('open', this);
			this.comm.removeEventListener('error', this);
			this.comm.removeEventListener('close', this);
			this.comm.removeEventListener('message', this);
		}

		// TODO: Disconnect generates an error for Unix servers serving WSS://, as of server build 32.
		// This is due to the server not sending the closing handshake properly before the connection is closed.
		// I've added this workaround to swallow the disconnect error.
		this.ignoreDisconnectErrorHack = true;

		this.comm.close();
		if (this.isDead) {
			this.comm = null;
			this.OnDisconnect();
		}
		// else wait for socket close event to call OnDisconnect
	};
	this.SetName = function (newName) {
		/// <summary> Sets name. Called internally. </summary>
		/// <param name="newName" type="String"> The new name to set to.
		///		Must not be longer than 254 chars. </param>
		if (!this.isConnectApproved) {
			this.CreateError("Not connected, cannot set name");
			return;
		}
		if (newName.length > 254) {
			this.CreateError("Name cannot be longer than 254 characters");
			return;
		}

		const realName = this.encoder.encode(newName);
		// Length maximum is not specified by the SetName request format,
		// but it's specified by any name-set response, so we should infer anyway.
		if (realName.length > 254) {
			this.CreateError("Name has been encoded to UTF-8, and has gotten larger than 254 characters");
			return;
		}
		const msg = new Uint8Array(2 + realName.byteLength);
		msg.set([0, 1], 0);
		msg.set(realName, 2);
		this.$SendRawTCPServerCmd(msg);
	};
	this.getChannel = function (channelID) {
		return this.channelList.find(function (f) { return f.id == channelID; });
	};
	this.SendMsg = function (subChannel, userMsg, variant) {
		/// <summary> Sends a user-defined message via TCP to the server. </summary>
		/// <param name="subChannel" type="Number"> The subchannel, 0-255. Uint8. </param>
		/// <param name="userMsg" type="ArrayBuffer"> The user message. </param>
		/// <param name="variant" type="Number"> The variant. 0-2 only, integer. </param>
		subChannel = ~~subChannel;

		if (subChannel == null || userMsg == null || variant == null ||
			subChannel > 255 || subChannel < 0 || variant > 2 || variant < 0 ||
			(variant == 1 && userMsg.byteLength != 4)) {
			// Serious error with client implementation, so die horrifically.
			throw "Argument out of range in Bluewing_Client.SendMsg().";
		}

		const msg = new Uint8Array(2 + userMsg.byteLength);
		msg.set([ 0x10 | variant, subChannel], 0);
		msg.set(new Uint8Array(userMsg), 2);
		this.$SendRawTCP(msg);
	};
	this.BlastMsg = function (subChannel, userMsg, variant) {
		/// <summary> Sends a user-defined message via UDP to the server. </summary>
		/// <param name="subChannel" type="Number"> The subchannel, 0-255. Uint8. </param>
		/// <param name="userMsg" type="ArrayBuffer"> The user message. </param>
		/// <param name="variant" type="Number"> The variant. 0-2 only, integer. </param>
		subChannel = ~~subChannel;
		variant = ~~variant;

		if (subChannel == null || userMsg == null || variant == null ||
			subChannel > 255 || subChannel < 0 || variant > 2 || variant < 0 ||
			(variant == 1 && userMsg.byteLength != 4)) {
			// Serious error with client implementation, so die horrifically.
			alert("Argument out of range in Bluewing_Client.BlastServer().");
			throw "Argument out of range in Bluewing_Client.BlastServer().";
		}

		const msg = new Uint8Array(2 + userMsg.byteLength);
		msg.set([ 0x10 | variant, subChannel], 0);
		msg.set(new Uint8Array(userMsg), 2);
		this.$SendRawUDP(msg);
	};
	this.ListChannels = function () {
		const msg = new Uint8Array([0, 4]);
		this.$SendRawTCPServerCmd(msg);
	};
	this.JoinChannel = function (channelName, hidden, autoclose) {
		if (hidden.constructor.name !== 'Boolean' || autoclose.constructor.name !== 'Boolean') {
			throw "invalid join flags";
		}
		const realName = this.encoder.encode(channelName);
		const msg = new Uint8Array(3 + realName.byteLength);
		msg.set([0, 2, hidden | (autoclose << 1)], 0);
		msg.set(realName, 3);
		this.$SendRawTCPServerCmd(msg);
	};

	// Any exceptions will be from socket not being connected, in which case we'll get a close or error event anyway,
	// so no need to create more errors
	this.$SendRawTCPServerCmd = function (msg) {
		try { this.comm.send(msg); }
		catch (e) { }
	};
	this.$SendRawTCP = function (msg) {
		try { this.comm.send(msg); }
		catch (e) { }
	};
	this.$SendRawUDP = function (msg) {
		if (msg.byteLength > this.relay_max_udp_payload) {
			console.debug("UDP message too large, discarded");
			return;
		}

		const temp = new Uint8Array(msg, 0, 1);
		temp.set([temp[0] | 0x8], 0);
		try { this.comm.send(msg); }
		catch (e) { }
	};
}
