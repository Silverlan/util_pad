// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <fsys/vfileptr.h>
#include <vector>
#include <fsys/fsys_package.hpp>
#include <fsys/filesystem.h>

export module pragma.pad:vfileptr;

export namespace pragma::pad { class PADPackage; };

namespace pragma::pad {
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
