/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "util_vfileptr.hpp"
#include "util_pad.hpp"
#include <cstring>

import pragma.uva;

//#define MAX_BUFFER_SIZE 5242880
upad::VFilePtrInternalPack::VFilePtrInternalPack() : VFilePtrInternal() { m_type = VFILE_PACKAGE; }

bool upad::VFilePtrInternalPack::Construct(upad::PADPackage &package, const std::string &fname, bool bBinary)
{
	m_bBinary = bBinary;
	m_bRead = true;
	auto *archFile = package.GetArchiveFile();
	if(archFile == nullptr || archFile->ExtractData(fname, m_data) == false)
		return false;
	m_bValid = true;
	return true;
}

size_t upad::VFilePtrInternalPack::Read(void *ptr, size_t size)
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

unsigned long long upad::VFilePtrInternalPack::GetSize() { return m_data.size(); }
unsigned long long upad::VFilePtrInternalPack::Tell() { return m_offset; }
void upad::VFilePtrInternalPack::Seek(unsigned long long offset)
{
	m_offset = offset;
	m_bEof = false;
}
int32_t upad::VFilePtrInternalPack::Eof() { return !m_bEof ? 0 : EOF; }
int32_t upad::VFilePtrInternalPack::ReadChar()
{
	if(m_offset >= GetSize()) {
		m_bEof = true;
		return m_data.empty() ? EOF : m_data.back();
	}
	return m_data.at(m_offset++);
}
