// fixjpg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TIFFfile.h"

using namespace std;

void dumpTo(
	const char *,
	const short,
	const unsigned char *
);

void processIFD1(
	const unsigned int bufferStart,
	const unsigned char * buf,
	const unsigned int len,
	const unsigned int offset);

void processInterop(
	const unsigned int bufferStart,
	const unsigned char * buf,
	const unsigned int len,
	const unsigned int offset);





void displayIFD(
	const int bufferStart,
	const unsigned char * buf,
	const int len,
	const int start,
	const int n, 
	const int t, 
	const bool lsbFirst
)
{
	//
	// IFD data array is a 12-byte are
	//
	// byte		|	value
	//	0-1		|	Tag
	//	2-3		|	type
	//	4-7		|	count
	//	8-11	|	value offset
	//

	const unsigned char *	ifd = buf + start;
	unsigned short	tiffTag{};
	unsigned short	tiffType{};
	unsigned long	tiffCount{};
	unsigned long	tiffOffset{};
	char *			pField{};
	long			base{};
	
	//dumpTo("IFD0 field: ", 40, (buf+start));
	//printf(
	//	"\n\tdisplayIFD: field array: field %d of %d. HEX Address is 0x%X\n",
	//	n, t, (bufferStart + start)
	//);
	//printf("\t  Buffer start: %d (0x%X)\n", bufferStart, bufferStart);
	//printf("\t  len: %d\n", len);
	//printf("\t  start: %d\n", start);
	//
	// hex is start + header + 2 bytes field count +
	// 12 bytes per field. n is the field number starting
	// at 1
	//
	memcpy(&tiffTag, ifd, 2);
	//printf("\tTAG is 0x%X\n", tiffTag);
	memcpy(&tiffType, ifd + 2, 2);
	//printf("\tType is 0x%X\n", tiffType);
	memcpy(&tiffCount, ifd + 4, 4);
	//printf("\tCount is 0x%X\n", tiffCount);
	memcpy(&tiffOffset, ifd + 8, 4);
	//printf("\tOffset is 0x%X\n", tiffOffset);

	switch (tiffTag)
	{
	case 0x0002:
		printf("\t0002H Interop Version\n");
		if (tiffType != 7)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		pField = new char[5];
		pField[4] = 0;
		memcpy(pField, &tiffOffset, tiffCount);
		printf("\n\tExif Interop Version is [%c%c.%c%c]\n",
			pField[0],
			pField[1], 
			pField[2], 
			pField[3] );
		delete pField;
		break;

	case 0x0100:
		printf("\t0100H ImageWidth\n");
		if (tiffType != 4)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		printf("\tImage Width %d (0x%X)\n",
			tiffOffset, tiffOffset);
		break;

	case 0x0101:
		printf("\t0101H ImageWidth\n");
		if (tiffType != 4)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		printf("\tImage Width %d (0x%X)\n",
			tiffOffset, tiffOffset);
		break;

	case 0x10F:
		printf("\t10FH Make\n");
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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
		}
		printf("\tMake [%s]\n", pField);
		delete pField;
		break;

	case 0x110:
		printf("\t110H Model\n");
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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
		}
		printf("\tModel [%s]\n", pField);
		delete pField;
		break;

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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
		}
		printf("\tArtist [%s]\n", pField);
		delete pField;
		break;

	case 0x1001:
		printf("\t1001H RelatedImageWidth\n");
		if (tiffType != 4)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		printf("\tRelated Image Width %d (0x%X)\n",
			tiffOffset, tiffOffset);
		break;

	case 0x1002:
		printf("\t1002H RelatedImageHeight\n");
		if (tiffType != 4)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		printf("\tRelated Image Height %d (0x%X)\n",
			tiffOffset, tiffOffset);
		break;

	case 0x8769:
		printf("\n\t8769H Exif specific IFD\n");
		printf("\tTIFF start is 0x%X\n", bufferStart);
		printf("\tExif IFD1 offset is 0x%X\n", tiffOffset);
		printf(
			"\tExif IFD1 starts at 0x%X in disk\n",
			(bufferStart + tiffOffset)
		);
		//
		processIFD1(
			bufferStart,
			buf,
			len,
			(tiffOffset)
		);
		//
		break;

	case 0x9000:
		printf("\t9000H Exif Version\n");
		if (tiffType != 7)
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		// 0900 type is undefined: version MUST be 0220 in the
		//	tiffOffset position
		pField = new char[5];
		pField[4] = 0;
		memcpy(pField, &tiffOffset, tiffCount);
		printf("\n\tExif Version is [%s]\n", pField);
		delete pField;
		break;

	case 0x9003:
		printf("\t9003H DateTimeOriginal\n");
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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
		}
		printf("\tDate and time of original data generation [%s]\n", pField);
		delete pField;
		break;

	case 0xA002:
		printf("\tA002H PixelXDimension\n");
		if (tiffType == 3)
		{
			// short
			printf("\tValid Image Width %d (0x%X)\n",
				tiffOffset, tiffOffset);
		}
		else
		{
			if (tiffType == 4)
			{
				// long
				printf("\tValid Image Width %d (0x%X)\n",
					tiffOffset, tiffOffset);
			}
			else
			{
				printf("\tWrong type %d for 0x%X Tag\n\t",
					tiffType,
					tiffTag);
				break;
			}
		}
		break;

	case 0xA003:
		printf("\tA003H PixelYDimension\n");
		if (tiffType == 3)
		{
			// short
			printf("\tValid Image Height %d (0x%X)\n",
				tiffOffset, tiffOffset);
		}
		else
		{
			if (tiffType == 4)
			{
				// long
				printf("\tValid Image Height %d (0x%X)\n",
					tiffOffset, tiffOffset);
			}
			else
			{
				printf("\tWrong type %d for 0x%X Tag\n\t",
					tiffType,
					tiffTag);
				break;
			}
		}
		break;

	case 0xA005:
		printf("\tA005H Interoperability IFD Pointer\n");
		if (tiffType == 4)
		{
			printf("\tInteroperability IFD Pointer (0x%X)\n",
				tiffOffset);
			processInterop(
				bufferStart,
				buf,
				len,
				tiffOffset);
		}
		else
		{
			printf("\tWrong type %d for 0x%X Tag\n\t",
				tiffType,
				tiffTag);
			break;
		}
		break;

	case 0xA420:
		printf("\tA0420H ImageUniqueID\n");
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
			*(pField + i) = *(buf + 6 + tiffOffset + i);
		}
		printf("\tUnique Image ID [%s]\n", pField);
		delete pField;
		break;

	default:
		printf("\t0x%X not treated by now\n", tiffTag);
		break;
	}	// end switch(tiffTag)
	return;
}	// end displayIFD()






void dumpAPP0(const unsigned char * buf, int l, long address)
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





void dumpAPP1(
	const int bufferStart,
	const unsigned char * buf,
	const int len)
{
	
	char 		id[6]{};
	uint16_t	dWord{};
	uint32_t	qWord{};
	uint32_t	offset{};
	bool		lsbFirst = false;
	int			TIFFStart{};
	int			TIFFLen{};
	int			IFD0Start{};
	short		fieldCount{};
	int			l = len;

	printf("\tdumpAPP1: data (%d bytes) start at 0x%X\n", len, bufferStart);
	if (*(buf + 4) != 0)
	{
		return;
	}
	printf("\n\tAPP1 record size is %d\n", len);
	memcpy(id, buf, 4);
	printf("\tAPP1 Marker is \"%s\"\n", id);
	//
	// by now only treating Exif APP1 data
	//
	if (strcmp(id, "Exif"))
	{
		printf("\tAPP1 record \"%s\" skipped\n", id);
		return;
	}
	// skip 6 bytes: Exif\0\0
	printf("\tExif marker found\n");
	TIFFStart = bufferStart + 6;
	TIFFLen = 6;
	printf("\tTIFF file (in disk) starts at 0x%X\n", TIFFStart);
	printf("\tTIFF file (in buffer) starts at %d\n", TIFFLen);

	//
	// process TIFF Image File Header
	//
	memcpy(&dWord, buf+6, 2);
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
		return;
	}
	memcpy(&dWord, buf + 8, 2);
	printf("\tTIFF Marker: 0x%04X\n", dWord);
	memcpy(&offset, buf + 10, 4);
	printf("\tIFD0 offset is: 0x%08X or %d\n", offset, offset);
	IFD0Start = TIFFStart + offset;
	printf("\tIFD0 address is: 0x%08X\n", IFD0Start);
	//dumpTo("IFD0", 40, buf + 14);
	memcpy(&fieldCount, buf + 14, 2);
	printf("\tHex address in file [0x%X]\n", TIFFStart+8);
	printf("\tIFD0 field count is: 0x%04X or %d\n", fieldCount, fieldCount);

	int startIFD = 16;
	for (int p = 0; p < fieldCount; p++)
	{
		displayIFD(
			TIFFStart,
			buf,
			len,
			(startIFD + (12 * p)),
			p + 1,
			fieldCount,
			true
		);
	}
}	// end dumpAPP1()


 


void dumpTo(
	const char *	title,
	const short		l,
	const unsigned char * buf
)
{
	//
	// dump first len bytes
	//
	printf("\n\t%s [first %d bytes]\n", title, l);
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
}	// end dumpFrom()





void processIFD1(
	const unsigned int bufferStart,
	const unsigned char * buf,
	const unsigned int len,
	const unsigned int offset
)
{
	short		fieldCount{};
	int			l = len;

	printf("\n\tprocessIFD1:\n\tstart %d (0x%X)\n", bufferStart, bufferStart);
	printf(  "\tlength %d (0x%X)\n", len, len);
	printf(  "\tIFD1 offset is %d (0x%X)\n", offset, offset);

	printf("\tIFD1 address (in disk) is %d (0x%X)\n", offset+bufferStart, offset+bufferStart);

	dumpTo("IFD1", 30, buf+offset+6);
	printf("\t  Hex address in file [0x%X]\n", bufferStart+offset);
	memcpy(&fieldCount, buf+offset+6, 2);
	printf("\tIFD1 field count is: 0x%04X or %d\n", fieldCount, fieldCount);
	int startIFD = offset+8;
	for (int p = 0; p < fieldCount; p++)
	{
		displayIFD(
			bufferStart,
			buf,
			len,
			(startIFD + (12 * p)),
			p + 1,
			fieldCount,
			true
		);
	}
};





void processInterop(
	const unsigned int bufferStart,
	const unsigned char * buf,
	const unsigned int len,
	const unsigned int offset)
{
	//
	short		fieldCount{};
	int			l = len;

	printf("\n\tprocessInterop:\n\tstart %d (0x%X)\n", bufferStart, bufferStart);
	printf("\tlength %d (0x%X)\n", len, len);
	printf("\tInteroperability IFD offset is %d (0x%X)\n", offset, offset);
	printf("\tInteroperability IFD address (in disk) is %d (0x%X)\n", offset + bufferStart, offset + bufferStart);
	dumpTo("Interoperability IFD", 40, buf + offset + 6);

	memcpy(&fieldCount, buf + offset + 6, 2);
	printf("\tIFD1 field count is: 0x%04X or %d\n", fieldCount, fieldCount);
	int startIFD = offset + 8;
	for (int p = 0; p < fieldCount; p++)
	{
		displayIFD(
			bufferStart,
			buf,
			len,
			(startIFD + (12 * p)),
			p + 1,
			fieldCount,
			true
		);
	}
	return;
}	// end processInterop()





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
	unsigned char *		buffer{ nullptr };
	int			pbuf{ 0 };
	int			bufferStart{};


	if (argc < 2)
	{
		return 1;
	}
	std::cout << "\n" << argv[0] << ": file is \"" << argv[1] << "\"\n" << endl;;
	filename = argv[1];

	TIFFfile	work(filename);

	work.fileStatus();
	work.fileLoad();
	work.fileStatus();
	work.dumpIFDset(&work.ifdSet);

	exit(0);

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
				printf("\n\n\tAPP1 start at 0x%X\n", segmentStart);
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
				buffer = new unsigned char[len];	// could be len - 2
				pbuf = 0;	// index at buffer
				st = 5;
				break;
			case 1:
				// APP1
				printf("\tAPP1 segment lentgh is %d (0x%04X) bytes  [%d ignored so far]\n", len, len, ign);
				toRead = len - 2;
				buffer = new unsigned char[toRead];	// could be len - 2
				pbuf = 0;	// index at buffer
				bufferStart = segmentStart + 4;
				printf("\t%d-bytes Buffer allocated for data starting at 0x%X\n", len-2, bufferStart);
				st = 6;
				break;
			default:
				// fixed length segment, not APP0, not APP1
				printf("\tSegment lentgh is %d (0x%04X) bytes.\t[%d ignored so far]\n", len, len, ign);
				toRead = len - 2;
				buffer = new unsigned char[len];	// could be len - 2
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
				//  full segment was read
				st = 0;
				firstByte = true;
				cout << "\tAPP1 marker is " << APPn_marker << endl;
				printf("\tBuffer is loaded, %d bytes\n", pbuf);
				dumpAPP1(
					bufferStart,
					buffer, 
					pbuf);
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

