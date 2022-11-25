
#include "Common.h"

bool Extension::AlwaysTrue() const
{
	return true;
}
bool Extension::LoopNameMatch(const TCHAR * loopName) const
{
	return !_tcsicmp(curLoopName.c_str(), loopName);
}
bool Extension::OnFunction(const TCHAR * name)
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGV(_T("OnFunction(\"%s\" %p) on event %i. Self aware = 0. Exiting.\n"), name, name, DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}
	assert(!globals->runningFuncs.empty());

	const std::tstring nameR(ToLower(name));
	auto & lowest = *globals->runningFuncs.back();
	LOGV(_T("OnFunction(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.funcTemplate->name.c_str(), lowest.currentIterationTriggering ? 1 : 0);

	if (lowest.funcTemplate->nameL != nameR || !lowest.currentIterationTriggering)
		return false;
	if (lowest.keepObjectSelection)
		evt_RestoreSelectedObjects(lowest.selectedObjects, true);
	lowest.eventWasHandled = true;
	return true;
}
bool Extension::OnForeachFunction(const TCHAR* name, HeaderObject *obj)
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGV(_T("OnForeachFunction(\"%s\") on event %i. Self aware = 0. Exiting.\n"), name, DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}
	assert(!globals->runningFuncs.empty());
	std::tstring nameR(ToLower(name));
	auto& lowest = *globals->runningFuncs.back();
	LOGV(_T("OnForeachFunction(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.funcTemplate->name.c_str(), lowest.currentIterationTriggering ? 1 : 0);

	if (lowest.funcTemplate->nameL != nameR || !lowest.currentIterationTriggering)
		return false;

	// Since it's a foreach
	RunObject* ro = Runtime.RunObjPtrFromFixed(lowest.currentForeachObjFV);
	assert(ro != nullptr);
	const short expectedOi = lowest.currentForeachOil;

	// obj should be RunObject *, with ParamObject * being exclusively used as event editor's internal structs and not passed to ext.
	// Note ScopeControl expects RunObject but won't read it, instead relying on CurrentParam.
	EventParam* evp = rdPtr->rHo.CurrentParam; // this is param 0, name...
	evp = (EventParam *)((std::uint8_t *)evp + evp->size); // but we want param 1, obj
	const short oil = evp->evp.W[0];
	const objInfoList * poil;

	// Oi doesn't match, so the condition's selected object does not match the Foreach action.
	// And, the loop is either not being run on a qualifier, or the singular-on-qualifier property isn't enabled.
	if (oil != expectedOi && (expectedOi >= 0 || !allowForeachSingular))
	{
		LOGV(_T("OnForeachFunction(\"%s\") on event %i. Oi %i does not match expected Oi %i.\n"),
			name, DarkEdif::GetCurrentFusionEventNum(this), oil, expectedOi);
		// If we just return false, it crashes.
		// We have to deselect all the qualifier entries, then return false.
		qualToOi* pqoi = (qualToOi*)((LPBYTE)rhPtr->QualToOiList + (expectedOi & 0x7FFF));
		while (pqoi->OiList >= 0)
		{
			Runtime.ObjectSelection.SelectNone(pqoi->OiList);
			pqoi = (qualToOi*)((std::uint8_t*)pqoi + 4);
		}

		return false;
	}

	// Condition selected a singular object
	if (oil >= 0)
	{
		// It doesn't match, so we can assume expectedOi is a qualifier
		if (oil != expectedOi)
		{
			poil = (const objInfoList*)(((char*)rhPtr->OiList) + Runtime.ObjectSelection.oiListItemSize * oil);
			if (poil->Oi != ro->roHo.Oi) //(obj->Number != (lowest.currentForeachObjFV & 0xFFFF))
			{
				LOGV(_T("OnForeachFunction(\"%s\") on event %i. Expected FV is %i, Number = %i; does not equal passed FV is %i, number %i. Exiting.\n"),
					name, DarkEdif::GetCurrentFusionEventNum(this),
					lowest.currentForeachObjFV, (lowest.currentForeachObjFV & 0xFFFF),
					(obj->CreationId << 16) + obj->Number, obj->Number);

				return false;
			}
		}

		// TODO: we may not need to select none to cancel this event.
		// However, it does seem that in some scenarios, not deselecting everything causes the event to proceed
		// as if all objects matched the selection.
		if (!lowest.currentIterationTriggering)
			Runtime.ObjectSelection.SelectNone(oil);
		else
			Runtime.ObjectSelection.SelectOneObject(ro);
	}
	else // we're on qualifier, with qualifier hack enabled
	{
		qualToOi * pqoi = (qualToOi*)((LPBYTE)rhPtr->QualToOiList + (oil & 0x7FFF));
		bool found = false;
		// For each object in qualifier
		while (pqoi->OiList >= 0)
		{
			poil = (objInfoList*)(((char*)rhPtr->OiList) + Runtime.ObjectSelection.oiListItemSize * pqoi->OiList);
			Runtime.ObjectSelection.SelectNone(pqoi->OiList);
			if (!found && ro->roHo.OiList == poil)
			{
				if (lowest.currentIterationTriggering)
					Runtime.ObjectSelection.SelectOneObject(ro);
				found = true;
				// don't break, we want to select none for all objects in qualifier,
				// or it'll keep the ones we haven't selected none for with all selected and run the event
			}
			// Next object in qualifier
			pqoi = (qualToOi *)((std::uint8_t *)pqoi + 4);
		}

		if (!found)
		{
			LOGV(_T("OnForeachFunction(\"%s\") on event %i. Couldn't find FV %i in qualifier. Exiting.\n"),
				name, DarkEdif::GetCurrentFusionEventNum(this), lowest.currentForeachObjFV);
			return false;
		}
	}

	LOGV(_T("OnForeachFunction(\"%s\") on event %i. Current FV = %d. Event count %d. Triggering.\n"),
		name, DarkEdif::GetCurrentFusionEventNum(this), lowest.currentForeachObjFV, rhPtr->rh2.EventCount);

	// Pre-restore any objects selected, if needed
	if (lowest.keepObjectSelection)
		assert(false);
		//evt_RestoreSelectedObjects(lowest.selectedObjects, true);

	//Runtime.ObjectSelection.SelectOneObject(ro);
	lowest.eventWasHandled = true;
	return lowest.currentIterationTriggering;
}
bool Extension::OnFunctionAborted(const TCHAR* name)
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGV(_T("OnFunctionAborted(\"%s\") on event %i. Self aware = 0. Exiting.\n"), name, DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}
	assert(!globals->runningFuncs.empty() && !globals->runningFuncs.back()->active);
	std::tstring nameR(ToLower(name));
	auto& lowest = *globals->runningFuncs.back();
	LOGV(_T("OnFunctionAborted(\"%s\") on event %i. Self aware = 1. Current function: \"%s\". Current iteration = %i.\n"),
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


bool Extension::Logging_OnAnyFunction()
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGV(_T("Logging_OnAnyFunction() on event %i. Self aware = 0. Exiting.\n"), DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}

	LOGV(_T("Logging_OnAnyFunction() on event %i. Self aware = 1. Triggering.\n"), DarkEdif::GetCurrentFusionEventNum(this));
	return true;
}

bool Extension::Logging_OnAnyFunctionCompletedOK()
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGV(_T("Logging_OnAnyFunctionCompletedOK() on event %i. Self aware = 0. Exiting.\n"), DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}

	LOGV(_T("Logging_OnAnyFunctionCompletedOK() on event %i. Self aware = 1. Triggering.\n"), DarkEdif::GetCurrentFusionEventNum(this));
	return true;
}
bool Extension::Logging_OnAnyFunctionAborted()
{
	// We're not aware
	if (!selfAwareness)
	{
		LOGV(_T("Logging_OnAnyFunctionAborted() on event %i. Self aware = 0. Exiting.\n"), DarkEdif::GetCurrentFusionEventNum(this));
		return false;
	}

	LOGV(_T("Logging_OnAnyFunctionAborted() on event %i. Self aware = 1. Triggering.\n"), DarkEdif::GetCurrentFusionEventNum(this));
	return true;
}
