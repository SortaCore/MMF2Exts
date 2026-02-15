#include "Common.hpp"

bool Extension::AlwaysTrue() const
{
	return true;
}
bool Extension::LoopNameMatch(const TCHAR * loopName) const
{
	return !_tcsicmp(curLoopName.c_str(), loopName);
}

bool Extension::OnDarkScriptError()
{
	darkScriptErrorRead = true;
	return true;
}
bool Extension::OnFunction(const TCHAR * name)
{
	// We're running this in a dummy fashion; this may be used in future to make a dependency/call tree
	if (!selfAwareness)
	{
		LOGD(_T("OnFunction(\"%s\" %p) on event %i. Self aware = 0. Exiting.\n"), name, name, DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}
	assert(!globals->runningFuncs.empty());

	const std::tstring nameR(ToLower(name));
	auto & lowest = *globals->runningFuncs.back();
	LOGD(_T("OnFunction(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.funcTemplate->name.c_str(), lowest.currentIterationTriggering ? 1 : 0);

	if (lowest.funcTemplate->nameL != nameR || !lowest.currentIterationTriggering)
		return false;
	if (lowest.keepObjectSelection)
		evt_RestoreSelectedObjects(lowest.selectedObjects, true);
	lowest.eventWasHandled = true;
	return true;
}
bool Extension::OnForeachFunction(const TCHAR* name, int objOiList)
{
	// We're running this in a dummy fashion; this may be used in future to make a dependency/call tree
	if (!selfAwareness)
	{
		LOGD(_T("OnForeachFunction(\"%s\") on event %i. Self aware = 0. Exiting.\n"), name, DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}
	assert(!globals->runningFuncs.empty());
	const std::tstring nameR = ToLower(name);
	auto& lowest = *globals->runningFuncs.back();
	LOGD(_T("OnForeachFunction(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.funcTemplate->name.c_str(), lowest.currentIterationTriggering ? 1 : 0);

	if (lowest.funcTemplate->nameL != nameR || !lowest.currentIterationTriggering)
		return false;

	// Since it's a foreach
	RunObjectMultiPlatPtr ro = Runtime.RunObjPtrFromFixed(lowest.currentForeachObjFV);
	assert(ro != nullptr);
	HeaderObject* const ho = ro->get_rHo();
	const short actionExptOi = lowest.currentForeachOil;

	// For conditions, object parameters are passed as EventParam *, a ParamObject *, to exts;
	// but DarkEdif reads the OIList num from it and passes that instead, as an int.
	const short conditionExptOi = (short)objOiList;

	// Oi doesn't match, so the condition's selected object does not match the Foreach action.
	// If an oi >= 0, was on object, otherwise qualifier.
	// If a singular condition does not match the singular action, or both are qualifiers and don't match,
	// then we don't trigger.
	if (conditionExptOi != actionExptOi && (((conditionExptOi < 0) == (actionExptOi < 0)) ||
		// action is actionExptOi < 0, so action was run on qualifier. oi >= 0, condition is a singular.
		(actionExptOi < 0 && !allowQualifierToTriggerSingularForeach) ||
		// actionExptOi >= 0, so action was run on singular. oi < 0, condition is a qualifier.
		(actionExptOi >= 0 && !allowSingularToTriggerQualifierForeach)))
	{
		LOGD(_T("OnForeachFunction(\"%s\") on event %i. Oi %i does not match expected Oi %i.\n"),
			name, DarkEdif::GetCurrentFusionEventNum(this), conditionExptOi, actionExptOi);

		// If we just return false from a qualifier event, Fusion crashes.
		// We have to deselect all the qualifier entries, then return false.
		for (auto oi : DarkEdif::QualifierOIListIterator(rhPtr, actionExptOi, DarkEdif::Selection::All))
			oi->SelectNone(rhPtr);

		return false;
	}

	// Pre-restore any objects selected, if needed; don't deselect selected ones,
	// as the only ones selected are intended at this point
	if (lowest.keepObjectSelection)
		evt_RestoreSelectedObjects(lowest.selectedObjects, true);

	// Condition selected a singular object
	if (conditionExptOi >= 0)
	{
		// It doesn't match, so we can assume actionExptOi is a qualifier - or the earlier if would've killed it
		if (conditionExptOi != actionExptOi)
		{
			objInfoList * const poil = rhPtr->GetOIListByIndex(conditionExptOi);
			if (poil->get_Oi() != ho->get_Oi())
			{
				LOGD(_T("OnForeachFunction(\"%s\") on event %i. Expected FV is %i, Number = %i; does not equal passed FV is %i, number %i. Exiting.\n"),
					name, DarkEdif::GetCurrentFusionEventNum(this),
					lowest.currentForeachObjFV, (lowest.currentForeachObjFV & 0xFFFF),
					ho->GetFixedValue(), ho->get_Number());

				return false;
			}
			LOGD(_T("OnForeachFunction(\"%s\") on event %i. Fall through. Continuing...\n"),
				name, DarkEdif::GetCurrentFusionEventNum(this));
		}

		LOGD(_T("OnForeachFunction(\"%s\") on event %i. Expected FV is %i, Number = %i, Oi %i; equals passed FV is %i, number %i. Continuing...\n"),
			name, DarkEdif::GetCurrentFusionEventNum(this),
			lowest.currentForeachObjFV, (lowest.currentForeachObjFV & 0xFFFF),
			ho->get_Oi(), ho->GetFixedValue(), ho->get_Number());

		// TODO: we may not need to select none to cancel this event.
		// However, it does seem that in some OR scenarios, not deselecting everything causes the event to proceed
		// as if all objects matched the selection.
		if (!lowest.currentIterationTriggering)
			Runtime.ObjectSelection.SelectNone(conditionExptOi);
		else
			Runtime.ObjectSelection.SelectOneObject(*ro);
	}
	else // condition has qualifier
	{
		// Qualifier matches actionExptOi, or actionExptOi is a singular;
		// if actExptOi was not singular and mismatched, the earlier if would've killed it
		// We need to check the foreach's Oi is within the condition's qualifier Oi list
		bool found = false;

		// For each object in qualifier
		for (auto poil : DarkEdif::QualifierOIListIterator(rhPtr, conditionExptOi, DarkEdif::Selection::All))
		{
			poil->SelectNone(rhPtr);

			if (found)
				continue;

			if (ho->get_Oi() == poil->get_Oi())
			{
				if (lowest.currentIterationTriggering)
					Runtime.ObjectSelection.SelectOneObject(*ro);
				found = true;

				LOGD(_T("Found OI match: ro FV %i, ro OI %hi, its poil %p, name %s, matches poil OI %hi, %p, name %s.\n"),
					ho->GetFixedValue(), ho->get_Oi(), ho->get_OiList(), ho->get_OiList()->get_name(),
					poil->get_Oi(), poil, poil->get_name());

				// don't break loop, we want to select none for all objects in qualifier,
				// or it'll keep the ones we haven't selected none for as implicitly all selected
			}
			else
			{
				LOGD(_T("Haven't found OI match yet: ro FV %i, ro OI %hi, its poil %p, name %s, does not match poil OI %hi, %p, name %s.\n"),
					ho->GetFixedValue(), ho->get_Oi(), ho->get_OiList(), ho->get_OiList()->get_name(),
					poil->get_Oi(), poil, poil->get_name());
			}
		}

		if (!found)
		{
			LOGD(_T("OnForeachFunction(\"%s\") on event %i. Couldn't find FV %i (OI %hi) in qualifier %hi's OI list. Exiting.\n"),
				name, DarkEdif::GetCurrentFusionEventNum(this), lowest.currentForeachObjFV, ho->get_Oi(), conditionExptOi);
			return false;
		}
	}

#if (DARKEDIF_LOG_MIN_LEVEL <= DARKEDIF_LOG_DEBUG)
	std::size_t numSel = Runtime.ObjectSelection.GetNumberOfSelected(actionExptOi);

	LOGD(_T("OnForeachFunction(\"%s\") on event %i. Current FV = %d; number %i. Event count %d. Checking selection count: %zu. Triggering.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.currentForeachObjFV, lowest.currentForeachObjFV & 0xFFFF, rhPtr->GetRH2EventCount(), numSel);
#endif

	//Runtime.ObjectSelection.SelectOneObject(ro);
	lowest.eventWasHandled = true;
	return lowest.currentIterationTriggering;
}
bool Extension::OnFunctionAborted(const TCHAR* name)
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGD(_T("OnFunctionAborted(\"%s\") on event %i. Self aware = 0. Exiting.\n"), name, DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}
	assert(!globals->runningFuncs.empty() && !globals->runningFuncs.back()->active);
	std::tstring nameR(ToLower(name));
	auto& lowest = *globals->runningFuncs.back();
	LOGD(_T("OnFunctionAborted(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.funcTemplate->name.c_str(), lowest.currentIterationTriggering ? 1 : 0);
	// We can assume the function has aborted if this is running.
	if (lowest.funcTemplate->nameL != nameR)
		return false;
	lowest.abortWasHandled = true;
	return true;
}
bool Extension::IsRunningFuncStillActive()
{
	const auto rf = Sub_GetRunningFunc(_T(__FUNCTION__), _T(""));
	return rf && rf->currentIterationTriggering;
}
bool Extension::DoesFunctionHaveTemplate(const TCHAR* name)
{
	const std::tstring nameR(ToLower(name));
	return std::any_of(globals->functionTemplates.cbegin(), globals->functionTemplates.cend(),
		[&](const auto& f) { return f->nameL == nameR; });
}
bool Extension::IsFunctionInCallStack(const TCHAR* name)
{
	const std::tstring nameR(ToLower(name));
	return std::any_of(globals->runningFuncs.crbegin(), globals->runningFuncs.crend(),
		[&](const auto& f) { return f->funcTemplate->nameL == nameR; });
}
bool Extension::IsLastRepeatOfFunction(const TCHAR* name)
{
	const auto rf = Sub_GetRunningFunc(_T(__FUNCTION__), name);
	return rf && (rf->index >= rf->numRepeats || !rf->nextRepeatIterationTriggering);
}


bool Extension::Logging_OnAnyFunction()
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGD(_T("Logging_OnAnyFunction() on event %i. Self aware = 0. Exiting.\n"), DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}

	LOGD(_T("Logging_OnAnyFunction() on event %i. Self aware = 1. Triggering.\n"), DarkEdif::GetCurrentFusionEventNum(this));
	return true;
}

bool Extension::Logging_OnAnyFunctionCompletedOK()
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGD(_T("Logging_OnAnyFunctionCompletedOK() on event %i. Self aware = 0. Exiting.\n"), DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}

	LOGD(_T("Logging_OnAnyFunctionCompletedOK() on event %i. Self aware = 1. Triggering.\n"), DarkEdif::GetCurrentFusionEventNum(this));
	return true;
}
bool Extension::Logging_OnAnyFunctionAborted()
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGD(_T("Logging_OnAnyFunctionAborted() on event %i. Self aware = 0. Exiting.\n"), DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}

	LOGD(_T("Logging_OnAnyFunctionAborted() on event %i. Self aware = 1. Triggering.\n"), DarkEdif::GetCurrentFusionEventNum(this));
	return true;
}
