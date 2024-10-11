/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <cstring>
#include <fsys/filesystem.h>

module pragma.pad;

import pragma.uva;

//#define MAX_BUFFER_SIZE 5242880
pragma::pad::VFilePtrInternalPack::VFilePtrInternalPack() : VFilePtrInternal() { m_type = VFILE_PACKAGE; }

bool pragma::pad::VFilePtrInternalPack::Construct(pragma::pad::PADPackage &package, const std::string &fname, bool bBinary)
{
	m_bBinary = bBinary;
	m_bRead = true;
	auto *archFile = package.GetArchiveFile();
	if(archFile == nullptr || archFile->ExtractData(fname, m_data) == false)
		return false;
	m_bValid = true;
	return true;
}

size_t pragma::pad::VFilePtrInternalPack::Read(void *ptr, size_t size)
{
	if(Eof() == EOF)
		return std::numeric_limits<size_t>::max();
	if(m_offset >= GetSize()) {
		m_bEof = true;
		return std::numeric_limits<size_t>::max();
	}
	auto szMin = GetSize() - m_offset;
	if(size > szMin)
		size = szMin;
	memcpy(ptr, m_data.data() + m_offset, size);
	m_offset += size;
	if(m_offset > GetSize()) // Only if larger!
		m_bEof = true;
	return size;
}

unsigned long long pragma::pad::VFilePtrInternalPack::GetSize() { return m_data.size(); }
unsigned long long pragma::pad::VFilePtrInternalPack::Tell() { return m_offset; }
void pragma::pad::VFilePtrInternalPack::Seek(unsigned long long offset)
{
	m_offset = offset;
	m_bEof = false;
}
int32_t pragma::pad::VFilePtrInternalPack::Eof() { return !m_bEof ? 0 : EOF; }
int32_t pragma::pad::VFilePtrInternalPack::ReadChar()
{
	if(m_offset >= GetSize()) {
		m_bEof = true;
		return m_data.empty() ? EOF : m_data.back();
	}
	return m_data.at(m_offset++);
}
