// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <unordered_map>
#include <sharedutils/util_version.h>
#include <string>
#include <memory>
#include <fsys/fsys_package.hpp>
#include <array>
#include <fsys/vfileptr.h>

export module pragma.pad;

import :vfileptr;
import pragma.uva;

export namespace pragma::pad {
	class PackageManager;
	PackageManager *link_to_file_system();
	void compose(const util::Version &version, const std::string &updateListFile, const std::string &archiveFile);
	void extract(const std::string &archiveFile, const std::string &outPath);

	class PADPackage : public fsys::Package {
	  public:
		struct Header {
			Header();
			uint32_t version = 0;
			uint32_t flags = 0;
			std::array<char, 33> packageId;
		};
		static std::unique_ptr<PADPackage> Create(const std::string &package, fsys::SearchFlags searchFlags);
		PADPackage(fsys::SearchFlags searchFlags);
		const pragma::uva::ArchiveFile *GetArchiveFile() const;
		pragma::uva::ArchiveFile *GetArchiveFile();

		const Header &GetHeader() const;
		uint32_t GetVersion() const;
		uint32_t GetFlags() const;
		std::string GetPackageId() const;
		util::Version GetPackageVersion();
		void Close();
		bool Open();
	  private:
		std::shared_ptr<Header> m_header = nullptr;
		std::string m_packageName;
		std::unique_ptr<pragma::uva::ArchiveFile> m_arcFile = nullptr;
	};
	VFilePtr open_package_file(PADPackage &package, const std::string &fname, bool bBinary, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags);
	pragma::uva::FileInfo *get_file_info(PADPackage &package, const std::string &fname, const fsys::SearchFlags *searchFlags = nullptr);

	std::unique_ptr<pragma::uva::ArchiveFile> open(const std::string &archiveFile);
	std::unique_ptr<pragma::uva::ArchiveFile> open(const std::string &archiveFile, std::shared_ptr<PADPackage::Header> &header);

	class PackageManager : public fsys::PackageManager {
	  public:
		PackageManager() = default;
		PADPackage *GetPackage(std::string package);
		virtual fsys::Package *LoadPackage(std::string package, fsys::SearchFlags searchMode = fsys::SearchFlags::Local) override;
		virtual void ClearPackages(fsys::SearchFlags searchMode) override;
		virtual void FindFiles(const std::string &target, const std::string &path, std::vector<std::string> *resfiles, std::vector<std::string> *resdirs, bool bKeepPath, fsys::SearchFlags includeFlags) const override;
		virtual bool GetSize(const std::string &name, uint64_t &size) const override;
		virtual bool Exists(const std::string &name, fsys::SearchFlags includeFlags) const override;
		virtual bool GetFileFlags(const std::string &name, fsys::SearchFlags includeFlags, uint64_t &flags) const override;
		virtual VFilePtr OpenFile(const std::string &path, bool bBinary, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags) const override;
		VFilePtr OpenFile(const std::string &package, const std::string &path, bool bBinary, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags) const;
	  protected:
		std::string GetPackageFileName(std::string package) const;
		std::unordered_map<std::string, std::unique_ptr<PADPackage>> m_packages;
	};
};
