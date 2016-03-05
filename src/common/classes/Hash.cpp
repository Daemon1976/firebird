/*
 *	PROGRAM:	Common Library
 *	MODULE:		Hash.cpp
 *	DESCRIPTION:	Hash of data
 *
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  The Original Code was created by Dmitry Sibiryakov
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2015 Dmitry Sibiryakov
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 *
 */

#include "firebird.h"
#include "../common/classes/Hash.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

using namespace Firebird;

unsigned int CRC32C(unsigned int length, const unsigned char* value);

namespace
{
	bool SSE4_2Supported()
	{
#if defined(_M_IX86) || defined(_M_X64) || defined(__x86_64__) || defined(__i386__)

#ifdef _MSC_VER
		const int bit_SSE4_2 = 1 << 20;
		// MS VC has its own definition of __cpuid
		int flags[4];
		__cpuid(flags, 1);
		return (flags[2] & bit_SSE4_2) != 0;
#else
		// GCC - its own
		unsigned int eax,ebx,ecx,edx;
		__cpuid(1, eax, ebx, ecx, edx);
		return (ecx & bit_SSE4_2) != 0;
#endif

#else
		return false;
#endif // architecture check
	}

	unsigned int basicHash(unsigned int length, const UCHAR* value)
	{
		unsigned int hash_value = 0;

		UCHAR* p;
		const UCHAR* q = value;

		while (length >= 4)
		{
			p = (UCHAR*) &hash_value;
			p[0] += q[0];
			p[1] += q[1];
			p[2] += q[2];
			p[3] += q[3];
			length -= 4;
			q += 4;
		}

		p = (UCHAR*) &hash_value;

		if (length >= 2)
		{
			p[0] += q[0];
			p[1] += q[1];
			length -= 2;
		}

		if (length)
		{
			q += 2;
			*p += *q;
		}

		return hash_value;
	}

	typedef unsigned int (*hashFunc)(unsigned int length, const UCHAR* value);
	hashFunc internalHash = SSE4_2Supported() ? CRC32C : basicHash;
}

unsigned int InternalHash::hash(unsigned int length, const UCHAR* value)
{
	return internalHash(length, value);
}
