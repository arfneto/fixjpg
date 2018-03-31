#pragma once

#include "stdafx.h"
#include <inttypes.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <memory>
#include <inttypes.h>

using namespace std;

#define		__BUFFER0JPEG__		65536





struct fileHeader
{
	uint16_t	byteOrder;
	uint16_t	tiffID;
	uint32_t	offset;
};

struct fileLog
{
	string		header;
	uint32_t	offset;
	uint32_t 	len;
	string		name;
};

struct JFIF
{
	char	identifier[5];
	char	majorVersion;
	char	minorVersion;
	char	units;
	char	xDensityM;
	char	xDensityL;
	char	yDensityM;
	char	yDensityL;
	char	xThumbnail;
	char	yThumbnail;
};

struct EXIF
{
	char		identifier[6];		// EXIF\0\0
	uint16_t	endian;				// 0x4949 little 0x4D4D big
	uint16_t	aa;					// 0x002A 0x2A00	
	uint32_t	offset;				// must be on word boundary

};

struct IFDfield
{
	uint16_t	tag;
	uint16_t	type;
	uint32_t	count;
	uint32_t	offset;
};

struct IFD
{
	uint32_t			address;
	uint16_t			entries;
	uint32_t			nextIFD;
	vector<IFDfield>	ifdEntry;
};

struct IFDVector
{
	short		scanning;
	uint32_t	offsetBase;
	vector<IFD>	ifd;
};




class TIFFfile
{
public:
	string	diskFile;

	TIFFfile(string fileName);
	TIFFfile();

	~TIFFfile();

	void dumpAPP0(int);

	void dumpEXIF(int);

	void dumpFrom(const char * title, const short l, const unsigned char * buf) const;

	void dumpIFDset(IFDVector *);

	void fileLoad();

	void fileStatus();

	void logThis(fileLog);
	void logThis(fileLog l, string comment);

	void processField(
		 IFDfield,
		 uint32_t, 
		 uint32_t, 
		 uint16_t,
		 uint16_t);

private:

	ifstream			jpgFile;
	unsigned char		buffer[__BUFFER0JPEG__];

	uint32_t	next{};
	uint32_t	bufSize;

	long		lastAccessed{ 0 };
	long		highestAccessed{ 0 };

	int			status;
	int			ignored;

	int			total;		// bytes read
	bool		atEof;
	bool		atError;

	short		st;			// state
	uint16_t	marker;		// byte being analyzed
	bool		inDataArea;
	bool		jpgEnd;

	vector<fileLog>		log;	// reader history
	JFIF				jfif;
	EXIF				exif;
	vector<IFD>			ifd;	// all IFDs
	IFDVector			ifdSet;

	bool				lsbFirst;
};

