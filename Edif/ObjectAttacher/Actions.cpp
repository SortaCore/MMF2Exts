
#include "Common.h"

void Extension::AttachObjects(RunObject* Object)
{
	/*	This action will be called for each object instance automatically.
		All we need to do is add the pointer to the object to our array!	*/
	AttachedObjects.push_back(Object);
}