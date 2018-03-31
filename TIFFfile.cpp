#include "stdafx.h"
#include "TIFFfile.h"

using namespace std;

TIFFfile::TIFFfile() : status(-1) {};

TIFFfile::TIFFfile(
	string	fileName
)
{
	diskFile = fileName;
	jpgFile.open(diskFile, ios::in | ios::binary);
	if (!jpgFile)
	{
		cout << "could not open " << fileName << " in desired mode" << endl;;
		status = -1;
		return;
	}
	jpgFile.read( (char *) buffer, __BUFFER0JPEG__);
	bufSize = (int) jpgFile.gcount();
	printf(
		"\t%d bytes loaded from disk\n",
		bufSize
	);

	next = 0;
	atEof = jpgFile.eof();
	atError = jpgFile.failbit || jpgFile.badbit;
	total = 0;
	inDataArea = false;
	ignored = 0;
	jpgEnd = false;
	st = 0;
	lsbFirst = false;

	ifdSet.scanning = -1;
	ifdSet.offsetBase = -1;

}	// end TIFFfile()





TIFFfile::~TIFFfile()
{
	jpgFile.close();
	cout << diskFile << " is now closed in the destrutor" << endl;;
	status = 1;
}





void TIFFfile::dumpAPP0(int address)
{

	int	x = jfif.xDensityL + jfif.xDensityM * 256;
	int	y = jfif.yDensityL + jfif.yDensityM * 256;
	printf("\n\tAPP0 address is 0x%X\n", address);
	printf("\tIdentifier is %s\n", jfif.identifier);
	printf(
		"\tVersion is %d.%02d\n",
		(short)jfif.majorVersion, (short)jfif.minorVersion
	);
	switch (jfif.units)
	{
	case 0:
		printf(
			"\tdensity is %dx%d pixels\n", x, y
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
		jfif.xThumbnail,
		jfif.yThumbnail
	);
	return;
}





void TIFFfile::dumpEXIF(int address)
{
	printf("\tdumpEXIF: APP1 address is 0x%X\n", address);
	printf("\tIdentifier is %s\n", exif.identifier);
	printf("\tByte Order: 0x%04X\n", exif.endian);
	if (exif.endian = 0x4949)
	{
		lsbFirst = true;
		printf("\tLSB first: little endian\n");
	}
	else
	{
		lsbFirst = false;
		printf("\tMSB first: big endian\n");
		return;
	}
	printf(
		"\tEXIF id is 0x%04X\n",
		(short)exif.aa
	);
	printf(
		"\tIFD0 offset is %d\n",
		(short)exif.offset
	);
	printf(
		"\tIFD0 address in disk is 0x%X\n",
		exif.offset + ifdSet.offsetBase
	);
	return;
}	//	end dumpEXIF()





void TIFFfile::dumpFrom(
	const char *	title,
	const short		l,
	const unsigned char * buf
) const
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





void TIFFfile::dumpIFDset(IFDVector * v)
{
	// dumps all data
	printf(
		"\ndumpIFDset: %d entries, now scanning %d\tOffset is 0x%X\n",
		v->ifd.size(),
		v->scanning,
		v->offsetBase 
		);
	for (unsigned int i = 0;
		i < v->ifd.size();
		i++)
	{
		printf(
			"\n\tIFD%d: %d field (s), address is [0x%X]. Address of next is [0x%X]\n",
			i,
			v->ifd[i].entries,
			v->ifd[i].address,
			v->ifd[i].nextIFD
		);
		for (int j = 0;
			j < v->ifd[i].entries;
			j++)
		{
			printf(
				"\t\t%d:\ttag 0x%X,\ttype 0x%X,\tcount 0x%X,\toffset 0x%X\n",
				j,
				v->ifd[i].ifdEntry[j].tag,
				v->ifd[i].ifdEntry[j].type,
				v->ifd[i].ifdEntry[j].count,
				v->ifd[i].ifdEntry[j].offset
			);
		}	// end for j
	};	// end for i
	return;
}





void TIFFfile::fileLoad()
{
	uint32_t		segmentStart{ 0 };
	uint32_t		len{ 0 };
	uint32_t		toRead{ 0 };
	int				nAPP{ -1 };
	uint16_t		dWord{};
	uint32_t		offset{};
	uint32_t		base{};
	IFDfield		oneField;	// ifd field to build vector
	IFD				oneIFD;		// ifd used to build vector
	char			msg[80];	// field for building messages
	char			sMsg[80];
	char			sValue[80];
	IFDVector		oneIFDstruct;
	uint16_t		nIFD{};
	uint16_t		nEntry{};

	printf("\tFileLoad()\n");
	//dumpFrom("buffer at start", 40, (unsigned char *) buffer);

	next = 0;
	lastAccessed = 0;
	highestAccessed = 0;

	while (next < bufSize)
	{
		marker = buffer[next];
		next += 1;
		switch (st)
		{
		case 0:
			if (marker == 0xff)
			{
				st = 1;
			}
			else
			{
				if (!inDataArea)
				{
					ignored += 1;
				}
			}
			break;

		case 1:
			// post FF. FF00 seems to be an escape sequence when in data area
			segmentStart = next - 2;
			if (marker != 0)
			{
				// a new header
				inDataArea = false;
			}
			// FFXX what is XX?
			nAPP = marker;
			switch(nAPP)
			{
			case 0:
				// it seems to be an escape sequence into the
				// data segments to follow every FF with 00
				// so just go on in this case
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
				lastAccessed = next - 1;
				if (lastAccessed > highestAccessed)
				{
					highestAccessed = lastAccessed;
				};
				logThis(
					{ "[FFD8]", next-2, 2, "SOI - start of image"	}, ""
				);
				st = 0;
				break;
			case 0xD9:
				printf("\tEOI image at 0x%X\n", segmentStart);
				sprintf_s(sMsg, "[FF%X]", nAPP);
				jpgEnd = true;
				logThis(
					{ sMsg, base, (next - base - 2), "EOI - End of Image (or thumbnail)" }, ""
				);
				st = 15;
				break;
			case 0xDA:
				printf("\tFFDA start at 0x%X\n", segmentStart);
				sprintf_s(sMsg, "[FF%X]", nAPP);
				inDataArea = true;
				logThis(
					{ "[FFDA]", segmentStart-2, 0, "SOS - Start of Scan" }, ""
				);
				st = 2;
				break;
			case 0xDB:
				printf("\tFFDB start at 0x%X\n", segmentStart);
				st = 3;
				break;
			case 0xE0:
				// APP0
				printf("\tFFE0: APP0 start at 0x%X\n", segmentStart);
				st = 3;
				break;
			case 0xE1:
				// APP1
				printf("\tFFE1: APP1 start at 0x%X\n", segmentStart);
				st = 3;
				break;
			case 0xED:
				printf("\tFFED start at 0x%X\n", segmentStart);
				st = 3;
				break;
			default:
				printf("\tUNKNOWN segment type FF%02X at 0x%X\n", marker, segmentStart);
				st = 0;
				//firstByte = false;
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
			break;

		case 4:
			// second byte of length
			len = len + marker;
			toRead = len - 2;

			switch (nAPP)
			{
			case 0xC0:	// SOF
				printf("\t[FFC0]\n");
				printf("\t[FF%X] segment lentgh is %d (0x%04X) bytes\n", nAPP, len, len);
				sprintf_s(sMsg, "[FF%X]", nAPP);
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					next += toRead;
					base = next - 4;
					lastAccessed = next - 1;
					if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;	};
					logThis( { sMsg, base, (next - base), "SOF - Start of Frame" }, "" );
					dumpFrom("after FFC0 data", 40, buffer + next);
					// pointing now to first byte of ifd count. must go back one
					// byte before returning to loop
					next -= 1;
					st = 0;
				}
				else
				{
					st = 15;
				}
				break;

			case 0xC4:	// DHT
				printf("\t[FF%X] Define Huffman Table\n", nAPP);
				sprintf_s(sMsg, "[FF%X]", nAPP);
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					next += toRead;
					base = next - 4;
					lastAccessed = next - 1;
					if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;		};
					logThis(
						{ sMsg, base, (next - base), "DHT - Define Huffman Table" }, ""
					);
					// pointing now to first byte of ifd count. must go back one
					// byte before returning to loop
					next -= 1;
					st = 0;
				}
				else
				{
					st = 15;
				}
				break;

			case 0xDA:	// Start of Frame SOF
				//
				// this segment has no defined length except for a 12-byte header
				// just skip all data until FFD9
				printf("\t[FFDA]\n");
				printf("\t[FF%X] header lentgh is %d (0x%04X) bytes\n", nAPP, len, len);
				sprintf_s(sMsg, "[FF%X]", nAPP);
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					next += toRead;
					base = next - 4;
					lastAccessed = next - 1;
					if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;	};
					logThis(
						{ sMsg, base, (next - base), "SOS - Start of Scan" }, ""
					);
					// pointing now to first byte of ifd count. must go back one
					// byte before returning to loop
					next -= 1;
					inDataArea = true;
					st = 2;
				}
				else
				{
					st = 15;
				}
				break;

			case 0xDB:	// FFDB
				printf("\t[FFDB]\n");
				printf("\t[FF%X] segment lentgh is %d (0x%04X) bytes\n", nAPP, len, len);
				sprintf_s(sMsg, "[FF%X]", nAPP);
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					next += toRead;
					base = next - 4;
					lastAccessed = next - 1;
					if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;	};
					logThis(
						{ sMsg, base, (next - base), "DQT - Define Quantization Table" }, ""
					);
					dumpFrom("after FFDB data", 40, buffer + next);
					// pointing now to first byte of ifd count. must go back one
					// byte before returning to loop
					next -= 1;
					st = 0;
				}
				else
				{
					st = 15;
				}
				break;

			case 0xE0:
				// APP0 is a 16 bytes area, including 2 bytes for length 
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					base = next - 4;
					memcpy(&jfif, buffer + next, toRead);

					dumpAPP0(base);

					lastAccessed = base + toRead + 4 - 1;
					if (lastAccessed > highestAccessed)	{ highestAccessed = lastAccessed;};
					logThis(	{ "[FFE0]", base, (toRead+4), "APP0 JFIF Mandatory marker" }, ""	);
					next = next + toRead - 1;
					st = 0;	
				}
				else
				{	// something went wrong. may need to read more data int buffer
					st = 15;
				}
				break;

			case 0xE1:
				// APP1
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					base = next - 4;
					memcpy(&exif, buffer + next, 6);
					next += 6;
					//
					ifdSet.offsetBase = next;	// this is the global offset
					//
					memcpy(&exif.endian, buffer + next, 2);
					next += 2;
					memcpy(&exif.aa, buffer + next, 2);
					next += 2;
					memcpy(&offset, buffer + next, 4);
					next += 4;
					exif.offset = offset;
					dumpFrom("EXIF data", 80, (buffer+base));

					dumpEXIF(base);

					lastAccessed = next - 1;
					if (lastAccessed > highestAccessed)	{ highestAccessed = lastAccessed;	};
					logThis( { "FFE1", base, (next-base), "APP1 EXIF marker" }, ""	);
					//dumpFrom("past EXIF data", 40, buffer + next);
					// pointing now to first byte of ifd count. must go back one
					// byte before returning to loop
					next -= 1;
					// now push this new IFD into set of IFDs
					oneIFD.address = offset + ifdSet.offsetBase;
					oneIFD.entries = 0;
					oneIFD.nextIFD = 0;
					ifdSet.ifd.push_back(oneIFD);
					printf(
						"FFE1: inserted in vector. size now is %d\n",
						ifdSet.ifd.size()
					);
					// this is all for now
					st = 8;
				}
				else
				{
					// something went wrong. may need to read more data int buffer
					st = 15;
				}
				break;

			default:
				// fixed length segment, not APP0, not APP1
				printf("\t[FF%X] segment lentgh is %d (0x%04X) bytes\n", nAPP, len, len);
				sprintf_s( sMsg, "[FF%X]", nAPP);
				toRead = len - 2;
				if ((bufSize - next) > toRead)
				{
					next += toRead;
					base = next - 4;
					lastAccessed = next - 1;
					if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;	};
					logThis(
						{ sMsg, base, (next - base), "fixed length segment" }, ""
					);
					dumpFrom("another segment data", 40, buffer + next);
					// pointing now to first byte of ifd count. must go back one
					// byte before returning to loop
					next -= 1;
					st = 0;
				}
				else
				{
					st = 15;
				}
				break;
			}	// end switch(marker)
			break;
		case 5:
			// FFD9: end game
			return;
			break;

		case 6:
			break;

		case 7:
			break;

		case 8:
			//
			//  one IFD can point to another
			//  one field inside an IFD can
			//  point to another IFD
			//  so we stay in state 8 until 
			//  all IFDs in ifdSet are scanned
			//
			ifdSet.scanning += 1;
			nIFD = ifdSet.scanning;	// number of IFD in use
			printf(
				"\n\n\tstate 8: total of %d IFDs. Now scanning IFD%d\n",
				ifdSet.ifd.size(),
				nIFD);
			memcpy(
				&nEntry,
				buffer + next,
				2
			);
			ifdSet.ifd[nIFD].entries = nEntry;
			printf(
				"\n\n*****  %d entries *****\n",
				ifdSet.ifd[nIFD].entries
			);
			sprintf_s(
				msg,
				"IFD%d",
				nIFD
			);
			dumpFrom(msg, 40, buffer + next);

			base = next;	// base is the starting address
			ifdSet.ifd[nIFD].address = base;
			next += 2;

			// there are nEntry 12-bytes fields
			// loop thru them all and store 
			memcpy(&offset, buffer + base + 2 + (12 * nEntry), 4);
			// offset not zero points to next IFD
			// build another record into ifdSet
			// now push this new IFD into set of IFDs
			if (offset != 0)
			{
				// offset points to next IFD
				ifdSet.ifd[nIFD].nextIFD = offset;
				// now builds new
				oneIFD.address = offset + ifdSet.offsetBase;
				oneIFD.entries = 0;
				oneIFD.nextIFD = 0;
				// now save it
				ifdSet.ifd.push_back(oneIFD);
				printf(
					"\n\n***** inserted next pointed IFD size now is %d ***** \n\n",
					ifdSet.ifd.size()
				);
				// this is all for now
			}
			for (int i = 0; i < nEntry; i++)
			{	// save Directory Entry
				memcpy(&oneField, buffer + next, 12);
				//sprintf_s(msg, "IFD field #%d", i	);
				//dumpFrom(msg, 12, buffer + next);
				ifdSet.ifd[nIFD].ifdEntry.push_back(oneField);
				next += 12;

				processField(
					oneField,
					ifdSet.offsetBase,
					next - 12,
					(uint16_t)i + 1,
					nEntry);

			}	// end for
			next += 4;	// skip offset
			lastAccessed = next - 1;
			if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;	};
			sprintf_s(
				msg,
				"IFD%d",
				nIFD
			);
			logThis({ msg, base, (next - base), "Image File Directory" }, "");
			//dumpFrom("past IFD data", 40, buffer + next);
			// back one byte
			next = next - 1;
			if (offset == 0)
			{
			}
			else
			{
				sprintf_s(sMsg,
					"IFD%d",
					nIFD+1
				);
				dumpFrom(sMsg, 80, buffer + ifdSet.offsetBase + offset);
				sprintf_s(sValue,
					"IFD%d offset is 0x%X {%d}. In disk at %X",
					nIFD+1,
					offset,
					offset,
					offset + ifdSet.offsetBase
				);
				logThis({ sMsg, 0, 0, sValue }, "");
				next = ifdSet.offsetBase + offset - 1;
				dumpIFDset(&ifdSet);
			}
			break;

		case 15:
			break;

		default:
			break;
		}	// end switch
	}	// end while()
}	// end fileLoad()





void TIFFfile::fileStatus(){
	printf(
		"\tFile in disk: %s\n", diskFile.c_str()
	);
	printf(
		"\tbuffer capacity: %d\n", bufSize
	);
	printf(
		"\tPointer at: %d\n", next
	);
	printf(
		"\t# of log entries is %d\n",
		log.size()
	);
	for (vector<fileLog>::iterator it = log.begin(); it != log.end(); it++)
	{
		printf(
			"\t%s[0x%X,0x%X]\t[%d,%d]\t - %s\n",
			it->header.c_str(),
			it->offset,
			it->len,
			it->offset,
			it->len,
			it->name.c_str()
		);
	}
	return;
}	// end fileStatus()

void TIFFfile::logThis(fileLog l)
{
	// just to have control over logging
	// while testing
	log.push_back(l);
}

void TIFFfile::logThis(fileLog l, string comment)
{
	// just to have control over logging
	// while testing
	log.push_back(l);
	if (comment.length() == 0)
	{
		printf(
			"\t[%s]:\t[0x%X,0x%X]\t{%d,%d}]\t\t'%s'\n",
			l.header.c_str(),
			l.offset,
			l.len,
			l.offset,
			l.len,
			l.name.c_str()
		);
	}
	else
	{
		printf(
			"\t\t\t%s ==>\n\t[%s]:\t[0x%X,0x%X]\t{%d,%d}]\t\t'%s'\n",
			comment.c_str(),
			l.header.c_str(),
			l.offset,
			l.len,
			l.offset,
			l.len,
			l.name.c_str()
		);
	}
}





void TIFFfile::processField(
	 IFDfield field,
	 uint32_t globalOffset,
	 uint32_t address,
	 uint16_t n,
	 uint16_t t ) 
{
	char *		pField;
	char		sMsg[80];
	char		sValue[80];
	string		s;
	uint16_t	n16{};
	uint32_t	n32a{};
	uint32_t	n32b{};
	IFD			oneIFD;

	//
	//dumpFrom("from offsetbase", 80, buffer+globalOffset);
	sprintf_s(
		sMsg, "%04X", field.tag
	);
	sprintf_s(
		sValue,
		"[%X] type: [%X] count: [%X], offset: [%X]\t(%d of %d)",
		field.tag,
		field.type,
		field.count,
		field.offset,
		n, t
		);
	s = sValue;
	logThis({ sMsg, 0, 0, sValue }, "");

	// field types are
	// 1 - byte 8 bit unsigned
	// 2 - ascii
	// 3 - unsigned short 
	// 4 - unsigned long
	// 5 - rational a/b longs
	// 6 - signed byte 8 bits
	// 7 - undefined
	// 8 - signed short 16 bits
	// 9 - signed long 32 bits
	// 10- srational two slongs = a/b
	// 11- float 4 bytes
	// 12- float 8 bytes

	switch (field.type)
	{
	case 2:
		// ASCII
		pField = new char[field.count];
		for (unsigned int i = 0; i < field.count; i++)
		{
			*(pField + i) = *(buffer + globalOffset + field.offset + i);
		}
		sprintf_s( sValue, "[2], value is [%s]", pField);
		s = sValue;
		if (field.tag == 0xA420)	// ImageUniqueID seems to have a padding byte
		{
			field.count = field.count + 1;
			//printf("\t0xA420: Extra byte included after image id\n");
		}
		lastAccessed = globalOffset + field.offset + field.count - 1;
		if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed;	};
		logThis( { sMsg, (globalOffset + field.offset), field.count, sValue }, "" );
		delete pField;
		break;

	case 3:
		// unsigned short, 2 bytes
		n16 = field.offset;
		sprintf_s(sValue, "type 3, value is %d [0x%X]", n16, n16);
		logThis( { sMsg, 0, 0, sValue }, "");
		break;

	case 4:
		// LONG
		sprintf_s(sValue,
			"TAG %X type 4 (unsigned long), value is %d [0x%X]",
			field.tag,
			field.offset,
			field.offset
		);
		logThis({ sMsg, 0, 0, sValue },	"");
		if (field.tag == 0x8769)	// Exif IFD
		{
			dumpFrom("Field 8769", 12, buffer + address);
			sprintf_s(
				sValue,
				"TAG %X Exif IFD. offset is 0x%X. Address in disk is 0x%X",
				field.tag,
				field.offset,
				field.offset + globalOffset
			);
			logThis({ sMsg, 0, 0, sValue },	"");
			// now push this new IFD into set of IFDs
			oneIFD.address = ifdSet.offsetBase + field.offset;
			oneIFD.entries = 0;
			oneIFD.nextIFD = 0;
			ifdSet.ifd.push_back(oneIFD);
			// this is all for now
			// now record this to process later
			printf(
				"\n\n***** inserted Exif IFD TAG 8769 size now is %d ***** \n\n",
				ifdSet.ifd.size()
			);
				// this is all for now in ifdSet
		}
		else
		{
			if (field.tag == 0xA005)	// Interoperability IFD
			{
				dumpFrom("Field A005", 12, buffer + address);
				sprintf_s(
					sValue,
					"TAG %X Interop IFD. offset is 0x%X. Address in disk is 0x%X",
					field.tag,
					field.offset,
					field.offset + globalOffset
				);
				logThis({ sMsg, 0, 0, sValue },	"");
				// now record this to process later
				oneIFD.address = ifdSet.offsetBase + field.offset;
				oneIFD.entries = 0;
				oneIFD.nextIFD = 0;
				ifdSet.ifd.push_back(oneIFD);
				printf("\n\n***** inserting Interop IFD TAG A005 ***** \n\n");
				printf(
					"\n\n***** inserted Interop IFD TAG A005 size now is %d ***** \n\n",
					ifdSet.ifd.size()
				);
				// this is all for now in ifdSet
			}
			else
			{
				//
			}
		}
		break;

	case 5:
		// rational: A/B
		memcpy(&n32a, buffer + globalOffset + field.offset, 4);
		memcpy(&n32b, buffer + globalOffset + field.offset+4, 4);
		sprintf_s(
			sValue, "Type 5, A/B = (%d/%d) [0x%X/0x%X] Count is %d",
			n32a, n32b, n32a, n32b,
			field.count
		);
		lastAccessed = globalOffset + field.offset + 8 - 1;
		if (lastAccessed > highestAccessed)	{	highestAccessed = lastAccessed; };
		logThis({ sMsg, (globalOffset + field.offset), 8, sValue },	"");
		break;

	case 7:
		// undefined
		switch (field.tag)
		{
		case 0x0002:
			memcpy(sValue, &field.offset, 4);
			sValue[4] = 0;
			sprintf_s(
				sValue,
				"Exif Interoperability version is %c%c.%c%c",
				sValue[0], sValue[1], sValue[2], sValue[3]
			);
			logThis({ sMsg, 0, 0, sValue }, ""	);
			break;

		case 0x9000:
			memcpy(sValue, &field.offset, 4);
			sValue[4] = 0;
			sprintf_s(
				sValue,
				"Exif version is %c%c.%c%c",
				sValue[0], sValue[1], sValue[2], sValue[3]
			);
			logThis({ sMsg, 0, 0, sValue }, "");
			break;

		case 0x9101:
			memcpy(sValue, &field.offset, 4);
			sValue[4] = 0;
			sprintf_s(
				sValue,
				"Components Configuration [%c%c%c%c]",
				sValue[0], sValue[1], sValue[2], sValue[3]
			);
			logThis({ sMsg, 0, 0, sValue }, "");
			break;

		case 0xA000:
			memcpy(sValue, &field.offset, 4);
			sValue[4] = 0;
			sprintf_s(
				sValue,
				"FlashPix Version [%c%c.%c%c]",
				sValue[0], sValue[1], sValue[2], sValue[3]
			);
			logThis({ sMsg, 0, 0, sValue }, "");
			break;

		default:
			logThis( { sMsg, 0, 0, "type is 7, not yet implemented" }, "" );
			break;
		}
		break;

	default:
		// tags to be implemented?
		sprintf_s( sValue,
			"*** [0x%x], type is [0x%x], count is [0x%x], offset is [0x%x]",
			field.tag, field.type,
			field.count, field.offset
		);
		logThis({ sMsg, 0, 0, sValue },	"");
		break;
	}	// end switch

}	// end processField()
