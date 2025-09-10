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

#include "stdafx.h"
#include "File.h"
#include <exception>
#include <cstdlib>
#include <sstream>
#include <cassert>
#include <ctime>

File::File() : rfile(0), wfile(0), opened(false)
{}

File::File(const std::string& name) : mFileName(name)
{
	rfile = fopen(name.c_str(),"rb");
	if (!rfile)
		throw std::runtime_error("File " + name + " does not exist!");

	// generate temporary output file name
	std::stringstream s;
	srand((unsigned int)time(0));
	s << name << "." << rand()%1000 << ".tmp";
	mTempFile = s.str();

	wfile = fopen(mTempFile.c_str(),"wb");
	if (!wfile)
		throw std::runtime_error("Error occured while creating temporary file "+mTempFile+"!");

	opened = true;
}

void File::open(const std::string& name)
{
	if (opened)
		throw std::logic_error("File already open!");

	mFileName = name;

	rfile = fopen(name.c_str(),"rb");
	if (!rfile)
		throw std::runtime_error("File " + name + " does not exist!");

	// generate temporary output file name
	std::stringstream s;
	srand((unsigned int)time(0));
	s << name << "." << rand()%1000 << ".tmp";
	mTempFile = s.str();

	wfile = fopen(mTempFile.c_str(),"wb");
	if (!wfile)
		throw std::runtime_error("Error occured while creating temporary file "+mTempFile+"!");

	opened = true;
}

void File::close()
{
	if (opened)
	{
		assert(rfile && wfile);
		fclose(rfile);
		fclose(wfile);

		// save file attrib and remove READONLY attribute
		DWORD attrib = GetFileAttributesA(mFileName.c_str());
		SetFileAttributesA(mFileName.c_str(),attrib & (~FILE_ATTRIBUTE_READONLY));

		// delete original file
		if (!DeleteFileA(mFileName.c_str()))
			throw std::runtime_error("Cannot replace file " + mFileName +"!");

		// rename temp file to original file
		if (!MoveFileA(mTempFile.c_str(),mFileName.c_str()))
			throw std::runtime_error("Cannot rename temp file!");

		SetFileAttributesA(mFileName.c_str(),attrib);

		opened = false;
	}
}

void File::revert()
{
	if (opened)
	{
		assert(rfile && wfile);
		fclose(rfile);
		fclose(wfile);

		DeleteFileA(mTempFile.c_str());
		opened = false;
	}
}

int File::read(char* buf,size_t size)
{
	return fread(buf,1,size,rfile);
}

void File::write(const char* buf,size_t size)
{
	if (fwrite(buf,1,size,wfile) != size)
		throw std::runtime_error("Could not write to file. Disk may be full!");
}

size_t File::size()
{
	size_t curpos = ftell(rfile);

	fseek(rfile,0,SEEK_END);
	size_t filesize = ftell(rfile);

	fseek(rfile,curpos,SEEK_SET);
	return filesize;
}

bool File::eof()
{
	if (feof(rfile))
		return true;
	else
		return false;
}
