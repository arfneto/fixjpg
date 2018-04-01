#include "stdafx.h"
#include "IFDlist.h"


IFDlist::IFDlist()
{
	//struct IFD
	//{
	//	uint32_t			address;
	//	uint16_t			entries;
	//	IFDfield *			fieldList;
	//	short				index;
	//	uint32_t			nextIFDoffset;
	//	bool				processed;

	//	IFD *				next;
	//};

	offsetBase = -1;
	ifdCount = 0;
	ifd = new(IFD);

	ifd->address = -1;
	ifd->entries = 0;
	ifd->fieldList = nullptr;
	ifd->index = -1;
	ifd->nextIFDoffset = -1;
	ifd->processed = false;
	ifd->next = nullptr;
}


IFDlist::~IFDlist()
{
}





void IFDlist::insertField(
	IFDfield * f,
	IFD * pIFD
)
{
	// insert a field *f into field array *pIFD
	f->next = nullptr;	// just to be sure
	if ( pIFD->fieldList == nullptr )
	{
		// first field
		printf(
			"\tinsertField: first field inserted\n"
		);
		pIFD->fieldList = f;
		pIFD->entries = 1;
		return;
	}
	// points to head
	IFDfield * p = pIFD->fieldList;
	// navigate to end
	while (p->next != nullptr)
	{
		p = p->next;
	};
	// now insert
	p->next = f;
	pIFD->entries += 1;
	printf(
		"\tinsertIFD: %dth item inserted\n",
		pIFD->entries
	);
	return;
};	// end insertIFD()





IFD * IFDlist::insertIFD(
	IFD * f,
	IFDlist * s
)
{
	f->fieldList = nullptr;

	if (s->ifd == nullptr)
	{
		// first ifd
		s->ifdCount = 1;
		s->ifd = f;
		return f;
	}
	// point p to head
	IFD * p = s->ifd;
	while (p->next != nullptr)
	{
		p = p->next;
	};
	// now points to end
	p->next = f;
	s->ifdCount += 1;
	return p;
};	// end insertIFD()
