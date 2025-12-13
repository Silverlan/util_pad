// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.pad;

import :vfileptr;
import pragma.uva;

export namespace pragma::pad {
	class PackageManager;
	PackageManager *link_to_file_system();
	void compose(const util::Version &version, const std::string &updateListFile, const std::string &archiveFile);
	void extract(const std::string &archiveFile, const std::string &outPath);

	class PADPackage : public fs::Package {
	  public:
		struct Header {
			Header();
			uint32_t version = 0;
			uint32_t flags = 0;
			std::array<char, 33> packageId;
		};
		static std::unique_ptr<PADPackage> Create(const std::string &package, fs::SearchFlags searchFlags);
		PADPackage(fs::SearchFlags searchFlags);
		const uva::ArchiveFile *GetArchiveFile() const;
		uva::ArchiveFile *GetArchiveFile();

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
		std::unique_ptr<uva::ArchiveFile> m_arcFile = nullptr;
	};
	fs::VFilePtr open_package_file(PADPackage &package, const std::string &fname, bool bBinary, fs::SearchFlags includeFlags, fs::SearchFlags excludeFlags);
	uva::FileInfo *get_file_info(PADPackage &package, const std::string &fname, const fs::SearchFlags *searchFlags = nullptr);

	std::unique_ptr<uva::ArchiveFile> open(const std::string &archiveFile);
	std::unique_ptr<uva::ArchiveFile> open(const std::string &archiveFile, std::shared_ptr<PADPackage::Header> &header);

	class PackageManager : public fs::PackageManager {
	  public:
		PackageManager() = default;
		PADPackage *GetPackage(std::string package);
		virtual fs::Package *LoadPackage(std::string package, fs::SearchFlags searchMode = fs::SearchFlags::Local) override;
		virtual void ClearPackages(fs::SearchFlags searchMode) override;
		virtual void FindFiles(const std::string &target, const std::string &path, std::vector<std::string> *resfiles, std::vector<std::string> *resdirs, bool bKeepPath, fs::SearchFlags includeFlags) const override;
		virtual bool GetSize(const std::string &name, uint64_t &size) const override;
		virtual bool Exists(const std::string &name, fs::SearchFlags includeFlags) const override;
		virtual bool GetFileFlags(const std::string &name, fs::SearchFlags includeFlags, fs::FVFile &flags) const override;
		virtual fs::VFilePtr OpenFile(const std::string &path, bool bBinary, fs::SearchFlags includeFlags, fs::SearchFlags excludeFlags) const override;
		fs::VFilePtr OpenFile(const std::string &package, const std::string &path, bool bBinary, fs::SearchFlags includeFlags, fs::SearchFlags excludeFlags) const;
	  protected:
		std::string GetPackageFileName(std::string package) const;
		std::unordered_map<std::string, std::unique_ptr<PADPackage>> m_packages;
	};
};
