// fixjpg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <memory>
#include <inttypes.h>

using namespace std;




//
// IFD data array is a 12-byte are
//
// byte		|	value
//	0-1		|	Tag
//	2-3		|	type
//	4-7		|	count
//	8-11	|	value offset
//

void displayIFD(
	const char * buf,
	const int len,
	const int start,
	const int n, 
	const int t, 
	const long address,
	const bool lsbFirst
)
{
	const char *	ifd = buf + start;
	unsigned short	tiffTag{};
	unsigned short	tiffType{};
	unsigned long	tiffCount{};
	unsigned long	tiffOffset{};
	char *	pField{};

	// buffer 12 bytes
	//printf("\n\tIFD data, 12 bytes\n");
	//for (int i = 0; i < 12;)
	//{
	//	printf("%02" PRIX8 " ", *(buf + start + i));
	//	i += 1;
	//	if (i % 10 == 5)
	//	{
	//		printf("    ");
	//	}
	//	else
	//	{
	//		if ((i % 10 == 0) && (i != 0))
	//		{
	//			printf("\n");
	//		}
	//	}
	//}
	//printf("\n");
	// now explains the data
	//
	// for now only 4949 LSB first
	printf(
		"\n\tIFD field array %d of %d. HEX Address is 0x%X\n",
		n, t, address
	);
	memcpy(&tiffTag, ifd, 2);
	printf("\tTAG is 0x%X\n", tiffTag);
	memcpy(&tiffType, ifd + 2, 2);
	printf("\tType is 0x%X\n", tiffType);
	memcpy(&tiffCount, ifd + 4, 4);
	printf("\tCount is 0x%X\n", tiffCount);
	memcpy(&tiffOffset, ifd + 8, 4);
	printf("\tOffset is 0x%X\n", tiffOffset);
	switch (tiffTag)
	{
	// 0131H 
	case 0x131:
		printf("\t0131H Software\n");
		if (tiffType != 2)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		pField = new char[tiffCount];
		for (unsigned int i = 0; i < tiffCount; i++)
		{
			*(pField + i) = *(buf + tiffOffset + 6 + i);
		}
		printf("\tSoftware [%s]\n", pField);
		delete pField;
		break;
	case 0x132:
		printf("\t0132H DateTime\n");
		if (tiffType != 2)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		pField = new char[tiffCount];
		for (unsigned int i = 0; i < tiffCount; i++)
		{
			*(pField + i) = *(buf + tiffOffset + 6 + i);
		}
		printf("\tDateTime [%s]\n", pField);
		delete pField;
		break;
	case 0x13B:
		printf("\t013BH Artist\n");
		if (tiffType != 2)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		pField = new char[tiffCount];
		for (unsigned int i = 0; i < tiffCount; i++)
		{
			*(pField + i) = *(buf + tiffOffset + 6 + i);
		}
		printf("\tArtist [%s]\n", pField);
		delete pField;
		break;
	case 0x8769:
		printf("\t8769H Exif specific IFD\n");
		printf("\tExif IFD address is 0x%X\n", tiffOffset);
		break;
	default:
		printf("\t0x%X not treated by now", tiffTag);
		break;
	}	// end switch(tiffTag)
	return;
}



//
// Exif is
//		bytes		|	Value
//		0-3			|	"Exif"
//		4			|	null
//		5			|	null (padding)
//		TIFF HEADER
//		6-7			|	4949.H or 4D4D.H II or MM
//					|	little endian LSB first II
//					|	big endian MSB first MM
//		8-9			|	002A.H fixed
//		10-13		|	IFD0 offset,
//					|		00 00 00 08.H if follows header
//					|			immediately
//
void dumpAPP1(char * buf, int len, long address)
{
	char 		id[6]{};
	uint16_t	dWord{};
	uint32_t	qWord{};
	long		addressTIFF{};		// hex addrsss of TIFF start in Exif
	int			baseTIFF{};			// starting postion of TIFF file in buf
	bool		lsbFirst = false;
	short		fieldCount{};
	int			l = len;

	printf("\tdumpAPP1: data start at 0x%X\n", address);

	if (*(buf + 4) != 0)
	{
		return;
	}
	printf("\n\tAPP1 record size is %d\n", l);
	memcpy(id, buf, 4);
	printf("\tAPP1 Marker is \"%s\"\n", id);
	if (strcmp(id, "Exif"))
	{
		return;
	}
	// Exif
	printf("\tExif marker found: TIFF file starts at 0x%X\n", address + 6);
	printf("\tProcessing TIFF File Header\n");
	memcpy(&dWord, buf + 6, 2);
	printf("\tByte Order: 0x%04X\n", dWord);
	if (dWord = 0x4949)
	{
		lsbFirst = true;
		printf("\tLSB fisrt: little endian\n");
	}
	else
	{
		lsbFirst = false;
		printf("\tMSB fisrt: big endian\n");
	}
	memcpy(&dWord, buf + 8, 2);
	printf("\tTIFF Marker: 0x%04X\n", dWord);
	memcpy(&qWord, buf + 10, 4);
	printf("\tIFD0 offset is: 0x%08X or %d\n\n", qWord, qWord);

	addressTIFF = address + 6 + qWord;
	printf("\tTIFF address is: 0x%08X\n", addressTIFF);
	printf("\tdifference is: %d\n", addressTIFF-address);
	baseTIFF = 6 + qWord;	// length is len - 6
	if (lsbFirst)
	{
		memcpy(&fieldCount, (buf + baseTIFF), 2);
	}
	else
	{
		fieldCount = *(buf + baseTIFF) * 256 +
			*(buf + baseTIFF + 1);
	}
	printf("\tIFD0 field count is: 0x%04X or %d\n", fieldCount, fieldCount);
	int startIFD = baseTIFF+2;
	for (int p = 0; p < fieldCount; p++)
	{
		displayIFD(
			buf,
			len-6, 
			(startIFD + (12*p)),
			p+1,
			fieldCount, 
			addressTIFF+2+(12*p),
			lsbFirst
		);
	}
	//
	// dump first 80 bytes
	//
	if (l > 80)
	{
		l = 80;
	}
	printf("\n\tAPP1 data, first %d bytes\n\n", l);
	for (int i = 0; i < l;)
	{
		printf("%02" PRIX8 " ", *(buf + i));
		i += 1;
		if (i % 10 == 5)
		{
			printf("    ");
		}
		else
		{
			if ((i % 10 == 0) && (i != 0))
			{
				printf("\n");
			}
		}
	}
	printf("\n");
	//
	// dump last 20 bytes
	//
	if (l > 80)
	{
		l = 80;
	}
	printf("\n\tAPP1 data, last %d bytes. End at 0x%X\n\n", l, address+len-2-1);
	for (int i = 0; i < l;)
	{
		printf("%02" PRIX8 " ", (unsigned)*(buf + (len - l) - 2 + i));
		i += 1;
		if (i % 10 == 5)
		{
			printf("    ");
		}
		else
		{
			if ((i % 10 == 0) && (i != 0))
			{
				printf("\n");
			}
		}
	}
	printf("\n");
}	// end dumpAPP1()





void dumpAPP0(char * buf, int l, long address)
{
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

	JFIF *		pJFIF{};
	pJFIF = (JFIF *)buf;
	int	x = pJFIF->xDensityL + pJFIF->xDensityM * 256;
	int	y = pJFIF->yDensityL + pJFIF->yDensityM * 256;
	printf("\n\tAPP0 address is 0x%X\n", address);
	printf(
		"\tVersion is %d.%02d\n",
		(short)pJFIF->majorVersion, (short)pJFIF->minorVersion
	);
	switch (pJFIF->units)
	{
	case 0:
		printf(
			"\tdensity is %dx%d pixels\n", x,y
		);
		break;
	case 1:
		printf(
			"\tdensity is %dx%d dots per inch\n", x, y
		);
		break;
	case 2:
		printf(
			"\tdensity is %dx%d dots per cm", x, y
		);
		break;
	}	//	end switch
	printf("\t%dx%d thumbnail\n",
		pJFIF->xThumbnail,
		pJFIF->yThumbnail
	);
	return;
}





int main(
	int argc,
	char ** argv)
{
	ifstream	jpgFile;
	string		filename;
	string		APPn_marker = "";
	uint16_t	marker = 0;
	int			len = 0;
	long		total = 0;
	long		segmentStart{ 0 };
	short		st = 0;
	int			toRead = 0;
	int			nAPP = 0;
	bool		firstByte = true;
	bool		jpgEnd = false;
	bool		id = false;
	bool		inDataArea = false;
	int			ign = 0;
	char *		buffer{ nullptr };
	int			pbuf{ 0 };

	if (argc < 2)
	{
		return 1;
	}
	std::cout << "\n" << argv[0] << ": file is \"" << argv[1] << "\"\n" << endl;;
	filename = argv[1];
	jpgFile.open(filename, ios::in | ios::binary);
	cout << argv[1] << " is now open" << endl;;
	if(!jpgFile)
	{
		cout << "could not open " << argv[1] << " in desired mode" << endl;;
		return 2;
	}





	while ((!jpgFile.eof()) && (!jpgEnd))
	{
		jpgFile.read((char *)&marker, 1);
		total += 1;
		switch (st)
		{
		case 0:
			// start of cycle: must read FF
			nAPP = -1;
			if (marker == 0xff)
			{
				st = 1;
			}
			else
			{
				if (!inDataArea)
				{
					ign += 1;
				}
			}
			break;
		case 1:
			segmentStart = total - 2;
			// post FF
			// printf("post FF, read %02x\n", marker);
			if (marker != 0)
			{
				// a new header
				inDataArea = false;
			}
			switch (marker)
			{
			case 0:
				// it seems to be an escape sequence into the
				// data segments to follow every FF with 00
				// just go on
				// printf("\tFF00 at 0x%X skipped\n", total - 2);
				st = 2;
				break;
			case 0xC0:
				printf("\tFFC0 start at 0x%X\n", segmentStart);
				st = 3;
				break;
			case 0xC4:
				printf("\tFFC4 start at 0x%X\n", segmentStart);
				st = 3;
				break;
			case 0xD8:
				printf("\tSOI image start at 0x%X\n", segmentStart);
				st = 0;
				break;
			case 0xD9:
				cout << "\tEOI End of Image marker at 0xFFD9" << endl;
				printf("\tEOI image start at 0x%X\n", segmentStart);
				jpgEnd = true;
				break;
			case 0xDA:
				printf("\tFFDA start at 0x%X\n", segmentStart);
				inDataArea = true;
				st = 2;
				break;
			case 0xDB:
				printf("\tFFDB start at 0x%X\n", segmentStart);
				st = 3;
				break;
			case 0xE0:
				// APP0
				printf("\tAPP0 start at 0x%X\n", segmentStart);
				nAPP = 0;
				st = 3;
				break;
			case 0xE1:
				// APP1
				printf("\n\tAPP1 start at 0x%X\n", segmentStart);
				nAPP = 1;
				st = 3;
				break;
			case 0xED:
				printf("\tFFED start at 0x%X\n", segmentStart);
				st = 3;
				break;
			default:
				printf("\tUNKNOWN segment type FF%02X at 0x%X\n", marker, segmentStart);
				st = 0;
				firstByte = false;
				break;
			}	// end switch marker
			break;
		case 2:
			// skipping possibly compressed data
			if (marker == 0xff)
			{
				// if it is FF00 just go, else end of segment
				st = 1;
			}
			break;
		case 3:
			// first byte of length
			len = 256 * marker;
			st = 4;
			//printf("First Byte of Header is %02X\n", marker);
			break;
		case 4:
			// second byte of length
			len = len + marker;
			toRead = len - 2;
			//printf("Second Byte of Header is %02X\n", marker);
			switch (nAPP)
			{
			case 0:
				// APP0 is a 16 bytes area, including 2 bytes for length 
				printf("\tAPP0 segment lentgh is %d bytes\t[%d ignored so far]\n", len, ign);
				toRead = len - 2;
				buffer = new char[len];	// could be len - 2
				pbuf = 0;	// index at buffer
				st = 5;
				break;
			case 1:
				// APP1
				printf("\tAPP1 segment lentgh is %d (0x%04X) bytes\t[%d ignored so far]\n", len, len, ign);
				toRead = len - 2;
				buffer = new char[len-2];	// could be len - 2
				pbuf = 0;	// index at buffer
				printf("\tBuffer[%d] allocated for data starting at 0x%X\n", len-2, segmentStart+4);
				st = 6;
				break;
			default:
				// fixed length segment, not APP0, not APP1
				printf("\tSegment lentgh is %d (0x%04X) bytes.\t[%d ignored so far]\n", len, len, ign);
				toRead = len - 2;
				buffer = new char[len];	// could be len - 2
				pbuf = 0;	// index at buffer
				st = 7;		// another segment
				break;
			}	// end switch(nAPP)
			APPn_marker = "";
			id = false;
			break;
		case 5:
			// reading segment APP0: toRead bytes left
			buffer[pbuf] = char(marker);
			pbuf += 1;
			toRead -= 1;
			if (!id)
			{
				if (marker != 0)
				{
					APPn_marker = APPn_marker + char(marker);
				}
				else
				{
					id = true;
				}
			}
			if (toRead == 0)
			{
				st = 0;
				if (id)
				{
					cout << "\tAPP0 marker is " << APPn_marker << endl;
					APPn_marker = "";
				}
				dumpAPP0(buffer, len-2, segmentStart);
				delete buffer;
				pbuf = 0;
			}
			break;

		case 6:
			// reading APP1 segment
			if (!id)
			{
				if (marker != 0)
				{
					APPn_marker = APPn_marker + char(marker);
				}
				else
				{
					id = true;
				}
			}
			buffer[pbuf] = char(marker);
			pbuf += 1;
			toRead -= 1;
			if (toRead == 0)
			{
				//  full segment read
				st = 0;
				firstByte = true;
				cout << "\tAPP1 marker is " << APPn_marker << flush;
				printf(", %d bytes in buffer\n", pbuf);

				dumpAPP1(buffer, len, segmentStart+4);

				delete buffer;
				pbuf = 0;
			}
			break;
		case 7:
			buffer[pbuf] = char(marker);
			pbuf += 1;
			toRead -= 1;
			if (toRead == 0)
			{
				//  full segment read
				st = 0;
				firstByte = true;
				delete buffer;
				pbuf = 0;
			}
			break;
		default:
			// ok
			break;
		}	// end switch(st)
	}	// end while
	cout << "\n\n\t" << total << " bytes read from " << argv[1] << endl;
	cout << "\tIgnored " << ign << " from " << argv[1] << endl;
	jpgFile.close();
	return 0;
}

