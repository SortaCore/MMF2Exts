
// By Anders Riggelsen (Andos)
// http://www.clickteam.com/epicenter/ubbthreads.php?ubb=showflat&Number=214148&gonew=1

// These files do not match up with the originals - modified for Edif (callbacks inside the extension class, etc..)

#include "Edif.hpp"
#include <type_traits>

#ifndef OBJECTSELECTION
#define OBJECTSELECTION

#ifndef EF_ISHWA
	#define EF_ISHWA		112
#endif
#ifndef EF_ISUNICODE
	#define EF_ISUNICODE	113
#endif
namespace Edif {
	class Runtime;
}
namespace DarkEdif {
	class ObjectIterator;
	class QualifierOIListIterator;
	class AllObjectIterator;
	class AllOIListIterator;

	class ObjectSelection
	{
	public:

		Extension * pExtension;

		ObjectSelection(RunHeader * rhPtr);

		void SelectAll(short Oi) const;
		void SelectNone(short oiList) const;
		void SelectNone(RunObject& object) const;
		void SelectOneObject(RunObject& object) const;
		void SelectObjects(short Oi, RunObjectMultiPlatPtr * objects, std::size_t count) const;
		// Checks if the passed object is part of qualifier or singular Oi
		bool ObjectIsOfType(RunObject& object, short qoil) const;
		// Gets number of explicitly selected for qualifier or singular Oi
		std::size_t GetNumberOfSelected(short Oi) const;

		template<class Ext, class T>
		bool FilterObjects(short Oi, bool negate, T (Ext::*filterFunction))
		{
			if (Oi & 0x8000)
				return FilterQualifierObjects(Oi & 0x7FFF, negate, filterFunction) ^ negate;
			else
				return FilterNonQualifierObjects(Oi, negate, filterFunction) ^ negate;
		}

		objInfoList* GetOILFromOI(short Oi) const;
	protected:

#ifdef _WIN32
		static int oiListItemSize;
#endif
		friend Edif::Runtime;
		friend ObjectIterator;
		friend QualifierOIListIterator;
		friend AllOIListIterator;
		friend AllObjectIterator;
		friend objInfoList;
		friend RunHeader;
		RunHeader * rhPtr = nullptr;
#ifdef _WIN32
		objectsList * ObjectList = nullptr;
		objInfoList * OiList = nullptr;
		qualToOi * QualToOiList = nullptr;
#endif


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

		template<class Ext, class T> bool FilterQualifierObjects(short Oi, bool negate, T (Ext::* filterFunction));

		template<class Ext, class T> bool FilterNonQualifierObjects(short Oi, bool negate, T(Ext::* filterFunction));
	};

	/// @remarks In Active A alt value A = 0 event, Active B actions can be run.
	///			 Based on that, include all B's (implicit), exclude B's (explicit), or select everything (all).
	///			 Most selection use cases will need implicit.
	enum class Selection
	{
		// Selects all
		All,
		// Includes selected explicitly by conditions, includes all other obj instances selected implicitly
		Implicit,
		// Selected explicitly by conditions only, excludes objects that have all obj instances selected implicitly
		Explicit
	};
	// Iterates the object instances in qualifier OI or singular OI
	class ObjectIterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = RunObjectMultiPlatPtr;
		using difference_type = std::ptrdiff_t;
		using pointer = RunObjectMultiPlatPtr*;
		using reference = RunObjectMultiPlatPtr&;

		RunHeader* const rhPtr;
		const short oiList = -1;
		const Selection select;
		const bool includeDestroyed = false;

		short curOiList = -1;
		HeaderObject * curHo = nullptr;
		RunObjectMultiPlatPtr curRo = nullptr;
		objInfoList* oil = nullptr;
		std::vector<short> qualOiList;
		std::size_t qualOiListAt = 0;
		std::size_t numNextRun = 0;

		void GetNext();
		explicit ObjectIterator(RunHeader* rhPtr, int oiList, Selection select, bool destroy, bool);
	public:
		// Iterator for all the object instances in qualifier OI or singular OI
		explicit ObjectIterator(RunHeader* rhPtr, int oiList, Selection selection, bool includeDestroyed = false);
		ObjectIterator& operator++();
		// x++, instead of ++x
		ObjectIterator operator++(int);
		bool operator==(ObjectIterator other) const;
		bool operator!=(ObjectIterator other) const;
		reference operator*();
		friend difference_type operator -(const ObjectIterator& from, const ObjectIterator& to) {
			return from.numNextRun - to.numNextRun;
		}
		ObjectIterator begin() const;
		ObjectIterator end() const;
	};
	// Iterates the OI List in a qualifier OI; it is valid to pass a singular OI or -1
	class QualifierOIListIterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = objInfoList*;
		using difference_type = std::ptrdiff_t;
		using pointer = objInfoList*;
		using reference = objInfoList*;

		RunHeader* const rhPtr;
		const short oiList;
		const Selection select;
		std::vector<short> qualOiList;

		short curOiList = -1;
		objInfoList* oil = nullptr;
		std::size_t qualOiListAt = 0;

		void GetNext();
		QualifierOIListIterator(RunHeader* rhPtr, int oiList, Selection select, bool);
	public:
		// Iterator for all the OI List in a qualifier OI; it is valid to pass a singular OI or -1
		explicit QualifierOIListIterator(RunHeader* rhPtr, int oiList, Selection select);
		QualifierOIListIterator& operator++();
		// x++, instead of ++x
		QualifierOIListIterator operator++(int);
		bool operator==(const QualifierOIListIterator& other) const;
		bool operator!=(const QualifierOIListIterator& other) const;
		reference operator*() const;
		QualifierOIListIterator begin() const;
		QualifierOIListIterator end() const;
	};
	// Iterates all the object instances in entire frame
	class AllObjectIterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = RunObjectMultiPlatPtr;
		using difference_type = std::ptrdiff_t;
		using pointer = RunObjectMultiPlatPtr;
		using reference = RunObjectMultiPlatPtr;

		RunHeader* const rhPtr;
		const std::size_t numObjectsInFrame;

		pointer curRo = nullptr;
		// Index of this object in rhObjectList, nulls in rhObjectList ignored
		std::size_t objListAt = 0;
		// Index of this object in rhObjectList, nulls in rhObjectList included
		std::size_t objListTrueIndex = 0;

		explicit AllObjectIterator(RunHeader* rhPtr, bool);
	public:
		// Iterator for all the object instances in entire frame
		explicit AllObjectIterator(RunHeader* rhPtr);
		AllObjectIterator& operator++();
		// x++, instead of ++x
		AllObjectIterator operator++(int);
		bool operator==(AllObjectIterator other) const;
		bool operator!=(AllObjectIterator other) const;
		reference operator*() const;
		AllObjectIterator begin() const;
		AllObjectIterator end() const;
	};
	// Iterates all the OI List in entire frame
	class AllOIListIterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = objInfoList*;
		using difference_type = std::ptrdiff_t;
		using pointer = objInfoList*;
		using reference = objInfoList*;

		RunHeader* const rhPtr;
		const std::size_t numOI;
		objInfoList* oil = nullptr;
		std::size_t oiListAt = 0;

		explicit AllOIListIterator(RunHeader* rhPtr, bool);

	public:
		// Iterator for all the OI List in entire frame
		explicit AllOIListIterator(RunHeader* rhPtr);
		AllOIListIterator& operator++();
		// x++, instead of ++x
		AllOIListIterator operator++(int);
		bool operator==(AllOIListIterator other) const;
		bool operator!=(AllOIListIterator other) const;
		reference operator*() const;

		AllOIListIterator begin() const;
		AllOIListIterator end() const;
	};
}
#endif // !defined(OBJECTSELECTION)
