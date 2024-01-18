// Note: this file was changed by Bluewing Client, using UWP exporter build 286.6.0 file as template.
// Data\Runtime\Wua\version.txt was 286.6.0, but the UWP runtime also has
//    window["FusionVersion"] == "Clickteam Fusion HTML5 Exporter Build 291.2"
// in runtime\application\CRunApp.js
// Anyway, if the UWP exporter is updated, then copy lines 32-35 to a newer version

// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkId=232509
(function () {
    //"use strict";

    //setup some defaults in UWP
    navigator.gamepadInputEmulation = "gamepad";//disable xbox gamepad emulation by default (it will be turned on later)

    //bootup

    //multiple_source_code_begin
    //create array of scripts (we do this so that we can decide how to include them into the document)
    var scripts = [];

    //debug


    //libs
    scripts.push("js/runtime/libs/jszip.js");
    scripts.push("js/runtime/libs/jszip-inflate.js");
    scripts.push("js/runtime/libs/jszip-load.js");
    scripts.push("js/runtime/libs/Matrix.js");
    scripts.push("js/runtime/libs/Matrix2D.js");
    scripts.push("js/runtime/libs/Box2D.js");

	// Bluewing Client libraries for text encoding, zlib compression, and counting UTF-8 graphemes
    scripts.push("js/runtime/libs/encoding.min.js");
    scripts.push("js/runtime/libs/zlib.min.js");
    scripts.push("js/runtime/libs/grapheme-splitter.min.js");

    //debug only libs

    //core stuff
    scripts.push("js/runtime/objects/CBoundingCache.js");

    //compile time
    scripts.push("js/Extensions.js");
    scripts.push("js/Movements.js");

    //services
    scripts.push("js/runtime/services/CServices.js");
    scripts.push("js/runtime/services/CArrayList.js");
    scripts.push("js/runtime/services/CBrowserDetect.js");
    scripts.push("js/runtime/services/CDisplayText.js");
    scripts.push("js/runtime/services/CFile.js");
    scripts.push("js/runtime/services/CFontInfo.js");
    scripts.push("js/runtime/services/CIni.js");
    scripts.push("js/runtime/services/CObjectSelection.js");
    scripts.push("js/runtime/services/CPoint.js");
    scripts.push("js/runtime/services/CRect.js");
    scripts.push("js/runtime/services/CReplaceColor.js");
    scripts.push("js/runtime/services/CTokeniser.js");
    scripts.push("js/runtime/services/CZone.js");
    scripts.push("js/runtime/services/CGraphicFont.js");

    //renderers
    scripts.push("js/runtime/renderers/CRenderer.js");
    scripts.push("js/runtime/renderers/CRendererImageBuffer.js");
    scripts.push("js/runtime/renderers/CRendererTextContainer.js");
    scripts.push("js/runtime/renderers/CRendererImageContainer.js");

    scripts.push("js/runtime/renderers/canvas/CCanvasRenderer.js");
    scripts.push("js/runtime/renderers/canvas/CCanvasRendererImageBuffer.js");
    scripts.push("js/runtime/renderers/canvas/CCanvasRendererTextContainer.js");
    scripts.push("js/runtime/renderers/canvas/CCanvasRendererImageContainer.js");

    scripts.push("js/runtime/renderers/webgl/CWebGLRenderer.js");
    scripts.push("js/runtime/renderers/webgl/CWebGLRendererTexture.js");
    scripts.push("js/runtime/renderers/webgl/CWebGLRendererShader.js");
    scripts.push("js/runtime/renderers/webgl/CWebGLRendererImageBuffer.js");
    scripts.push("js/runtime/renderers/webgl/CWebGLRendererTextContainer.js");
    scripts.push("js/runtime/renderers/webgl/CWebGLRendererImageContainer.js");

    //shaders
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderBasic.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderBasic.vert");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderFilled.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderFilled.vert");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderGradient.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderGradient.vert");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderGradientEllipse.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderGradientEllipse.vert");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderPattern.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderPattern.vert");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderTextured.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderTextured.vert");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderTexturedEllipse.frag");
    //+scripts.push("js/runtime/renderers/webgl/shaders/shaderTexturedEllipse.vert");

    //application
    scripts.push("js/runtime/application/CRunApp.js");
    scripts.push("js/runtime/application/CRunFrame.js");
    scripts.push("js/runtime/application/CSoundPlayer.js");
    scripts.push("js/runtime/application/CGamepad.js");
    scripts.push("js/runtime/application/CVirtualMouse.js");
    scripts.push("js/runtime/application/CVirtualJoystick.js");
    scripts.push("js/runtime/application/CVirtualJoystickTouch.js");
    scripts.push("js/runtime/application/CVirtualJoystickAccelerometer.js");
    scripts.push("js/runtime/application/CEmbeddedFile.js");
    scripts.push("js/runtime/application/CFakeTouchEvent.js");
    scripts.push("js/runtime/application/CFakePointerEvent.js");

    //events
    scripts.push("js/runtime/events/CEventProgram.js");
    scripts.push("js/runtime/events/CEventGroup.js");
    scripts.push("js/runtime/events/CEvent.js");
    scripts.push("js/runtime/events/CPushedEvent.js");
    scripts.push("js/runtime/events/CGroupFind.js");
    scripts.push("js/runtime/events/CLoadQualifiers.js");
    scripts.push("js/runtime/events/CPosOnLoop.js");
    scripts.push("js/runtime/events/CQualToOiList.js");
    scripts.push("js/runtime/events/CForEach.js");
    scripts.push("js/runtime/events/CTimerEvents.js");
    scripts.push("js/runtime/events/SaveSelection.js");

    //animations
    scripts.push("js/runtime/animations/CAnim.js");
    scripts.push("js/runtime/animations/CAnimDir.js");
    scripts.push("js/runtime/animations/CAnimHeader.js");
    scripts.push("js/runtime/animations/CRAni.js");

    //params
    scripts.push("js/runtime/params/CParam.js");
    scripts.push("js/runtime/params/CPosition.js");
    scripts.push("js/runtime/params/CPositionInfo.js");
    scripts.push("js/runtime/params/PARAM_2SHORTS.js");
    scripts.push("js/runtime/params/PARAM_CMPTIME.js");
    scripts.push("js/runtime/params/PARAM_COLOUR.js");
    scripts.push("js/runtime/params/PARAM_CREATE.js");
    scripts.push("js/runtime/params/PARAM_EVERY.js");
    scripts.push("js/runtime/params/PARAM_EXPRESSION.js");
    scripts.push("js/runtime/params/PARAM_EXTENSION.js");
    scripts.push("js/runtime/params/PARAM_GROUP.js");
    scripts.push("js/runtime/params/PARAM_GROUPPOINTER.js");
    scripts.push("js/runtime/params/PARAM_INT.js");
    scripts.push("js/runtime/params/PARAM_KEY.js");
    scripts.push("js/runtime/params/PARAM_OBJECT.js");
    scripts.push("js/runtime/params/PARAM_POSITION.js");
    scripts.push("js/runtime/params/PARAM_SAMPLE.js");
    scripts.push("js/runtime/params/PARAM_SHOOT.js");
    scripts.push("js/runtime/params/PARAM_SHORT.js");
    scripts.push("js/runtime/params/PARAM_STRING.js");
    scripts.push("js/runtime/params/PARAM_TIME.js");
    scripts.push("js/runtime/params/PARAM_ZONE.js");
    scripts.push("js/runtime/params/PARAM_CHILDEVENT.js");
    scripts.push("js/runtime/params/PARAM_MULTIPLEVAR.js");

    //movements
    scripts.push("js/runtime/movements/CMove.js");
    scripts.push("js/runtime/movements/CMoveDef.js");
    scripts.push("js/runtime/movements/CMoveDefList.js");
    scripts.push("js/runtime/movements/CMoveDefExtension.js");
    scripts.push("js/runtime/movements/CMoveExtension.js");
    scripts.push("js/runtime/movements/CRunMvtExtension.js");
    scripts.push("js/runtime/movements/CRMvt.js");

    //runlooop
    scripts.push("js/runtime/runloop/CRun.js");
    scripts.push("js/runtime/runloop/CCreateObjectInfo.js");
    scripts.push("js/runtime/runloop/CObjInfo.js");
    scripts.push("js/runtime/runloop/CRunMBase.js");
    scripts.push("js/runtime/runloop/CSaveGlobal.js");
    scripts.push("js/runtime/runloop/CSaveGlobalCounter.js");
    scripts.push("js/runtime/runloop/CSaveGlobalText.js");
    scripts.push("js/runtime/runloop/CSaveGlobalValues.js");

    //extensions
    scripts.push("js/runtime/extensions/CExtLoader.js");
    scripts.push("js/runtime/extensions/CExtLoad.js");
    scripts.push("js/runtime/extensions/CRunExtension.js");
    scripts.push("js/runtime/extensions/CActExtension.js");
    scripts.push("js/runtime/extensions/CCndExtension.js");
    scripts.push("js/runtime/extensions/CExpExtension.js");
    scripts.push("js/runtime/extensions/CRunControl.js");

    //objects
    scripts.push("js/runtime/objects/CSceneNode.js");
    scripts.push("js/runtime/objects/CObject.js");
    scripts.push("js/runtime/objects/CActive.js");
    scripts.push("js/runtime/objects/CCCA.js");
    scripts.push("js/runtime/objects/CCounter.js");
    scripts.push("js/runtime/objects/CExtension.js");
    scripts.push("js/runtime/objects/CLives.js");
    scripts.push("js/runtime/objects/CQuestion.js");
    scripts.push("js/runtime/objects/CRCom.js");
    scripts.push("js/runtime/objects/CScore.js");
    scripts.push("js/runtime/objects/CText.js");

    //sprites
    scripts.push("js/runtime/sprites/CRSpr.js");
    scripts.push("js/runtime/sprites/CMask.js");
    scripts.push("js/runtime/sprites/CColMask.js");
    scripts.push("js/runtime/sprites/CRotatedMask.js");

    //frame
    scripts.push("js/runtime/frame/CLO.js");
    scripts.push("js/runtime/frame/CLOList.js");
    scripts.push("js/runtime/frame/CBackInstance.js");
    scripts.push("js/runtime/frame/CLayerPlane.js");
    scripts.push("js/runtime/frame/CLayer.js");

    //oi
    scripts.push("js/runtime/oi/COI.js");
    scripts.push("js/runtime/oi/COIList.js");
    scripts.push("js/runtime/oi/COC.js");
    scripts.push("js/runtime/oi/COCBackground.js");
    scripts.push("js/runtime/oi/COCQBackdrop.js");
    scripts.push("js/runtime/oi/CObjectCommon.js");
    scripts.push("js/runtime/oi/CDefCCA.js");
    scripts.push("js/runtime/oi/CDefCounter.js");
    scripts.push("js/runtime/oi/CDefCounters.js");
    scripts.push("js/runtime/oi/CDefRtf.js");
    scripts.push("js/runtime/oi/CDefText.js");
    scripts.push("js/runtime/oi/CDefTexts.js");

    //banks
    scripts.push("js/runtime/banks/CImageBank.js");
    scripts.push("js/runtime/banks/CImage.js");
    scripts.push("js/runtime/banks/CSoundBank.js");
    scripts.push("js/runtime/banks/CSound.js");
    scripts.push("js/runtime/banks/CFontBank.js");
    scripts.push("js/runtime/banks/CFont.js");
    scripts.push("js/runtime/banks/CMosaic.js");

    //values
    scripts.push("js/runtime/values/CDefStrings.js");
    scripts.push("js/runtime/values/CDefValues.js");
    scripts.push("js/runtime/values/CRVal.js");

    //transitions
    scripts.push("js/runtime/transitions/CTransitionData.js");
    scripts.push("js/runtime/transitions/CTransition.js");
    scripts.push("js/runtime/transitions/CTrans.js");
    scripts.push("js/runtime/transitions/CTransitionManager.js");

    //box2d
    scripts.push("js/runtime/box2d/CContactListener.js");
    scripts.push("js/runtime/box2d/CJoint.js");

    //preloaders
    scripts.push("js/runtime/preloaders/CPreloaderProgress.js");
    scripts.push("js/runtime/preloaders/CPreloaderBlank.js");
    scripts.push("js/runtime/preloaders/CPreloaderImage.js");
    scripts.push("js/runtime/preloaders/CPreloaderFrame.js");
    scripts.push("js/runtime/preloaders/CPreloaderLastFrameBuffer.js");

    //dialogs
    scripts.push("js/runtime/dialogs/CDialog.js");
    scripts.push("js/runtime/dialogs/CMessageDialog.js");
    scripts.push("js/runtime/dialogs/CInputDialog.js");

    //targets
    scripts.push("js/runtime/targets/w10/CW10Runtime.js");
    scripts.push("js/runtime/targets/w10/CW10Application.js");
    scripts.push("js/runtime/targets/w10/CW10Gamepad.js");
    scripts.push("js/runtime/targets/w10/CW10MessageDialog.js");
    scripts.push("js/runtime/targets/w10/CW10InputDialog.js");
    //scripts.push("js/runtime/targets/web/CWebPolyfill.js");
    //scripts.push("js/runtime/targets/web/CWebRuntime.js");
    //scripts.push("js/runtime/targets/web/CWebApplication.js");
    //scripts.push("js/runtime/targets/web/CWebGamepad.js");
    //scripts.push("js/runtime/targets/web/CWebMessageDialog.js");
    //scripts.push("js/runtime/targets/web/CWebInputDialog.js");


    //conditions
    scripts.push("js/runtime/conditions/CCnd.js");
    scripts.push("js/runtime/conditions/CND_NEVER.js");
    scripts.push("js/runtime/conditions/CND_ALWAYS.js");
    scripts.push("js/runtime/conditions/CND_CHANCE.js");
    scripts.push("js/runtime/conditions/CND_COMPARE.js");
    scripts.push("js/runtime/conditions/CND_COMPAREG.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGCONST_EQ.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGCONST_NE.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGCONST_LE.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGCONST_LT.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGCONST_GE.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGCONST_GT.js");
    scripts.push("js/runtime/conditions/CND_COMPAREGSTRING.js");
    scripts.push("js/runtime/conditions/CND_GROUPACTIVATED.js");
    scripts.push("js/runtime/conditions/CND_GROUPSTART.js");
    scripts.push("js/runtime/conditions/CND_NOMORE.js");
    scripts.push("js/runtime/conditions/CND_NOTALWAYS.js");
    scripts.push("js/runtime/conditions/CND_ONCE.js");
    scripts.push("js/runtime/conditions/CND_ONLOOP.js");
    scripts.push("js/runtime/conditions/CND_REPEAT.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEALL.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEALL_OLD.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEALLINLINE.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEALLINZONE.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEALLINZONE_OLD.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEFLAGRESET.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEFLAGRESET_OLD.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEFLAGSET.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEFLAGSET_OLD.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEVALUE.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEVALUE_OLD.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEZONE.js");
    scripts.push("js/runtime/conditions/CND_CHOOSEZONE_OLD.js");
    scripts.push("js/runtime/conditions/CND_NOMOREALLZONE.js");
    scripts.push("js/runtime/conditions/CND_NOMOREALLZONE_OLD.js");
    scripts.push("js/runtime/conditions/CND_NUzMOFALLOBJECT.js");
    scripts.push("js/runtime/conditions/CND_NUMOFALLOBJECT_OLD.js");
    scripts.push("js/runtime/conditions/CND_NUMOFALLZONE.js");
    scripts.push("js/runtime/conditions/CND_NUMOFALLZONE_OLD.js");
    scripts.push("js/runtime/conditions/CND_PICKFROMID.js");
    scripts.push("js/runtime/conditions/CND_PICKFROMID_OLD.js");
    scripts.push("js/runtime/conditions/CND_EXTCHOOSE.js");
    scripts.push("js/runtime/conditions/CND_EXTONLOOP.js");
    scripts.push("js/runtime/conditions/CND_ENDOFPAUSE.js");
    scripts.push("js/runtime/conditions/CND_ISVSYNCON.js");
    scripts.push("js/runtime/conditions/CND_ISLADDER.js");
    scripts.push("js/runtime/conditions/CND_ISOBSTACLE.js");
    scripts.push("js/runtime/conditions/CND_QUITAPPLICATION.js");
    scripts.push("js/runtime/conditions/CND_START.js");
    scripts.push("js/runtime/conditions/CND_ONEVENT.js");
    scripts.push("js/runtime/conditions/CND_EVERY.js");
    scripts.push("js/runtime/conditions/CND_TIMEOUT.js");
    scripts.push("js/runtime/conditions/CND_TIMER.js");
    scripts.push("js/runtime/conditions/CND_TIMERINF.js");
    scripts.push("js/runtime/conditions/CND_EVERY2.js");
    scripts.push("js/runtime/conditions/CND_TIMEREQUALS.js");
    scripts.push("js/runtime/conditions/CND_TIMERSUP.js");
    scripts.push("js/runtime/conditions/CND_JOYPRESSED.js");
    scripts.push("js/runtime/conditions/CND_JOYPUSHED.js");
    scripts.push("js/runtime/conditions/CND_LIVE.js");
    scripts.push("js/runtime/conditions/CND_NOMORELIVE.js");
    scripts.push("js/runtime/conditions/CND_PLAYERPLAYING.js");
    scripts.push("js/runtime/conditions/CND_SCORE.js");
    scripts.push("js/runtime/conditions/CND_KBKEYDEPRESSED.js");
    scripts.push("js/runtime/conditions/CND_KBPRESSKEY.js");
    scripts.push("js/runtime/conditions/CND_MCLICK.js");
    scripts.push("js/runtime/conditions/CND_MCLICKINZONE.js");
    scripts.push("js/runtime/conditions/CND_MCLICKONOBJECT.js");
    scripts.push("js/runtime/conditions/CND_MINZONE.js");
    scripts.push("js/runtime/conditions/CND_MKEYDEPRESSED.js");
    scripts.push("js/runtime/conditions/CND_MONOBJECT.js");
    scripts.push("js/runtime/conditions/CND_MOUSEON.js");
    scripts.push("js/runtime/conditions/CND_ONMOUSEWHEELDOWN.js");
    scripts.push("js/runtime/conditions/CND_ONMOUSEWHEELUP.js");
    scripts.push("js/runtime/conditions/CND_ANYKEY.js");
    scripts.push("js/runtime/conditions/CND_NOSAMPLAYING.js");
    scripts.push("js/runtime/conditions/CND_NOSPCHANNELPLAYING.js");
    scripts.push("js/runtime/conditions/CND_NOSPSAMPLAYING.js");
    scripts.push("js/runtime/conditions/CND_SPCHANNELPAUSED.js");
    scripts.push("js/runtime/conditions/CND_SPSAMPAUSED.js");
    scripts.push("js/runtime/conditions/CND_QEQUAL.js");
    scripts.push("js/runtime/conditions/CND_QEXACT.js");
    scripts.push("js/runtime/conditions/CND_QFALSE.js");
    scripts.push("js/runtime/conditions/CND_CCAAPPFINISHED.js");
    scripts.push("js/runtime/conditions/CND_CCAFRAMECHANGED.js");
    scripts.push("js/runtime/conditions/CND_CCAISPAUSED.js");
    scripts.push("js/runtime/conditions/CND_CCAISVISIBLE.js");
    scripts.push("js/runtime/conditions/CND_CCOUNTER.js");
    scripts.push("js/runtime/conditions/CND_EXTHIDDEN.js");
    scripts.push("js/runtime/conditions/CND_EXTANIMENDOF.js");
    scripts.push("js/runtime/conditions/CND_EXTANIMPLAYING.js");
    scripts.push("js/runtime/conditions/CND_EXTBOUNCING.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPACC.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPDEC.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPFRAME.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPSPEED.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPVAR.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPVARCONST.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPVARFIXED.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPVARSTRING.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPX.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPY.js");
    scripts.push("js/runtime/conditions/CND_EXTCOLBACK.js");
    scripts.push("js/runtime/conditions/CND_EXTCOLLISION.js");
    scripts.push("js/runtime/conditions/CND_EXTENDPATH.js");
    scripts.push("js/runtime/conditions/CND_EXTFACING.js");
    scripts.push("js/runtime/conditions/CND_EXTFLAGRESET.js");
    scripts.push("js/runtime/conditions/CND_EXTFLAGSET.js");
    scripts.push("js/runtime/conditions/CND_EXTINPLAYFIELD.js");
    scripts.push("js/runtime/conditions/CND_EXTISBOLD.js");
    scripts.push("js/runtime/conditions/CND_EXTISCOLBACK.js");
    scripts.push("js/runtime/conditions/CND_EXTISCOLLIDING.js");
    scripts.push("js/runtime/conditions/CND_EXTISIN.js");
    scripts.push("js/runtime/conditions/CND_EXTISITALIC.js");
    scripts.push("js/runtime/conditions/CND_EXTISOUT.js");
    scripts.push("js/runtime/conditions/CND_EXTISSTRIKEOUT.js");
    scripts.push("js/runtime/conditions/CND_EXTISUNDERLINE.js");
    scripts.push("js/runtime/conditions/CND_EXTNEARBORDERS.js");
    scripts.push("js/runtime/conditions/CND_EXTNOMOREOBJECT.js");
    scripts.push("js/runtime/conditions/CND_EXTNOMOREZONE.js");
    scripts.push("js/runtime/conditions/CND_EXTNUMBERZONE.js");
    scripts.push("js/runtime/conditions/CND_EXTNUMOFOBJECT.js");
    scripts.push("js/runtime/conditions/CND_EXTOUTPLAYFIELD.js");
    scripts.push("js/runtime/conditions/CND_EXTPATHNODE.js");
    scripts.push("js/runtime/conditions/CND_EXTPATHNODENAME.js");
    scripts.push("js/runtime/conditions/CND_EXTREVERSED.js");
    scripts.push("js/runtime/conditions/CND_EXTSHOWN.js");
    scripts.push("js/runtime/conditions/CND_EXTSTOPPED.js");
    scripts.push("js/runtime/conditions/CND_RUNNINGAS.js");
    scripts.push("js/runtime/conditions/CND_STARTCHILDEVENT.js");

    scripts.push("js/runtime/conditions/CND_CMPANGLE.js");
    scripts.push("js/runtime/conditions/CND_CMPSCALEX.js");
    scripts.push("js/runtime/conditions/CND_CMPSCALEY.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPINSTANCEDATA.js");
    scripts.push("js/runtime/conditions/CND_EXTCMPLAYER.js");
    scripts.push("js/runtime/conditions/CND_EXTCOMPARE.js");
    scripts.push("js/runtime/conditions/CND_EXTPICKCLOSEST.js");
    scripts.push("js/runtime/conditions/CND_EXTPICKMAXVALUE.js");
    scripts.push("js/runtime/conditions/CND_EXTPICKMINVALUE.js");

    //actions
    scripts.push("js/runtime/actions/CAct.js");
    scripts.push("js/runtime/actions/CLoop.js");
    scripts.push("js/runtime/actions/ACT_SKIP.js");
    scripts.push("js/runtime/actions/ACT_ADDVARG.js");
    scripts.push("js/runtime/actions/ACT_ADDVARGCONST.js");
    scripts.push("js/runtime/actions/ACT_GRPACTIVATE.js");
    scripts.push("js/runtime/actions/ACT_GRPDEACTIVATE.js");
    scripts.push("js/runtime/actions/ACT_RANDOMIZE.js");
    scripts.push("js/runtime/actions/ACT_SETGLOBALSTRING.js");
    scripts.push("js/runtime/actions/ACT_SETLOOPINDEX.js");
    scripts.push("js/runtime/actions/ACT_SETVARG.js");
    scripts.push("js/runtime/actions/ACT_SETVARGCONST.js");
    scripts.push("js/runtime/actions/ACT_STOPLOOP.js");
    scripts.push("js/runtime/actions/ACT_SUBVARG.js");
    scripts.push("js/runtime/actions/ACT_SUBVARGCONST.js");
    scripts.push("js/runtime/actions/ACT_STARTLOOP.js");
    scripts.push("js/runtime/actions/ACT_EXECUTECHILDEVENTS.js");
    scripts.push("js/runtime/actions/ACT_HIDECURSOR.js");
    scripts.push("js/runtime/actions/ACT_SHOWCURSOR.js");
    scripts.push("js/runtime/actions/ACT_LOCKCHANNEL.js");
    scripts.push("js/runtime/actions/ACT_PAUSEALLCHANNELS.js");
    scripts.push("js/runtime/actions/ACT_PAUSECHANNEL.js");
    scripts.push("js/runtime/actions/ACT_PAUSESAMPLE.js");
    scripts.push("js/runtime/actions/ACT_PLAYCHANNEL.js");
    scripts.push("js/runtime/actions/ACT_PLAYLOOPCHANNEL.js");
    scripts.push("js/runtime/actions/ACT_PLAYLOOPSAMPLE.js");
    scripts.push("js/runtime/actions/ACT_PLAYSAMPLE.js");
    scripts.push("js/runtime/actions/ACT_PLAYSAMPLE2.js");
    scripts.push("js/runtime/actions/ACT_RESUMEALLCHANNELS.js");
    scripts.push("js/runtime/actions/ACT_RESUMECHANNEL.js");
    scripts.push("js/runtime/actions/ACT_RESUMESAMPLE.js");
    scripts.push("js/runtime/actions/ACT_SETCHANNELPOS.js");
    scripts.push("js/runtime/actions/ACT_SETCHANNELFREQ.js");
    scripts.push("js/runtime/actions/ACT_SETCHANNELVOL.js");
    scripts.push("js/runtime/actions/ACT_SETSAMPLEMAINVOL.js");
    scripts.push("js/runtime/actions/ACT_SETSAMPLEPOS.js");
    scripts.push("js/runtime/actions/ACT_SETSAMPLEFREQ.js");
    scripts.push("js/runtime/actions/ACT_SETSAMPLEVOL.js");
    scripts.push("js/runtime/actions/ACT_STOPCHANNEL.js");
    scripts.push("js/runtime/actions/ACT_STOPSAMPLE.js");
    scripts.push("js/runtime/actions/ACT_STOPSPESAMPLE.js");
    scripts.push("js/runtime/actions/ACT_UNLOCKCHANNEL.js");
    scripts.push("js/runtime/actions/ACT_STRDESTROY.js");
    scripts.push("js/runtime/actions/ACT_STRDISPLAY.js");
    scripts.push("js/runtime/actions/ACT_STRDISPLAYDURING.js");
    scripts.push("js/runtime/actions/ACT_STRDISPLAYSTRING.js");
    scripts.push("js/runtime/actions/ACT_STRNEXT.js");
    scripts.push("js/runtime/actions/ACT_STRPREV.js");
    scripts.push("js/runtime/actions/ACT_STRSET.js");
    scripts.push("js/runtime/actions/ACT_STRSETCOLOUR.js");
    scripts.push("js/runtime/actions/ACT_STRSETSTRING.js");
    scripts.push("js/runtime/actions/ACT_CADDVALUE.js");
    scripts.push("js/runtime/actions/ACT_CSETCOLOR1.js");
    scripts.push("js/runtime/actions/ACT_CSETCOLOR2.js");
    scripts.push("js/runtime/actions/ACT_CSETMAX.js");
    scripts.push("js/runtime/actions/ACT_CSETMIN.js");
    scripts.push("js/runtime/actions/ACT_CSETVALUE.js");
    scripts.push("js/runtime/actions/ACT_CSUBVALUE.js");
    scripts.push("js/runtime/actions/ACT_CCAENDAPP.js");
    scripts.push("js/runtime/actions/ACT_CCAHIDE.js");
    scripts.push("js/runtime/actions/ACT_CCAJUMPFRAME.js");
    scripts.push("js/runtime/actions/ACT_CCANEXTFRAME.js");
    scripts.push("js/runtime/actions/ACT_CCAPAUSEAPP.js");
    scripts.push("js/runtime/actions/ACT_CCAPREVIOUSFRAME.js");
    scripts.push("js/runtime/actions/ACT_CCARESTARTAPP.js");
    scripts.push("js/runtime/actions/ACT_CCARESTARTFRAME.js");
    scripts.push("js/runtime/actions/ACT_CCARESUMEAPP.js");
    scripts.push("js/runtime/actions/ACT_CCASETWIDTH.js");
    scripts.push("js/runtime/actions/ACT_CCASETHEIGHT.js");
    scripts.push("js/runtime/actions/ACT_CCASETGLOBALSTRING.js");
    scripts.push("js/runtime/actions/ACT_CCASETGLOBALVALUE.js");
    scripts.push("js/runtime/actions/ACT_CCASHOW.js");
    scripts.push("js/runtime/actions/ACT_ADDLIVES.js");
    scripts.push("js/runtime/actions/ACT_NOINPUT.js");
    scripts.push("js/runtime/actions/ACT_RESTINPUT.js");
    scripts.push("js/runtime/actions/ACT_SETINPUT.js");
    scripts.push("js/runtime/actions/ACT_SETINPUTKEY.js");
    scripts.push("js/runtime/actions/ACT_SETLIVES.js");
    scripts.push("js/runtime/actions/ACT_SETPLAYERNAME.js");
    scripts.push("js/runtime/actions/ACT_SETSCORE.js");
    scripts.push("js/runtime/actions/ACT_SUBLIVES.js");
    scripts.push("js/runtime/actions/ACT_SUBSCORE.js");
    scripts.push("js/runtime/actions/ACT_ADDSCORE.js");
    scripts.push("js/runtime/actions/ACT_EVENTAFTER.js");
    scripts.push("js/runtime/actions/ACT_NEVENTSAFTER.js");
    scripts.push("js/runtime/actions/ACT_SETTIMER.js");
    scripts.push("js/runtime/actions/ACT_CDISPLAY.js");
    scripts.push("js/runtime/actions/ACT_CDISPLAYX.js");
    scripts.push("js/runtime/actions/ACT_CDISPLAYY.js");
    scripts.push("js/runtime/actions/ACT_SETFRAMEBDKCOLOR.js");
    scripts.push("js/runtime/actions/ACT_DELALLCREATEDBKD.js");
    scripts.push("js/runtime/actions/ACT_DELCREATEDBKDAT.js");
    scripts.push("js/runtime/actions/ACT_ENDGAME.js");
    scripts.push("js/runtime/actions/ACT_GOLEVEL.js");
    scripts.push("js/runtime/actions/ACT_NEXTLEVEL.js");
    scripts.push("js/runtime/actions/ACT_PAUSEKEY.js");
    scripts.push("js/runtime/actions/ACT_SETFRAMERATE.js");
    scripts.push("js/runtime/actions/ACT_SETVSYNCON.js");
    scripts.push("js/runtime/actions/ACT_SETVSYNCOFF.js");
    scripts.push("js/runtime/actions/ACT_FULLSCREENMODE.js");
    scripts.push("js/runtime/actions/ACT_WINDOWEDMODE.js");
    scripts.push("js/runtime/actions/ACT_PAUSEANYKEY.js");
    scripts.push("js/runtime/actions/ACT_PLAYDEMO.js");
    scripts.push("js/runtime/actions/ACT_PREVLEVEL.js");
    scripts.push("js/runtime/actions/ACT_SETFRAMEHEIGHT.js");
    scripts.push("js/runtime/actions/ACT_SETFRAMEWIDTH.js");
    scripts.push("js/runtime/actions/ACT_SETVIRTUALHEIGHT.js");
    scripts.push("js/runtime/actions/ACT_SETVIRTUALWIDTH.js");
    scripts.push("js/runtime/actions/ACT_SPRFRONT.js");
    scripts.push("js/runtime/actions/ACT_SPRPASTE.js");
    scripts.push("js/runtime/actions/ACT_SPRSETANGLE.js");
    scripts.push("js/runtime/actions/ACT_SPRLOADFRAME.js");
    scripts.push("js/runtime/actions/ACT_SPRREPLACECOLOR.js");
    scripts.push("js/runtime/actions/ACT_SPRSETSCALE.js");
    scripts.push("js/runtime/actions/ACT_SPRSETSCALEX.js");
    scripts.push("js/runtime/actions/ACT_SPRSETSCALEY.js");
    scripts.push("js/runtime/actions/ACT_SPRADDBKD.js");
    scripts.push("js/runtime/actions/ACT_QASK.js");
    scripts.push("js/runtime/actions/ACT_RESTARTGAME.js");
    scripts.push("js/runtime/actions/ACT_RESTARTLEVEL.js");
    scripts.push("js/runtime/actions/ACT_CREATE.js");
    scripts.push("js/runtime/actions/ACT_CREATEBYNAME.js");
    scripts.push("js/runtime/actions/ACT_CREATEBYNAMEEXP.js");
    scripts.push("js/runtime/actions/ACT_CREATEEXP.js");
    scripts.push("js/runtime/actions/ACT_EXTFOREACH.js");
    scripts.push("js/runtime/actions/ACT_EXTFOREACH2.js");
    scripts.push("js/runtime/actions/ACT_EXTBOUNCE.js");
    scripts.push("js/runtime/actions/ACT_EXTBRANCHNODE.js");
    scripts.push("js/runtime/actions/ACT_EXTCHGFLAG.js");
    scripts.push("js/runtime/actions/ACT_EXTCHGFLAGCONST.js");
    scripts.push("js/runtime/actions/ACT_EXTCLRFLAG.js");
    scripts.push("js/runtime/actions/ACT_EXTCLRFLAGCONST.js");
    scripts.push("js/runtime/actions/ACT_EXTDESTROY.js");
    scripts.push("js/runtime/actions/ACT_EXTDISPATCHVAR.js");
    scripts.push("js/runtime/actions/ACT_EXTDISPLAYDURING.js");
    scripts.push("js/runtime/actions/ACT_EXTFORCEANIM.js");
    scripts.push("js/runtime/actions/ACT_EXTFORCEDIR.js");
    scripts.push("js/runtime/actions/ACT_EXTFORCEFRAME.js");
    scripts.push("js/runtime/actions/ACT_EXTFORCESPEED.js");
    scripts.push("js/runtime/actions/ACT_EXTGOTONODE.js");
    scripts.push("js/runtime/actions/ACT_EXTHIDE.js");
    scripts.push("js/runtime/actions/ACT_EXTINKEFFECT.js");
    scripts.push("js/runtime/actions/ACT_EXTLOOKAT.js");
    scripts.push("js/runtime/actions/ACT_EXTMAXSPEED.js");
    scripts.push("js/runtime/actions/ACT_EXTMOVEAFTER.js");
    scripts.push("js/runtime/actions/ACT_EXTMOVEBEFORE.js");
    scripts.push("js/runtime/actions/ACT_EXTMOVETOLAYER.js");
    scripts.push("js/runtime/actions/ACT_EXTNEXTMOVE.js");
    scripts.push("js/runtime/actions/ACT_EXTPREVMOVE.js");
    scripts.push("js/runtime/actions/ACT_EXTRESTANIM.js");
    scripts.push("js/runtime/actions/ACT_EXTRESTDIR.js");
    scripts.push("js/runtime/actions/ACT_EXTRESTFRAME.js");
    scripts.push("js/runtime/actions/ACT_EXTRESTSPEED.js");
    scripts.push("js/runtime/actions/ACT_EXTREVERSE.js");
    scripts.push("js/runtime/actions/ACT_EXTSELMOVE.js");
    scripts.push("js/runtime/actions/ACT_EXTSETACCELERATION.js");
    scripts.push("js/runtime/actions/ACT_EXTSETALPHACOEF.js");
    scripts.push("js/runtime/actions/ACT_EXTSETBOLD.js");
    scripts.push("js/runtime/actions/ACT_EXTSETDECELERATION.js");
    scripts.push("js/runtime/actions/ACT_EXTSETDIR.js");
    scripts.push("js/runtime/actions/ACT_EXTSETDIRECTIONS.js");
    scripts.push("js/runtime/actions/ACT_EXTSETEFFECT.js");
    scripts.push("js/runtime/actions/ACT_EXTSETEFFECTPARAM.js");
    scripts.push("js/runtime/actions/ACT_EXTSETFLAG.js");
    scripts.push("js/runtime/actions/ACT_EXTSETFLAGCONST.js");
    scripts.push("js/runtime/actions/ACT_EXTSETFONTNAME.js");
    scripts.push("js/runtime/actions/ACT_EXTSETFONTSIZE.js");
    scripts.push("js/runtime/actions/ACT_EXTSETGRAVITY.js");
    scripts.push("js/runtime/actions/ACT_EXTSETITALIC.js");
    scripts.push("js/runtime/actions/ACT_EXTEXTRA.js");
    scripts.push("js/runtime/actions/ACT_EXTSETFLAGBYEXP.js");
    scripts.push("js/runtime/actions/ACT_EXTSETPOS.js");
    scripts.push("js/runtime/actions/ACT_EXTSETRGBCOEF.js");
    scripts.push("js/runtime/actions/ACT_EXTSETROTATINGSPEED.js");
    scripts.push("js/runtime/actions/ACT_EXTSETSEMITRANSPARENCY.js");
    scripts.push("js/runtime/actions/ACT_EXTSETTEXTCOLOR.js");
    scripts.push("js/runtime/actions/ACT_EXTSETUNDERLINE.js");
    scripts.push("js/runtime/actions/ACT_EXTSETVAR.js");
    scripts.push("js/runtime/actions/ACT_EXTSETVARCONST.js");
    scripts.push("js/runtime/actions/ACT_EXTSETVARSTRING.js");
    scripts.push("js/runtime/actions/ACT_EXTSETX.js");
    scripts.push("js/runtime/actions/ACT_EXTSETY.js");
    scripts.push("js/runtime/actions/ACT_EXTSHOOT.js");
    scripts.push("js/runtime/actions/ACT_EXTSHOOTTOWARD.js");
    scripts.push("js/runtime/actions/ACT_EXTSHOW.js");
    scripts.push("js/runtime/actions/ACT_EXTSHUFFLE.js");
    scripts.push("js/runtime/actions/ACT_EXTSPEED.js");
    scripts.push("js/runtime/actions/ACT_EXTSPRBACK.js");
    scripts.push("js/runtime/actions/ACT_EXTSPRFRONT.js");
    scripts.push("js/runtime/actions/ACT_SPRBACK.js");
    scripts.push("js/runtime/actions/ACT_EXTSTART.js");
    scripts.push("js/runtime/actions/ACT_EXTSTARTANIM.js");
    scripts.push("js/runtime/actions/ACT_EXTSTOP.js");
    scripts.push("js/runtime/actions/ACT_EXTSTOPANIM.js");
    scripts.push("js/runtime/actions/ACT_EXTADDVAR.js");
    scripts.push("js/runtime/actions/ACT_EXTADDVARCONST.js");
    scripts.push("js/runtime/actions/ACT_EXTSUBVAR.js");
    scripts.push("js/runtime/actions/ACT_EXTSUBVARCONST.js");
    scripts.push("js/runtime/actions/ACT_EXTWRAP.js");
    scripts.push("js/runtime/actions/ACT_EXTSETFRICTION.js");
    scripts.push("js/runtime/actions/ACT_EXTSETELASTICITY.js");
    scripts.push("js/runtime/actions/ACT_EXTAPPLYANGULARIMPULSE.js");
    scripts.push("js/runtime/actions/ACT_EXTAPPLYFORCE.js");
    scripts.push("js/runtime/actions/ACT_EXTAPPLYTORQUE.js");
    scripts.push("js/runtime/actions/ACT_EXTSETLINEARVELOCITY.js");
    scripts.push("js/runtime/actions/ACT_EXTSETANGULARVELOCITY.js");
    scripts.push("js/runtime/actions/ACT_EXTAPPLYIMPULSE.js");
    scripts.push("js/runtime/actions/ACT_EXTSTOPFORCE.js");
    scripts.push("js/runtime/actions/ACT_EXTSTOPTORQUE.js");

    //expressions
    scripts.push("js/runtime/expressions/CExp.js");
    scripts.push("js/runtime/expressions/EXP_EMPTY.js");
    scripts.push("js/runtime/expressions/EXP_ZERO.js");
    scripts.push("js/runtime/expressions/EXP_LONG.js");
    scripts.push("js/runtime/expressions/EXP_DOUBLE.js");
    scripts.push("js/runtime/expressions/EXP_EXTVAR.js");
    scripts.push("js/runtime/expressions/EXP_EXTVARSTRING.js");
    scripts.push("js/runtime/expressions/EXP_STRINGGLONAMED.js");
    scripts.push("js/runtime/expressions/EXP_VARGLONAMED.js");
    scripts.push("js/runtime/expressions/EXP_STRING.js");
    scripts.push("js/runtime/expressions/EXP_EXTVARBYINDEX.js");
    scripts.push("js/runtime/expressions/EXP_EXTVARSTRINGBYINDEX.js");
    scripts.push("js/runtime/expressions/EXP_DISTANCE.js");
    scripts.push("js/runtime/expressions/EXP_ANGLE.js");
    scripts.push("js/runtime/expressions/EXP_RANGE.js");
    scripts.push("js/runtime/expressions/EXP_RANDOMRANGE.js");
    scripts.push("js/runtime/expressions/EXP_RUNTIMENAME.js");
    scripts.push("js/runtime/expressions/EXP_ABS.js");
    scripts.push("js/runtime/expressions/EXP_ACOS.js");
    scripts.push("js/runtime/expressions/EXP_AND.js");
    scripts.push("js/runtime/expressions/EXP_ASIN.js");
    scripts.push("js/runtime/expressions/EXP_ATAN.js");
    scripts.push("js/runtime/expressions/EXP_ATAN2.js");
    scripts.push("js/runtime/expressions/EXP_CEIL.js");
    scripts.push("js/runtime/expressions/EXP_BIN.js");
    scripts.push("js/runtime/expressions/EXP_COS.js");
    scripts.push("js/runtime/expressions/EXP_DIV.js");
    scripts.push("js/runtime/expressions/EXP_DRIVE.js");
    scripts.push("js/runtime/expressions/EXP_DIRECTORY.js");
    scripts.push("js/runtime/expressions/EXP_PATH.js");
    scripts.push("js/runtime/expressions/EXP_EXP.js");
    scripts.push("js/runtime/expressions/EXP_FIND.js");
    scripts.push("js/runtime/expressions/EXP_FLOATTOSTRING.js");
    scripts.push("js/runtime/expressions/EXP_FLOOR.js");
    scripts.push("js/runtime/expressions/EXP_GETBLUE.js");
    scripts.push("js/runtime/expressions/EXP_GETGREEN.js");
    scripts.push("js/runtime/expressions/EXP_GETRED.js");
    scripts.push("js/runtime/expressions/EXP_GETRGB.js");
    scripts.push("js/runtime/expressions/EXP_HEX.js");
    scripts.push("js/runtime/expressions/EXP_INT.js");
    scripts.push("js/runtime/expressions/EXP_LEFT.js");
    scripts.push("js/runtime/expressions/EXP_LEN.js");
    scripts.push("js/runtime/expressions/EXP_LN.js");
    scripts.push("js/runtime/expressions/EXP_LOG.js");
    scripts.push("js/runtime/expressions/EXP_LOOPINDEX.js");
    scripts.push("js/runtime/expressions/EXP_LOWER.js");
    scripts.push("js/runtime/expressions/EXP_MAX.js");
    scripts.push("js/runtime/expressions/EXP_MID.js");
    scripts.push("js/runtime/expressions/EXP_MIN.js");
    scripts.push("js/runtime/expressions/EXP_MINUS.js");
    scripts.push("js/runtime/expressions/EXP_MOD.js");
    scripts.push("js/runtime/expressions/EXP_MULT.js");
    scripts.push("js/runtime/expressions/EXP_NEWLINE.js");
    scripts.push("js/runtime/expressions/EXP_OR.js");
    scripts.push("js/runtime/expressions/EXP_NOT.js");
    scripts.push("js/runtime/expressions/EXP_PARENTH1.js");
    scripts.push("js/runtime/expressions/EXP_PARENTH2.js");
    scripts.push("js/runtime/expressions/EXP_PLUS.js");
    scripts.push("js/runtime/expressions/EXP_POW.js");
    scripts.push("js/runtime/expressions/EXP_RANDOM.js");
    scripts.push("js/runtime/expressions/EXP_REVERSEFIND.js");
    scripts.push("js/runtime/expressions/EXP_RIGHT.js");
    scripts.push("js/runtime/expressions/EXP_ROUND.js");
    scripts.push("js/runtime/expressions/EXP_SIN.js");
    scripts.push("js/runtime/expressions/EXP_SQR.js");
    scripts.push("js/runtime/expressions/EXP_STR.js");
    scripts.push("js/runtime/expressions/EXP_STRINGGLO.js");
    scripts.push("js/runtime/expressions/EXP_TAN.js");
    scripts.push("js/runtime/expressions/EXP_UPPER.js");
    scripts.push("js/runtime/expressions/EXP_VAL.js");
    scripts.push("js/runtime/expressions/EXP_VARGLO.js");
    scripts.push("js/runtime/expressions/EXP_VIRGULE.js");
    scripts.push("js/runtime/expressions/EXP_XOR.js");
    scripts.push("js/runtime/expressions/EXP_EVENTAFTER.js");
    scripts.push("js/runtime/expressions/EXP_TIMCENT.js");
    scripts.push("js/runtime/expressions/EXP_TIMHOURS.js");
    scripts.push("js/runtime/expressions/EXP_TIMMINITS.js");
    scripts.push("js/runtime/expressions/EXP_TIMSECONDS.js");
    scripts.push("js/runtime/expressions/EXP_TIMVALUE.js");
    scripts.push("js/runtime/expressions/EXP_FRAMEALPHACOEF.js");
    scripts.push("js/runtime/expressions/EXP_FRAMERGBCOEF.js");
    scripts.push("js/runtime/expressions/EXP_FRAMERATE.js");
    scripts.push("js/runtime/expressions/EXP_GAMLEVEL.js");
    scripts.push("js/runtime/expressions/EXP_GAMLEVELNEW.js");
    scripts.push("js/runtime/expressions/EXP_GAMNPLAYER.js");
    scripts.push("js/runtime/expressions/EXP_GETCOLLISIONMASK.js");
    scripts.push("js/runtime/expressions/EXP_GETFRAMEBKDCOLOR.js");
    scripts.push("js/runtime/expressions/EXP_GETVIRTUALHEIGHT.js");
    scripts.push("js/runtime/expressions/EXP_GETVIRTUALWIDTH.js");
    scripts.push("js/runtime/expressions/EXP_PLAYHEIGHT.js");
    scripts.push("js/runtime/expressions/EXP_PLAYWIDTH.js");
    scripts.push("js/runtime/expressions/EXP_PLAYXLEFT.js");
    scripts.push("js/runtime/expressions/EXP_PLAYXRIGHT.js");
    scripts.push("js/runtime/expressions/EXP_PLAYYBOTTOM.js");
    scripts.push("js/runtime/expressions/EXP_PLAYYTOP.js");
    scripts.push("js/runtime/expressions/EXP_CRENUMBERALL.js");
    scripts.push("js/runtime/expressions/EXP_CGETCOLOR1.js");
    scripts.push("js/runtime/expressions/EXP_CGETCOLOR2.js");
    scripts.push("js/runtime/expressions/EXP_CGETMAX.js");
    scripts.push("js/runtime/expressions/EXP_CGETMIN.js");
    scripts.push("js/runtime/expressions/EXP_CVALUE.js");
    scripts.push("js/runtime/expressions/EXP_CCAGETGLOBALSTRING.js");
    scripts.push("js/runtime/expressions/EXP_CCAGETGLOBALVALUE.js");
    scripts.push("js/runtime/expressions/EXP_CCAGETFRAMENUMBER.js");
    scripts.push("js/runtime/expressions/EXP_STRGETCURRENT.js");
    scripts.push("js/runtime/expressions/EXP_STRGETNPARA.js");
    scripts.push("js/runtime/expressions/EXP_STRGETNUMBER.js");
    scripts.push("js/runtime/expressions/EXP_STRGETNUMERIC.js");
    scripts.push("js/runtime/expressions/EXP_STRNUMBER.js");
    scripts.push("js/runtime/expressions/EXP_EXTDISTANCE.js");
    scripts.push("js/runtime/expressions/EXP_EXTANGLE.js");
    scripts.push("js/runtime/expressions/EXP_EXTACC.js");
    scripts.push("js/runtime/expressions/EXP_EXTALPHACOEF.js");
    scripts.push("js/runtime/expressions/EXP_EXTDEC.js");
    scripts.push("js/runtime/expressions/EXP_EXTDIR.js");
    scripts.push("js/runtime/expressions/EXP_EXTFLAG.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETFONTCOLOR.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETFONTNAME.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETFONTSIZE.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETGRAVITY.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETLAYER.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETSEMITRANSPARENCY.js");
    scripts.push("js/runtime/expressions/EXP_EXTIDENTIFIER.js");
    scripts.push("js/runtime/expressions/EXP_EXTISPR.js");
    scripts.push("js/runtime/expressions/EXP_EXTNANI.js");
    scripts.push("js/runtime/expressions/EXP_EXTNMOVE.js");
    scripts.push("js/runtime/expressions/EXP_EXTNOBJECTS.js");
    scripts.push("js/runtime/expressions/EXP_EXTRGBCOEF.js");
    scripts.push("js/runtime/expressions/EXP_EXTSPEED.js");
    scripts.push("js/runtime/expressions/EXP_EXTXAP.js");
    scripts.push("js/runtime/expressions/EXP_EXTXLEFT.js");
    scripts.push("js/runtime/expressions/EXP_EXTXRIGHT.js");
    scripts.push("js/runtime/expressions/EXP_EXTXSPR.js");
    scripts.push("js/runtime/expressions/EXP_EXTYAP.js");
    scripts.push("js/runtime/expressions/EXP_EXTYBOTTOM.js");
    scripts.push("js/runtime/expressions/EXP_EXTYSPR.js");
    scripts.push("js/runtime/expressions/EXP_EXTYTOP.js");
    scripts.push("js/runtime/expressions/EXP_GETANGLE.js");
    scripts.push("js/runtime/expressions/EXP_GETRGBAT.js");
    scripts.push("js/runtime/expressions/EXP_GETSCALEX.js");
    scripts.push("js/runtime/expressions/EXP_GETSCALEY.js");
    scripts.push("js/runtime/expressions/EXP_GETCHANNELDUR.js");
    scripts.push("js/runtime/expressions/EXP_GETCHANNELFREQ.js");
    scripts.push("js/runtime/expressions/EXP_GETCHANNELPOS.js");
    scripts.push("js/runtime/expressions/EXP_GETCHANNELSNDNAME.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEDUR.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEFREQ.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEMAINPAN.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEMAINVOL.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEPOS.js");
    scripts.push("js/runtime/expressions/EXP_GETCHANNELVOL.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEVOL.js");
    scripts.push("js/runtime/expressions/EXP_GETSAMPLEPAN.js");
    scripts.push("js/runtime/expressions/EXP_GETCHANNELPAN.js");
    scripts.push("js/runtime/expressions/EXP_MOUSEWHEELDELTA.js");
    scripts.push("js/runtime/expressions/EXP_XMOUSE.js");
    scripts.push("js/runtime/expressions/EXP_YMOUSE.js");
    scripts.push("js/runtime/expressions/EXP_GETINPUT.js");
    scripts.push("js/runtime/expressions/EXP_GETINPUTKEY.js");
    scripts.push("js/runtime/expressions/EXP_GETPLAYERNAME.js");
    scripts.push("js/runtime/expressions/EXP_PLALIVES.js");
    scripts.push("js/runtime/expressions/EXP_PLASCORE.js");
    scripts.push("js/runtime/expressions/EXP_EXTLOOPINDEX.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETFRICTION.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETRESTITUTION.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETDENSITY.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETVELOCITY.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETANGLE.js");
    scripts.push("js/runtime/expressions/EXP_EXTWIDTH.js");
    scripts.push("js/runtime/expressions/EXP_EXTHEIGHT.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETMASS.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETANGULARVELOCITY.js");
    scripts.push("js/runtime/expressions/EXP_EXTGETNAME.js");
    scripts.push("js/runtime/expressions/EXP_EXTINSTANCEDATA.js");
    scripts.push("js/runtime/expressions/EXP_NUMBEROFSELECTED.js");

    //extensions_begin
    scripts.push("js/runtime/extensions/source/Accelerometer.js");
    scripts.push("js/runtime/extensions/source/ActiveBackdrop.js");
    scripts.push("js/runtime/extensions/source/AdvDir.js");
    scripts.push("js/runtime/extensions/source/AdvGameBoard.js");
    scripts.push("js/runtime/extensions/source/AdvPathMov.js");
    scripts.push("js/runtime/extensions/source/Box2DBase.js");
    scripts.push("js/runtime/extensions/source/Box2DFan.js");
    scripts.push("js/runtime/extensions/source/Box2DGround.js");
    scripts.push("js/runtime/extensions/source/Box2DJoint.js");
    scripts.push("js/runtime/extensions/source/Box2DMagnet.js");
    scripts.push("js/runtime/extensions/source/Box2DTreadmill.js");
    scripts.push("js/runtime/extensions/source/Box2DRopeAndChain.js");
    scripts.push("js/runtime/extensions/source/Box2DParticules.js");
    scripts.push("js/runtime/extensions/source/CalcRect.js");
    scripts.push("js/runtime/extensions/source/CreateByName.js");
    scripts.push("js/runtime/extensions/source/Easing.js");
    scripts.push("js/runtime/extensions/source/Facebook.js");
    scripts.push("js/runtime/extensions/source/clickteam_movement_controller.js");
    //scripts.push("js/runtime/extensions/source/fgl.js");
    //scripts.push("js/runtime/extensions/source/fglobject.js");
    scripts.push("js/runtime/extensions/source/ForEach.js");
    scripts.push("js/runtime/extensions/source/FunctionEggtimer.js");
    scripts.push("js/runtime/extensions/source/Get.js");
    scripts.push("js/runtime/extensions/source/GoogleMaps.js");
    scripts.push("js/runtime/extensions/source/GraphicFont.js");
    scripts.push("js/runtime/extensions/source/HTML5.js");
    scripts.push("js/runtime/extensions/source/HTML5Video.js");
    scripts.push("js/runtime/extensions/source/IIF.js");
    scripts.push("js/runtime/extensions/source/InAndOutController.js");
    scripts.push("js/runtime/extensions/source/Inventory.js");
    scripts.push("js/runtime/extensions/source/JoystickControl.js");
    scripts.push("js/runtime/extensions/source/KcArray.js");
    scripts.push("js/runtime/extensions/source/KcBoxA.js");
    scripts.push("js/runtime/extensions/source/KcBoxB.js");
    scripts.push("js/runtime/extensions/source/KcButton.js");
    scripts.push("js/runtime/extensions/source/kcclock.js");
    scripts.push("js/runtime/extensions/source/kccombo.js");
    scripts.push("js/runtime/extensions/source/KcCursor.js");
    scripts.push("js/runtime/extensions/source/KcDbl.js");
    scripts.push("js/runtime/extensions/source/kcdirect.js");
    scripts.push("js/runtime/extensions/source/kcedit.js");
    scripts.push("js/runtime/extensions/source/kcfile.js");
    scripts.push("js/runtime/extensions/source/kchisc.js");
    scripts.push("js/runtime/extensions/source/kcini.js");
    scripts.push("js/runtime/extensions/source/kclist.js");
    scripts.push("js/runtime/extensions/source/kcpica.js");
    scripts.push("js/runtime/extensions/source/kcpict.js");
    scripts.push("js/runtime/extensions/source/kcrandom.js");
    scripts.push("js/runtime/extensions/source/kcwctrl.js");
    scripts.push("js/runtime/extensions/source/Layer.js");
    scripts.push("js/runtime/extensions/source/Location.js");
    scripts.push("js/runtime/extensions/source/MobileFont.js");
    scripts.push("js/runtime/extensions/source/MoveSafely2.js");
    scripts.push("js/runtime/extensions/source/MultipleTouch.js");
    scripts.push("js/runtime/extensions/source/ObjectMover.js");
    scripts.push("js/runtime/extensions/source/parser.js");
    scripts.push("js/runtime/extensions/source/ScreenZoom.js");
    scripts.push("js/runtime/extensions/source/StringTokenizer.js");
    scripts.push("js/runtime/extensions/source/WargameMap.js");
    scripts.push("js/runtime/extensions/source/Perspective.js");
    scripts.push("js/runtime/extensions/source/Platform.js");
    scripts.push("js/runtime/extensions/source/XBOXGamepad.js");
    //extensions_end

	//movements
    scripts.push("js/runtime/movements/source/MoveBall.js");
    scripts.push("js/runtime/movements/source/MoveBullet.js");
    scripts.push("js/runtime/movements/source/MoveDisappear.js");
    scripts.push("js/runtime/movements/source/MoveGeneric.js");
    scripts.push("js/runtime/movements/source/MoveMouse.js");
    scripts.push("js/runtime/movements/source/MovePath.js");
    scripts.push("js/runtime/movements/source/MovePlatform.js");
    scripts.push("js/runtime/movements/source/MoveRace.js");
    scripts.push("js/runtime/movements/source/MoveStatic.js");

    //movement_extensions_begin
    scripts.push("js/runtime/movements/source/clickteam_circular.js");
    scripts.push("js/runtime/movements/source/clickteam_dragdrop.js");
    scripts.push("js/runtime/movements/source/clickteam_invaders.js");
    scripts.push("js/runtime/movements/source/clickteam_presentation.js");
    scripts.push("js/runtime/movements/source/clickteam_regpolygon.js");
    scripts.push("js/runtime/movements/source/clickteam_simple_ellipse.js");
    scripts.push("js/runtime/movements/source/clickteam_sinewave.js");
    scripts.push("js/runtime/movements/source/clickteam_vector.js");
    scripts.push("js/runtime/movements/source/Box2D8Directions.js");
    scripts.push("js/runtime/movements/source/Box2DAxial.js");
    scripts.push("js/runtime/movements/source/Box2DBackground.js");
    scripts.push("js/runtime/movements/source/Box2DBouncingBall.js");
    scripts.push("js/runtime/movements/source/Box2DPlatform.js");
    scripts.push("js/runtime/movements/source/Box2DRaceCar.js");
    scripts.push("js/runtime/movements/source/Box2DSpaceShip.js");
    scripts.push("js/runtime/movements/source/Box2DSpring.js");
    scripts.push("js/runtime/movements/source/Box2DStatic.js");
    scripts.push("js/runtime/movements/source/inandout.js");
    scripts.push("js/runtime/movements/source/pinball.js");
    scripts.push("js/runtime/movements/source/spaceship.js");
    //movement_extensions_end

    //transitions
    scripts.push("js/runtime/transitions/CTransition.js");
    scripts.push("js/runtime/transitions/CTransitionData.js");
    scripts.push("js/runtime/transitions/CTransitionCCTrans.js");
    scripts.push("js/runtime/transitions/CTransitionManager.js");
    scripts.push("js/runtime/transitions/CTrans.js");
    scripts.push("js/runtime/transitions/CTransAdvancedScrolling.js");
    scripts.push("js/runtime/transitions/CTransBack.js");
    scripts.push("js/runtime/transitions/CTransBand.js");
    scripts.push("js/runtime/transitions/CTransCell.js");
    scripts.push("js/runtime/transitions/CTransDoor.js");
    scripts.push("js/runtime/transitions/CTransFade.js");
    scripts.push("js/runtime/transitions/CTransLine.js");
    scripts.push("js/runtime/transitions/CTransMosaic.js");
    scripts.push("js/runtime/transitions/CTransOpen.js");
    scripts.push("js/runtime/transitions/CTransPush.js");
    scripts.push("js/runtime/transitions/CTransScroll.js");
    scripts.push("js/runtime/transitions/CTransSquare.js");
    scripts.push("js/runtime/transitions/CTransStretch.js");
    scripts.push("js/runtime/transitions/CTransStretch2.js");
    scripts.push("js/runtime/transitions/CTransTrame.js");
    scripts.push("js/runtime/transitions/CTransTurn.js");
    scripts.push("js/runtime/transitions/CTransTurn2.js");
    scripts.push("js/runtime/transitions/CTransZigZag.js");
    scripts.push("js/runtime/transitions/CTransZigZag2.js");
    scripts.push("js/runtime/transitions/CTransZoom.js");
    scripts.push("js/runtime/transitions/CTransZoom2.js");
	//next_script

    //load all of the scripts
    var now = Date.now();
    for (var index in scripts) {
        var script = scripts[index];
        document.write('<script src="' + script + '?' + now + '" type="text/javascript"></script>');
    }
    //multiple_source_code_end

    //create the winJS app
    var appWinJS = WinJS.Application;
    var activation = Windows.ApplicationModel.Activation;
    var isFirstActivation = true;
    var _scope = appWinJS;

    //make sure tehre is an empty Runtime object
    var runtimeCreated = false;
    if (typeof window.Runtime == "undefined") {
        window.Runtime = {};
    }

    //add some app argss
    appWinJS.onactivated = function (args) {
        //what type of launch
        if (args.detail.kind == activation.ActivationKind.launch) {
            //check previous mode of execution
            switch(args.detail.previousExecutionState) {
                case activation.ApplicationExecutionState.running:
                    //still running so dont have to do anything
                    break;
                case activation.ApplicationExecutionState.suspended:
                    //resume app

                    //update timer details before
                    Runtime.application.timer = Date.now();

                    //now resume
                    Runtime.application.run.resume();

                    break;
            }
        }

        //add visibility handler and let the app process
        if (isFirstActivation) {
            // TODO: The app was activated and had not been running. Do general startup initialization here.
            document.addEventListener("visibilitychange", onVisibilityChanged);

            args.setPromise(WinJS.UI.processAll());
        }

        isFirstActivation = false;
    };

    appWinJS.onbackclick = function (evt) {
        if (Runtime.isConsole())
            return true;
        return false;
    };

    function onVisibilityChanged(args) {
        //check if app is showing or hiding
        if (!document.hidden && !runtimeCreated) {
            //check if this is for the first time
            if (!runtimeCreated) {
                //create the runtime
                window.Runtime = new CW10Runtime();
                runtimeCreated = true;

                //init
                window.Runtime.onSetupDevice();
                window.Runtime.onInit(document.getElementById('FusionRoot'), document.getElementById('FusionCanvas'), document.getElementById('FusionContainer'), '../resources/PRJNAME.cch', _scope);
                window.Runtime.onSetupEvents();
            }
        }
    }

    appWinJS.addEventListener("checkpoint", function (args) {
        //the app is "suspending" so we have teh oppertunity to save some data
        if (Runtime.application) {
            //save some stuff to the session state (this will be available if the app then terminates)
            //WinJS.Application.sessionState.fusionState = { blah: 123 };

            //set async task to let fusion update once!
            args.setPromise(new WinJS.Promise(function (complete, error) {
                //we should inform the app that it is suspending

                //we now call an update in the app
                Runtime.application.onUpdate();

                //finish the async task
                complete();

            }).then(function () {
                //so the async task has complete, so we should tell fusion to pause the app
                Runtime.application.run.pause(true);//true keeps sounds active
            }));
        }
    });

    //we have to add teh resuming event differently to activated/checkpoint, just because the SDK says so...
    Windows.UI.WebUI.WebUIApplication.addEventListener("resuming", function (args) {
        //the app is resuming from a paused state, everything should be safe to continue without doing anything
        if (Runtime.application) {
            Runtime.application.run.resume(false);

            //trigger end of pause event, this is a bit hacky that we are doing it by hand, there should be a built in API in fusion rumtime...
            if (Runtime.application.appRunningState == CRunApp.SL_FRAMELOOPFIRST || Runtime.application.appRunningState == CRunApp.SL_FRAMELOOP) {
                //we only allow this if the frame was currently running in teh FRAMELOOP otherwise it will crash the runtime.. eg if it was in a frame transition

                //setup end of pause variable so teh condition returns true
                Runtime.application.run.rh4EndOfPause = this.rhLoopCount;

                //handle the condition: (-8 << 16) | 0xFFFD = CND_ENDOFPAUSE
                Runtime.application.run.rhEvtProg.handle_GlobalEvents((-8 << 16) | 0xFFFD);
            }
        }

    }, false);

    //bootup the app
    appWinJS.start();
})();

// Make XBOX features global
try {
    var xbox = Microsoft.Xbox.Services;
    var m_user = new Microsoft.Xbox.Services.System.XboxLiveUser();
    var statusManager = xbox.Statistics.Manager.StatisticManager.singletonInstance;
} catch (err) {
    Debug.writeln("Note:" + err);
}
