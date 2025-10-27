//
//  ArrayList.h
//
//  Created by Anders Riggelsen on 6/21/13.
//  Copyright (c) 2013 Clickteam. All rights reserved.
//

#pragma once
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include <new>

#define ARRAY_SSO_SIZE	8

	template <class T> class Array
	{
	private:
		T* elements;			//Points to "ssoBuffer" if capacity<=8, to external buffer otherwise
		size_t m_capacity;
		size_t numElements;
		char ssoBuffer[sizeof(T) * ARRAY_SSO_SIZE];
		void expandIfNecessaryForInsertion();
		void shrinkIfNecessary();

	public:

        /* Taken from List to allow for C++11 begin()/end() functions
         */
        class Iterator
        {
            Array * array;
            size_t i;

        public:

            friend class Array;

            inline Iterator (Array * _array, size_t _i)
                : array(_array), i(_i)
            {
            }

            inline Iterator ()
                : array(NULL), i(-1)
            {
            }

            inline T &operator * ()
            {
                return array->elements[i];
            }

            inline T &operator -> ()
            {
                return array->elements[i];
            }

            inline Iterator &operator ++ ()
            {
                ++ i;
                return *this;
            }

            inline Iterator &operator -- ()
            {
                -- i;
                return *this;
            }

            inline Iterator next ()
            {
                return Iterator (array, i + 1);
            }

            inline Iterator prev ()
            {
                return Iterator (array, i - 1);
            }

            inline Iterator &operator = (const Iterator &rhs)
            {
                array = rhs.array;
                i = rhs.i;

                return *this;
            }

            inline bool operator == (const Iterator &rhs)
            {
                return array == rhs.array && i == rhs.i;
            }

            inline bool operator != (const Iterator &rhs)
            {
                return array != rhs.array || i != rhs.i;
            }
        };

        /* begin: returns an Iterator at the beginning of the Array
         */
        inline Iterator begin ()
        {
            return Iterator (this, 0);
        }

        /* end: returns an Iterator one element after the end of the Array
         */
        inline Iterator end ()
        {
            return Iterator (this, numElements);
        }

		Array();
        Array(const Array &);
		Array(size_t capacity);
        ~ Array();

        Array &operator = (const Array &);

		size_t length() const;						//Returns the length of the list (number of items in it)
		size_t capacity() const;					//Returns the current capacity of the list
		void ensureCapacity(size_t capacity);		//Expands the list to contain space for at least <capacity> elements
		void growAndFill(T &var, size_t count);		//Fills the array to a maximum of 'count' copies of 'var'.
		void crop(size_t startIndex, size_t size);	//Crops the array to the given size from the given start index
		void crop();								//Crops the array to use only the memory needed to contain the elements in the list
		size_t push(const T &var);					//Insert at the end of the list and return the index
		size_t insert(const T &var, size_t index);		//Insert at the given index
		void set(const T &var, size_t index);				//Sets the object at the given index
		void setExpand(const T &var, size_t index);		//Set the object at the given index, expanding the array if nessecary to contain the object
		void deleteIndex(size_t index);				//Deletes a given indexed node in the list
		void swap(size_t indexA, size_t indexB);				//Swaps the elements at the two indexes
		void deleteElement(const T &var);					//Deletes a node with the given data

		void clear();								//Clears the entire array
		void clearRetainingCapacity();				//Clears the entire array but keeps the memory for later allocation

		bool contains(T &var);

		T &get(size_t index) const;					//Returns a given indexed element in the list
		T pop();

		void sort(int (*)(const T &a, const T &b));

		T& operator[] (const size_t index) const;


        /* TODO optimize
         */
        template<class D> inline Array<D> convert(D (* convertProc)(const T &))
        {
            Array<D> arr;
            arr.ensureCapacity(numElements);

            for(auto &elem : *this)
                arr.push(convertProc(elem));

            return arr;
        }

        template<class D> inline Array<D> convert()
        {
            Array<D> arr;
            arr.ensureCapacity(numElements);

            for(auto &elem : *this)
                arr.push((D) elem);

            return arr;
        }
	};

template <class T> Array<T>::Array()
{
	this->m_capacity = ARRAY_SSO_SIZE;
	this->numElements = 0;
	memset(ssoBuffer, 0, sizeof(T)*ARRAY_SSO_SIZE);
	this->elements = (T*)ssoBuffer;
}

template <class T> Array<T>::Array(size_t capacity)
{
	this->numElements = 0;
	memset(ssoBuffer, 0, sizeof(T)*ARRAY_SSO_SIZE);

	if(capacity > ARRAY_SSO_SIZE)
	{
		this->elements = (T*)calloc(capacity, sizeof(T));
		this->m_capacity = capacity;
	}
	else
	{
		this->elements = (T*)ssoBuffer;
		this->m_capacity = ARRAY_SSO_SIZE;
	}
}

template <class T> Array<T>::Array(const Array &rhs)
    : Array(rhs.m_capacity)
{
    numElements = rhs.length();

    for(unsigned int i = 0; i < numElements; ++ i)
        new (elements + i) T (rhs.elements[i]);
}

template <class T> Array<T> &Array<T>::operator =(const Array &rhs)
{
    this->~ Array();
    new (this) Array (rhs);

    return *this;
}

template <class T> T& Array<T>::operator[] (const size_t index) const
{
	assert(index >= 0);
	assert(index < numElements);
	return elements[index];
}

template <class T> size_t Array<T>::length() const
{
	return numElements;
}

template <class T> size_t Array<T>::capacity() const
{
	return m_capacity;
}

template <class T> void Array<T>::ensureCapacity(size_t capacity)
{
	if(m_capacity >= capacity || capacity < ARRAY_SSO_SIZE)
		return;

	//Is the internal storage too small? Allocate new memory and clear the old

    //TODO this will invalidate pointers.  either make it clear that you shouldn't
    //have a pointer to an object allocated as part of an Array, or delete and
    //re-construct (via copy constructor) all elements here.

	if(elements == (void*)&ssoBuffer)
	{
		if(m_capacity <= ARRAY_SSO_SIZE && capacity > ARRAY_SSO_SIZE)
		{
			/* SSO -> heap buffer
			 */
			elements = (T*)calloc(capacity, sizeof(T));
			memcpy(elements, ssoBuffer, numElements * sizeof(T));
		}
	}
	else
	{
		/* Grow heap buffer
		 */
		size_t extraspace = capacity - m_capacity;
		elements = (T*)realloc(elements, capacity*sizeof(T));
		memset(&this->elements[m_capacity], 0, extraspace*sizeof(T));
	}
	m_capacity = capacity;
}

template <class T> void Array<T>::growAndFill(T &var, size_t count)
{
	//If the array already has the elements necessary
	if(numElements >= count)
		return;

	ensureCapacity(count);
	size_t numExtra = count - numElements;
	for(size_t i=0; i<numExtra; ++i)
		push(var);

    assert(numElements == count);
}

template <class T> void Array<T>::expandIfNecessaryForInsertion()
{
	if(m_capacity == numElements)
	{
		size_t newSize = MIN(m_capacity*2, m_capacity+1024);	//Limit the size expansion to 1024 elements
		ensureCapacity(newSize);
	}
}

template <class T> void Array<T>::shrinkIfNecessary()
{
	//Shrink if the usage is only 1/4 of the capacity (don't waste memory) (limits to the SSO size)
	size_t oneFourth = m_capacity/4;
	if(m_capacity > ARRAY_SSO_SIZE && numElements < oneFourth)
	{
		if(oneFourth < ARRAY_SSO_SIZE)
		{
			memcpy(ssoBuffer, elements, oneFourth*sizeof(T));
			free(elements);
			elements = (T*)ssoBuffer;
			m_capacity = ARRAY_SSO_SIZE;
		}
		else
		{
			elements = (T*)realloc(elements, oneFourth*sizeof(T));
			m_capacity = oneFourth;
		}
	}
}

template <class T> void Array<T>::crop(size_t startIndex, size_t size)
{
	//If none of the original elements will be in the new list
	if(startIndex > numElements)
	{
		if(size == numElements)
		{
			//Identical to clearing the array but preserving the capacity
			memset(elements, 0, numElements*sizeof(T));
		}
		else
		{
			if(size > ARRAY_SSO_SIZE)
			{
				free(elements);
				elements = (T*)calloc(size, sizeof(T));
			}
			else
				elements = (T*)ssoBuffer;
		}
		return;
	}

	//Move the entire array from startIndex back to the beginning of the array
	if(startIndex != 0)
		memmove(&elements[0], &elements[startIndex], size*sizeof(T));

	//Resize
	if(size > ARRAY_SSO_SIZE)
	{
		if(m_capacity > ARRAY_SSO_SIZE)
			elements = (T*)realloc(elements, size*sizeof(T));
		else
		{
			elements = (T*)calloc(size, sizeof(T));
			memcpy(elements, ssoBuffer, sizeof(T)*m_capacity);
		}
	}

	//Zero out the new elements if the new size is larger than the old one
	size_t oldLastIndex = (size_t)MAX((int)numElements-(int)startIndex, 0);
	size_t numNewElements = (size_t)MAX((int)size-(int)numElements, 0) + startIndex;

	if(numNewElements > 0)
		memset(&elements[oldLastIndex], 0, numNewElements*sizeof(T));

	m_capacity = size;
	numElements = size;
}

template <class T> void Array<T>::crop()
{
	crop(0, numElements);
}

template <class T> size_t Array<T>::push(const T &var)
{
	expandIfNecessaryForInsertion();
	new (&elements[numElements]) T (var);
	return numElements++;
}

template <class T> size_t Array<T>::insert(const T &var, size_t index)
{
	expandIfNecessaryForInsertion();

	if(index < numElements)
		memmove(&elements[index], &elements[index+1], (numElements-index)*sizeof(T));	//Move the rest of the items.  TODO invalidates pointers
	else
		return add(var);	//insert at the end of the list out of bounds

    new (&elements[index]) T (var);
	return index;
}

template <class T> void Array<T>::set(const T &var, size_t index)
{
    (elements + index)->~ T();
	new (elements + index) T (var);
}

template <class T> void Array<T>::setExpand(T const &var, size_t index)
{
    /* TODO may need to destruct old value */

	ensureCapacity(index+1);
	numElements = MAX(numElements, index+1);
	new (elements + index) T (var);
}

template <class T> T &Array<T>::get(size_t index) const
{
	assert(index < numElements);
	return elements[index];
}

template <class T> T Array<T>::pop()
{
    assert(numElements > 0);

	T ret(elements[numElements-1]);
	(elements + numElements - 1)->~ T();

    -- numElements;

	shrinkIfNecessary();
	return ret;
}

template <class T> void Array<T>::swap(size_t indexA, size_t indexB)
{
	T c(elements[indexA]);
	elements[indexA]=elements[indexB];
	elements[indexB]=c;
}

template <class T> void Array<T>::deleteIndex(size_t index)
{
    /* TODO call destructor
     */

	if(index < numElements-1)
		memmove(&elements[index], &elements[index+1], (numElements-index-1)*sizeof(T));
	//(elements + numElements - 1)->~ T();
	--numElements;
	shrinkIfNecessary();
}

template <class T> void Array<T>::deleteElement(const T &var)
{
	for(size_t i=0; i<numElements; ++i)
	{
		if(elements[i] == var)
		{
			deleteIndex(i);
			return;
		}
	}
}

template <class T> bool Array<T>::contains(T &var)
{
	for(size_t i=0; i<numElements; ++i)
	{
		if(elements[i] == var)
			return true;
	}
	return false;
}

template <class T> void Array<T>::clear()
{
	for (size_t i = 0; i < numElements; ++i)
        (elements + i)->~ T();

	numElements = 0;
	if(m_capacity > ARRAY_SSO_SIZE)
	{
		free(elements);
		elements = (T*)ssoBuffer;
	}
	m_capacity = ARRAY_SSO_SIZE;
}

template <class T> void Array<T>::clearRetainingCapacity()
{
	for (size_t i = 0; i < numElements; ++i)
        (elements + i)->~ T();

	numElements = 0;
	memset(elements, 0, sizeof(T)*m_capacity);
}

template <class T> Array<T>::~Array()
{
	if(m_capacity > ARRAY_SSO_SIZE)
		free(elements);
	numElements = 0;
	m_capacity = 0;
	elements = NULL;
}

template <class T> void Array<T>::sort(int (*)(const T &a, const T &b))
{



}

