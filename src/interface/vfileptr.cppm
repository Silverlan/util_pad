/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <fsys/vfileptr.h>
#include <vector>
#include <fsys/fsys_package.hpp>
#include <fsys/filesystem.h>

export module pragma.pad:vfileptr;

namespace pragma::pad {
	class PADPackage;
	class VFilePtrInternalPack : public VFilePtrInternal {
		std::vector<uint8_t> m_data;
		uint64_t m_offset = 0;
		bool m_bValid = false;
		bool m_bEof = false;
	  public:
		VFilePtrInternalPack();
		size_t Read(void *ptr, size_t size) override;
		unsigned long long Tell() override;
		void Seek(unsigned long long offset) override;
		int32_t Eof() override;
		int32_t ReadChar() override;
		unsigned long long GetSize() override;
		bool Construct(PADPackage &package, const std::string &fname, bool bBinary);
	};
};
