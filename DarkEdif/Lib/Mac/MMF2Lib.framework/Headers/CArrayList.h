/* Copyright (c) 1996-2014 Clickteam
*
* This source code is part of the iOS exporter for Clickteam Multimedia Fusion 2
* and Clickteam Fusion 2.5.
*
* Permission is hereby granted to any person obtaining a legal copy
* of Clickteam Multimedia Fusion 2 or Clickteam Fusion 2.5 to use or modify this source
* code for debugging, optimizing, or customizing applications created with
* Clickteam Multimedia Fusion 2 and/or Clickteam Fusion 2.5.
* Any other use of this source code is prohibited.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------------
//
// CARRAYLIST : classe extensible de stockage
//
//----------------------------------------------------------------------------------

#import <Foundation/Foundation.h>

#define GROWTH_STEP 5

@interface CArrayList : NSObject
{
	void** pArray;
	NSUInteger length;
	NSUInteger numberOfEntries;
}
-(id)init;
-(void)dealloc;
-(void)getArray:(NSUInteger)max;
-(void)ensureCapacity:(NSUInteger)max;
-(NSUInteger)add:(void*)o;
-(NSUInteger)addInt:(int)o;
-(void)addIndex:(NSUInteger)index object:(void*)o;
-(void)addIndex:(NSUInteger)index integer:(int)o;
-(void*)get:(NSUInteger)index;
-(int)getInt:(NSUInteger)index;
-(void)set:(NSUInteger)index object:(void*)o;
-(void)set:(NSUInteger)index integer:(int)o;
-(void)setAtGrow:(NSUInteger)index object:(void*)o;
-(void)setAtGrow:(NSUInteger)index integer:(int)o;
-(void)removeIndex:(NSUInteger)index;
-(void)removeIndexFree:(NSUInteger)index;
-(NSInteger)indexOf:(void*)o;
-(NSInteger)indexOfInt:(int)o;
-(void)removeObject:(void*)o;
-(void)removeInt:(NSInteger)o;
-(void)removeClearIndex:(NSUInteger)index;
-(int)size;
-(void)clear;
-(void)clearRelease;
-(void)removeIndexRelease:(NSUInteger)n;
-(void)freeRelease;
-(void)removeObjectRelease:(void*)o;

-(NSInteger)findString:(NSString*)string startingAt:(NSUInteger)startIndex;
-(NSInteger)findStringExact:(NSString*)string startingAt:(NSUInteger)startIndex;

-(void)sortCListItems;
-(NSMutableArray*)getNSArray;

@end





@interface CListItem : NSObject
{
@public
	NSString* string;
	int	data;
}
-(id)initWithString:(NSString*)s andData:(int)d;
-(void)dealloc;
@end







#define CCGROWTH_STEP 10
class CCArrayList
{
	int numberOfEntries;
	int length;
	void** pArray;

public:
	CCArrayList()
	{
		numberOfEntries=0;
		length=0;
		pArray=nil;
	}
	~CCArrayList()
	{
		if (pArray!=nil)
		{
			free(pArray);
		}
	}
	void GetArray(int max)
	{
		if (pArray==nil)
		{
			pArray=(void**)malloc((max+CCGROWTH_STEP)*sizeof(void*));
			length=max+CCGROWTH_STEP;
		}
		else if (max>=length)
		{
			pArray=(void**)realloc(pArray, (max+CCGROWTH_STEP)*sizeof(void*));
			length=max+CCGROWTH_STEP;
		}
	}
	void EnsureCapacity(int max)
	{
		GetArray(max);
	}
	void Add(void* o)
	{
		GetArray(numberOfEntries);
		pArray[numberOfEntries++]=o;
	}
	void AddIndex(int index, void* o)
	{
		GetArray(numberOfEntries);
		if (index<numberOfEntries)
		{
			int n;
			for (n=numberOfEntries; n>index; n--)
			{
				pArray[n]=pArray[n-1];
			}
		}
		pArray[index]=o;
		numberOfEntries++;
	}
	void Swap(void* o1, void* o2)
	{
		int n1, n2;
		for (n1=0; n1<numberOfEntries; n1++)
			if (pArray[n1]==o1)
				break;
		for (n2=0; n2<numberOfEntries; n2++)
			if (pArray[n2]==o2)
				break;

		if (n1==numberOfEntries)
			return;
		if (n2==numberOfEntries)
		{
			RemoveIndex(n2);
			Add(o2);
		}
		else
		{
			void* temp=pArray[n1];
			pArray[n1]=pArray[n2];
			pArray[n2]=temp;
		}
	}
	void* Get(int index)
	{
		if (index<length)
		{
			return pArray[index];
		}
		return nil;
	}
	void Set(int index, void* o)
	{
		if (index<length)
		{
			pArray[index]=o;
		}
	}
	void RemoveIndex(int index)
	{
		if (index<length && numberOfEntries>0)
		{
			int n;
			for (n=index; n<numberOfEntries-1; n++)
			{
				pArray[n]=pArray[n+1];
			}
			numberOfEntries--;
			pArray[numberOfEntries]=nil;
		}
	}
	void RemoveIndexFree(int index)
	{
		void* o=Get(index);
		if (o!=nil)
		{
			free(pArray[index]);
		}
		if (index<length && numberOfEntries>0)
		{
			RemoveIndex(index);
		}
	}
	int IndexOf(void* o)
	{
		int n;
		for (n=0; n<numberOfEntries; n++)
		{
			if (pArray[n]==o)
			{
				return n;
			}
		}
		return -1;
	}
	void RemoveObject(void* o)
	{
		int n=IndexOf(o);
		if (n>=0)
		{
			RemoveIndex(n);
		}
	}
	int Size()
	{
		return numberOfEntries;
	}
	void Clear()
	{
		numberOfEntries=0;
	}
	void ClearFree()
	{
		int n;
		for (n=0; n<numberOfEntries; n++)
		{
			if (pArray[n]!=nil)
			{
				free(pArray[n]);
				pArray[n]=nil;
			}
		}
		numberOfEntries=0;
	}
	void Swap(int index1, int index2)
	{
		if (index1<length && index2<length)
		{
			void* temp=pArray[index1];
			pArray[index1]=pArray[index2];
			pArray[index2]=temp;
		}
	}
	void AddInt(int o)
	{
		GetArray(numberOfEntries);
		pArray[numberOfEntries++]=(void*)(signed long)o;
	}
	int GetInt(int index)
	{
		return (int)(signed long)Get(index);
	}
};
