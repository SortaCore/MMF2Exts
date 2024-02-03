#include "Common.hpp"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#ifdef _WIN32
Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this), FusionDebugger(this)
#elif defined(__ANDROID__)
Extension::Extension(const EDITDATA* const edPtr, const jobject javaExtPtr) :
	javaExtPtr(javaExtPtr, "Extension::javaExtPtr from Extension ctor"),
	Runtime(this, this->javaExtPtr), FusionDebugger(this)
#else
Extension::Extension(const EDITDATA* const edPtr, void* const objCExtPtr) :
	objCExtPtr(objCExtPtr), Runtime(this, objCExtPtr), FusionDebugger(this)
#endif
{
    /*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */

	// Actions

    LinkAction(0, Template_SetFuncSignature);
	LinkAction(1, Template_SetDefaultReturnI);
	LinkAction(2, Template_SetDefaultReturnF);
	LinkAction(3, Template_SetDefaultReturnS);
	LinkAction(4, Template_SetDefaultReturnN);
	LinkAction(5, Template_Param_SetDefaultValueI);
	LinkAction(6, Template_Param_SetDefaultValueF);
	LinkAction(7, Template_Param_SetDefaultValueS);
	LinkAction(8, Template_Param_SetDefaultValueN);
	LinkAction(9, Template_SetScopedVarOnStartI );
	LinkAction(10, Template_SetScopedVarOnStartF);
	LinkAction(11, Template_SetScopedVarOnStartS);
	LinkAction(12, Template_CancelScopedVarOnStart);
	LinkAction(13, Template_SetGlobalID)
	LinkAction(14, Template_SetEnabled);
	LinkAction(15, Template_RedirectFunction);
	LinkAction(16, Template_Loop);
	LinkAction(17, DelayedFunctions_Loop);
	LinkAction(18, DelayedFunctions_CancelByPrefix);
	LinkAction(19, RunFunction_ActionDummy_Num);
	LinkAction(20, RunFunction_ActionDummy_String);
	LinkAction(21, RunFunction_Foreach_Num);
	LinkAction(22, RunFunction_Foreach_String);
	LinkAction(23, RunFunction_Delayed_Num_MS);
	LinkAction(24, RunFunction_Delayed_String_MS);
	LinkAction(25, RunFunction_Delayed_Num_Ticks);
	LinkAction(26, RunFunction_Delayed_String_Ticks);
	LinkAction(27, RunFunction_Script);
	LinkAction(28, RunningFunc_SetReturnI);
	LinkAction(29, RunningFunc_SetReturnF);
	LinkAction(30, RunningFunc_SetReturnS);
	LinkAction(31, RunningFunc_ScopedVar_SetI)
	LinkAction(32, RunningFunc_ScopedVar_SetF);
	LinkAction(33, RunningFunc_ScopedVar_SetS);
	LinkAction(34, RunningFunc_Params_Loop);
	LinkAction(35, RunningFunc_ScopedVar_Loop);
	LinkAction(36, RunningFunc_StopFunction);
	LinkAction(37, RunningFunc_ChangeRepeatSetting);
	LinkAction(38, RunningFunc_Abort);
	LinkAction(39, Logging_SetLevel);
	LinkAction(40, Template_ImportFromAnotherFrame);


	// Conditions

    LinkCondition(0, AlwaysTrue); // OnDarkScriptError
	LinkCondition(1, OnFunction);
	LinkCondition(2, OnForeachFunction);
	LinkCondition(3, OnFunctionAborted);
	LinkCondition(4, IsRunningFuncStillActive);
	LinkCondition(5, DoesFunctionHaveTemplate);
	LinkCondition(6, IsFunctionInCallStack);
	LinkCondition(7, LoopNameMatch); // OnTemplateLoop
	LinkCondition(8, LoopNameMatch); // OnPendingFunctionLoop
	LinkCondition(9, LoopNameMatch); // OnParameterLoop
	LinkCondition(10, LoopNameMatch); // OnScopedVarLoop
	LinkCondition(11, Logging_OnAnyFunction);
	LinkCondition(12, Logging_OnAnyFunctionCompletedOK);
	LinkCondition(13, Logging_OnAnyFunctionAborted);


	// Expressions

	LinkExpression(0, Logging_GetDarkScriptError);
	LinkExpression(1, Logging_GetAbortReason);
	LinkExpression(2, Logging_GetCurrentLog);

	LinkExpression(3, RunningFunc_GetRepeatIndex);
	LinkExpression(4, RunningFunc_GetNumRepeatsLeft);
	LinkExpression(5, RunningFunc_GetNumRepeatsTotal);
	LinkExpression(6, RunningFunc_ForeachFV);
	LinkExpression(7, RunningFunc_NumParamsPassed);
	LinkExpression(8, RunningFunc_ScopedVar_GetI);
	LinkExpression(9, RunningFunc_ScopedVar_GetF);
	LinkExpression(10, RunningFunc_ScopedVar_GetS);
	LinkExpression(11, RunningFunc_GetParamValueByIndexI);
	LinkExpression(12, RunningFunc_GetParamValueByIndexF);
	LinkExpression(13, RunningFunc_GetParamValueByIndexS);
	LinkExpression(14, RunningFunc_GetAllParamsAsText);
	LinkExpression(15, RunningFunc_GetCallStack);

	LinkExpression(16, InternalLoop_GetIndex);
	LinkExpression(17, InternalLoop_GetVarName);
	LinkExpression(18, InternalLoop_GetVarType);

	LinkExpression(19, FuncTemplate_GetFunctionName);
	LinkExpression(20, FuncTemplate_GetNumRequiredParams);
	LinkExpression(21, FuncTemplate_GetNumPossibleParams);
	LinkExpression(22, FuncTemplate_GetReturnType);
	LinkExpression(23, FuncTemplate_ShouldRepeat);
	LinkExpression(24, FuncTemplate_ShouldBeDelayed);
	LinkExpression(25, FuncTemplate_RecursionAllowed);
	LinkExpression(26, FuncTemplate_IsEnabled);
	LinkExpression(27, FuncTemplate_GetRedirectFuncName);
	LinkExpression(28, FuncTemplate_GlobalID);
	LinkExpression(29, FuncTemplate_ParamNameByIndex);
	LinkExpression(30, FuncTemplate_ParamTypeByIndex);
	LinkExpression(31, FuncTemplate_ParamDefaultValByIndex);
	LinkExpression(32, FuncTemplate_ParamIndexByName);
	LinkExpression(33, FuncTemplate_ParamTypeByName);
	LinkExpression(34, FuncTemplate_ParamDefaultValByName);

	LinkExpression(35, LastReturn_AsInt);
	LinkExpression(36, LastReturn_AsFloat);
	LinkExpression(37, LastReturn_AsString);
	LinkExpression(38, LastReturn_Type);
	LinkExpression(39, RunningFunc_GetCalledFuncName);
	LinkExpression(40, TestFunc);

	// Used to make sure if we're doing a foreach, make sure we don't call a DarkScript function on another frame
	fusionFrameNum = rhPtr->App->nCurrentFrame;


	// Copy all object properties from EDITDATA to Extension
	globalID = edPtr->Props.GetPropertyStr("Global ID"sv);

	const std::tstring strictProp = edPtr->Props.GetPropertyStr("Conversion strictness level"sv);
	conversionStrictness = strictProp == _T("Exact"sv) ? ConversionStrictness::Exact :
		strictProp == _T("Exact"sv) ? ConversionStrictness::Exact :
		strictProp == _T("Float <-> Integer OK"sv) ? ConversionStrictness::Numeric :
		strictProp == _T("Any conversion (e.g. \"1\" -> 1.0)"sv) ? ConversionStrictness::AnyWithAborts :
		strictProp == _T("Any (no errors)"sv) ? ConversionStrictness::AnyWithDefaults : (ConversionStrictness)-1;
	if ((int)conversionStrictness == -1)
		DarkEdif::MsgBox::Error(_T("Property error"), _T("Unrecognised conversion strictness property value \"%s\"."), strictProp.c_str());

	funcsMustHaveTemplate = edPtr->Props.IsPropChecked("Require declaration for all functions"sv);
	keepTemplatesAcrossFrames = edPtr->Props.IsPropChecked("Keep declarations across frame switches"sv);
	keepGlobalScopedVarsAcrossFrames = edPtr->Props.IsPropChecked("Keep global vars across frame switches"sv);
	createErrorForUnhandledEvents = edPtr->Props.IsPropChecked("Create error for unhandled functions"sv);
	createErrorForUnhandledAborts = edPtr->Props.IsPropChecked("Create error for unhandled aborts"sv);
	enableOnAnyFunctions = edPtr->Props.IsPropChecked("Enable Any Function conditions"sv);
	createErrorOverridingScopedVars = edPtr->Props.IsPropChecked("Create an error when overriding scoped values"sv);
	preventAllRecursion = edPtr->Props.IsPropChecked("Prevent any recursion"sv);
	runAbortsOnDestination = edPtr->Props.IsPropChecked("Run errors/aborts on called extension"sv);
	allowForeachSingular = edPtr->Props.IsPropChecked("Allow singular objects in qualifier foreach"sv);
	inheritParametersAsScopedVariables = edPtr->Props.IsPropChecked("Inherit parameters as scoped variables"sv);


	const std::tstring globalScoped = edPtr->Props.GetPropertyStr("Global scoped vars"sv);
	// TODO: Loop global scoped ^ and insert

	GlobalData* gc = NULL;
	std::tstring key = _T("DarkScript"s) + globalID;
	if (!globalID.empty())
		gc = (GlobalData*)Runtime.ReadGlobal(key.c_str());
	if (gc == NULL)
	{
		globals = new GlobalData();
		if (!globalID.empty())
			Runtime.WriteGlobal(key.c_str(), globals);
	}
	else
	{
		globals = gc;
		curFrame = globals->curFrameOnFrameEnd;
	}
	globals->exts.push_back(this);

	// This can be set to false, then generate event, to find all events that have On Function conditions.
	// Of course, it won't detect group events inside deactivated groups.
	selfAwareness = true;
}

Extension::~Extension()
{
	globals->exts.erase(std::find(globals->exts.cbegin(), globals->exts.cend(), this));

	if (!globals->exts.empty())
		return; // only last ext cleans up

	// not a shared object; we can clear everything
	if (globalID.empty())
	{
		delete globals;
		return;
	}

	// should this ever be true
	if (!globals->runningFuncs.empty())
		DarkEdif::MsgBox::Error(_T("Extension dtor"), _T("Unexpectedly, functions were still executing during exit."));

	globals->pendingFuncs.erase(std::remove_if(globals->pendingFuncs.begin(), globals->pendingFuncs.end(),
		[](const auto& pf) { return !pf->keepAcrossFrames; }), globals->pendingFuncs.end());

	// Check function templates should be cleared on end
	if (!this->keepTemplatesAcrossFrames)
		globals->functionTemplates.clear();
	if (!this->keepGlobalScopedVarsAcrossFrames)
		globals->scopedVars.clear();
	globals->curFrameOnFrameEnd = curFrame;
}


REFLAG Extension::Handle()
{
	++curFrame;
	const auto now = std::chrono::system_clock::now();

	for (size_t i = 0; i < globals->pendingFuncs.size(); i++)
	{
		if (globals->pendingFuncs[i]->useTicks ? globals->pendingFuncs[i]->runAtTick <= curFrame : globals->pendingFuncs[i]->runAtTime < now)
		{
			std::shared_ptr<DelayedFunction> pf = globals->pendingFuncs[i];

			if (pf->useTicks && pf->startFrame < curFrame)
				CreateErrorT("Warning: delayed function \"%s\" started too late (%i < %i).", pf->funcToRun->funcTemplate->name.c_str(), pf->startFrame, curFrame);

			curDelayedFunc = pf;
			pf->funcToRun->runLocation = Sub_GetLocation(23);
			pf->funcToRun->isVoidRun = true;
			ExecuteFunction(NULL, pf->funcToRun);
			pf->funcToRun->runLocation.clear();
			if (--pf->numRepeats < 0 || !pf->funcToRun->active)
			{
				LOGI(_T("Delayed function %s was dequeued (num repeats = %i, abort reason: \"%s\").\n"), pf->funcToRun->funcTemplate->name.c_str(), pf->numRepeats, pf->funcToRun->abortReason.c_str());
				globals->pendingFuncs.erase(globals->pendingFuncs.begin() + i);
				--i;
			}
			else
			{
				if (pf->useTicks)
					pf->runAtTick = pf->numUnitsUntilRun;
				else
					pf->runAtTime = now + std::chrono::milliseconds(pf->numUnitsUntilRun);
			}
			curDelayedFunc.reset();
		}
	}

	return globals->pendingFuncs.empty() ? REFLAG::ONE_SHOT : REFLAG::NONE;
}


REFLAG Extension::Display()
{
    /*
       If you return REFLAG_DISPLAY in Handle() this routine will run.
    */

    // Ok
    return REFLAG::NONE;
}

short Extension::FusionRuntimePaused()
{
	if (globals->exts[0] == this && !globals->pendingFuncs.empty())
		globals->runtimepausedtime = decltype(globals->runtimepausedtime)::clock::now();

    // Ok
    return 0;
}

short Extension::FusionRuntimeContinued()
{
	if (globals->exts[0] == this && !globals->pendingFuncs.empty())
	{
		const auto diff = decltype(globals->runtimepausedtime)::clock::now() - globals->runtimepausedtime;
		for each (auto& f in globals->pendingFuncs)
			if (!f->useTicks)
				f->runAtTime += diff;
	}

    // Ok
    return 0;
}


// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error , but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}
