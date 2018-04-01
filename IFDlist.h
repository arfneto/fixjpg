#pragma once
#include <inttypes.h>
#include <list>
#include "stdafx.h"

using namespace std;

struct IFDfield
{
	uint16_t	tag;
	uint16_t	type;
	uint32_t	count;
	uint32_t	offset;
	IFDfield *	next;
};

struct IFD
{
	uint32_t			address;
	uint16_t			entries;
	IFDfield *			fieldList;
	short				index;
	uint32_t			nextIFDoffset;
	bool				processed;

	IFD *				next;

}; class IFDlist
{
public:

	uint32_t	offsetBase;
	short		ifdCount;
	IFD *		ifd;

	IFDlist();
	~IFDlist();

	void insertField(IFDfield *, IFD *);

	IFD * insertIFD( IFD * i, IFDlist * s );	// insert an IFD at the end of the list
};

