/*
-------------------------------------------------------------------------------
 This source file is part of ChipherShield (a file encryption software).

 Copyright (C) 2002-2003, Arijit De <arijit1985@yahoo.co.in>

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 http://www.gnu.org/copyleft/gpl.html.
-------------------------------------------------------------------------------
*/

#ifndef _Algorithm_H__
#define _Algorithm_H__

#include <cryptlib.h>

namespace Algorithm
{

	enum Type
	{
		AES = CRYPT_ALGO_AES,
		DES = CRYPT_ALGO_DES,
		TRIPLE_DES = CRYPT_ALGO_3DES,
		IDEA = CRYPT_ALGO_IDEA,
		CAST_128 = CRYPT_ALGO_CAST,
		RC2 = CRYPT_ALGO_RC2,
		RC4 = CRYPT_ALGO_RC4,
		RC5 = CRYPT_ALGO_RC5,
		BLOWFISH = CRYPT_ALGO_BLOWFISH,
		SKIPJACK = CRYPT_ALGO_SKIPJACK
	};

};

#endif // _Algorithm_H__