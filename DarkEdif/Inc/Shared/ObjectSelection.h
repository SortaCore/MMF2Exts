
// By Anders Riggelsen (Andos)
// http://www.clickteam.com/epicenter/ubbthreads.php?ubb=showflat&Number=214148&gonew=1

// These files do not match up with the originals - modified for Edif (callbacks inside the extension class, etc..)

#ifndef OBJECTSELECTION
#define OBJECTSELECTION

#ifndef EF_ISHWA
	#define EF_ISHWA		112
#endif
#ifndef EF_ISUNICODE
	#define EF_ISUNICODE	113
#endif

namespace Riggs
{
    class ObjectSelection  
    {
    public:

        Extension * pExtension;

	    ObjectSelection(RunHeader * rhPtr);

	    void SelectAll(short Oi);
	    void SelectNone(short Oi);
	    void SelectOneObject(RunObject * object);
	    void SelectObjects(short Oi, RunObject ** objects, int count);
	    bool ObjectIsOfType(RunObject * object, short Oi);
	    int GetNumberOfSelected(short Oi);
    	
	    template<class T> bool FilterObjects(short Oi, bool negate, T filterFunction)
        {
	        if (Oi & 0x8000)
		        return FilterQualifierObjects(Oi & 0x7FFF, negate, filterFunction) ^ negate;
	        else
		        return FilterNonQualifierObjects(Oi, negate, filterFunction) ^ negate;
        }

    protected:

	    RunHeader * rhPtr;
	    objectsList * ObjectList;
	    objInfoList	* OiList;
	    qualToOi * QualToOiList;
	    int oiListItemSize;

	    objInfoList * GetOILFromOI(short Oi);

        template<class T> bool DoCallback(void * Class, T Function, RunObject * Parameter)
        {
            T _Function = Function;
#ifdef _WIN32
            void * FunctionPointer = *(void **) &_Function;
#endif

            long Result;

#ifdef _WIN32
            __asm
            {
                pushad

                mov ecx, Class

                push Parameter
                    call FunctionPointer
                add esp, 4

                mov Result, eax

                popad
            };
#elif defined(__ANDROID__)
			// Never run
			/*
			__asm__("pushad				\n\
				mov %%ecx, %[Class]		\n\
				push %[Parameter]		\n\
				call %[FunctionPointer]	\n\
				add %%esp, 4			\n\
				mov %[Result], %%eax	\n\
				popad"
			: [Result] "=r"(Result)
			: [Class] "r" (Class),
			[Parameter] "m" (Parameter),
			[FunctionPointer] "m" (FunctionPointer));*/
#endif

            return (*(char *) &Result) != 0;
        }

	    template<class T> bool FilterQualifierObjects(short Oi, bool negate, T filterFunction)
        {
	        qualToOi * CurrentQualToOiStart = (qualToOi *)((char*)QualToOiList + Oi);
	        qualToOi * CurrentQualToOi = CurrentQualToOiStart;

	        bool hasSelected = false;
	        while(CurrentQualToOi->OiList >= 0)
	        {
		        objInfoList * CurrentOi = GetOILFromOI(CurrentQualToOi->OiList);
		        hasSelected |= FilterNonQualifierObjects(CurrentOi->Oi, negate, filterFunction);
		        CurrentQualToOi = (qualToOi *)((char*)CurrentQualToOi + 4);
	        }
	        return hasSelected;
        }

	    template<class T> bool FilterNonQualifierObjects(short Oi, bool negate, T filterFunction)
        {
	        objInfoList * pObjectInfo = GetOILFromOI(Oi);
	        bool hasSelected = false;

	        if (pObjectInfo->EventCount != rhPtr->GetEventCount())
		        SelectAll(Oi);	//The SOL is invalid, must reset.

	        //If SOL is empty
	        if (pObjectInfo->NumOfSelected <= 0)
		        return false;

	        int firstSelected = -1;
	        int count = 0;
	        int current = pObjectInfo->ListSelected;
	        HeaderObject * previous = NULL;

	        while(current >= 0)
	        {
				HeaderObject * pObject = ObjectList[current].oblOffset;
		        bool useObject = DoCallback((void *) pExtension, filterFunction, (RunObject *)pObject);

                if (negate)
                    useObject = !useObject;

		        hasSelected |= useObject;

		        if (useObject)
		        {
			        if (firstSelected == -1)
				        firstSelected = current;

			        if (previous != NULL)
				        previous->NextSelected = current;
        			
			        previous = pObject;
			        count++;
		        }
		        current = pObject->NextSelected;
	        }
	        if (previous != NULL)
		        previous->NextSelected = -1;

	        pObjectInfo->ListSelected = firstSelected;
	        pObjectInfo->NumOfSelected = count;

	        return hasSelected;
        }
    };
}

#endif // !defined(OBJECTSELECTION)