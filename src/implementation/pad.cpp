/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <fsys/filesystem.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_file.h>
#include <array>
#include <iostream>
#include <cstring>
#if UPAD_LUA_PRECOMPILE == 1
#include "lua_wrapper.hpp"
#endif

#define PAD_VERSION 0x0001

module pragma.pad;

import pragma.uva;

const std::array<char, 3> ident = {'P', 'A', 'D'};

static bool read_header(VFilePtr &f, std::shared_ptr<pragma::pad::PADPackage::Header> header)
{
	std::array<char, ident.size()> hdIdent = {};
	for(auto &c : hdIdent)
		c = f->Read<char>();
	if(ustring::compare(hdIdent.data(), ident.data(), true, ident.size()) == false)
		return false;
	header->version = f->Read<uint32_t>();
	header->flags = f->Read<uint32_t>();

	auto varchOffset = f->Read<uint64_t>();

	f->Read(header->packageId.data(), header->packageId.size());
	return true;
}

static bool write_header(VFilePtrReal &f, std::shared_ptr<pragma::pad::PADPackage::Header> header)
{
	f->Write(ident.data(), ident.size());
	f->Write<uint32_t>(PAD_VERSION);

	f->Write<uint32_t>(header->flags); // Flags

	auto offset = f->Tell();
	f->Write<uint64_t>(0u); // Offset to versioned archive data

	f->Write(header->packageId.data(), header->packageId.size()); // Placeholder for 32-character unique id (null-terminated)

	auto varchOffset = f->Tell();
	f->Seek(offset);
	f->Write<uint64_t>(varchOffset);
	f->Seek(varchOffset);
	return true;
}

#if UPAD_LUA_PRECOMPILE == 1
static std::vector<uint8_t> *s_compiledLuaData = nullptr;
struct lua_State;
static int lua_write_binary(lua_State *, unsigned char *str, size_t len, struct luaL_Buffer *)
{
	if(s_compiledLuaData == nullptr)
		return 0;
	auto offset = s_compiledLuaData->size();
	s_compiledLuaData->resize(s_compiledLuaData->size() + len);
	memcpy(s_compiledLuaData->data() + offset, str, len);
	return 0;
}

static void lua_compile(lua_State *l, std::vector<uint8_t> &data)
{
	data.clear();
	s_compiledLuaData = &data;
	luaL_Buffer buf;
	luaL_buffinit(l, &buf);
#ifdef USE_LUAJIT
	lua_dump_strip(l, (lua_Writer)lua_write_binary, &buf, 1);
#else
	lua_dump(l, (lua_Writer)lua_write_binary, &buf, 1);
#endif
	s_compiledLuaData = nullptr;
}
#endif

void pragma::pad::compose(const util::Version &version, const std::string &updateListFile, const std::string &archiveFile)
{
	auto result = pragma::uva::ArchiveFile::UpdateResult::Success;
#if UPAD_LUA_PRECOMPILE == 1
	auto *l = luaL_newstate();
#endif
	auto newVersion = version;
	auto header = std::make_shared<PADPackage::Header>();
	result = pragma::uva::ArchiveFile::PublishUpdate(newVersion, updateListFile, archiveFile, std::bind(read_header, std::placeholders::_1, header), std::bind(write_header, std::placeholders::_1, header),
	  [
#if UPAD_LUA_PRECOMPILE == 1
	    l
#endif
	](std::string &fname, std::string &archiveName, std::vector<uint8_t> &data) {
#if UPAD_LUA_PRECOMPILE == 1
		  std::string ext;
		  if(ufile::get_extension(fname, &ext) == true && ustring::compare<std::string>(ext, "lua", false) == true) {
			  auto status = luaL_loadfile(l, fname.c_str());
			  if(status) {
				  std::cout << "WARNING: Unable to compile Lua-file '" << fname << "': " << lua_tostring(l, -1) << std::endl;
				  std::cout << "Uncompiled Lua-file will be included..." << std::endl;
			  }
			  lua_compile(l, data);

			  ufile::remove_extension_from_filename(fname);
			  fname += ".clua";

			  ufile::remove_extension_from_filename(archiveName);
			  archiveName += ".clua";
		  }
#endif
	  });
#if UPAD_LUA_PRECOMPILE == 1
	lua_close(l);
#endif
	switch(result) {
	case pragma::uva::ArchiveFile::UpdateResult::Success:
		std::cout << "Update published successfully! New version: " << newVersion.ToString() << std::endl;
		break;
	case pragma::uva::ArchiveFile::UpdateResult::ListFileNotFound:
		std::cout << "Update failed: Update list not found!" << std::endl;
		break;
	case pragma::uva::ArchiveFile::UpdateResult::NothingToUpdate:
		std::cout << "Update failed: Nothing to update!" << std::endl;
		break;
	case pragma::uva::ArchiveFile::UpdateResult::UnableToCreateArchiveFile:
		std::cout << "Update failed: Unable to create archive file!" << std::endl;
		break;
	case pragma::uva::ArchiveFile::UpdateResult::VersionDiscrepancy:
		std::cout << "Update failed: Previous version number is greater than new version number!" << std::endl;
		break;
	case pragma::uva::ArchiveFile::UpdateResult::UnableToRemoveTemporaryFiles:
		std::cout << "Update failed: Unable to remove temporary files!" << std::endl;
		break;
	default:
		std::cout << "Update failed: Unknown error!" << std::endl;
		break;
	}
}

void pragma::pad::extract(const std::string &archiveFile, const std::string &outPath)
{
	auto archive = open(archiveFile);
	if(archive == nullptr)
		return;
	archive->ExtractAll(outPath);
}

std::unique_ptr<pragma::uva::ArchiveFile> pragma::pad::open(const std::string &archiveFile)
{
	std::shared_ptr<PADPackage::Header> header = nullptr;
	return open(archiveFile, header);
}

std::unique_ptr<pragma::uva::ArchiveFile> pragma::pad::open(const std::string &archiveFile, std::shared_ptr<PADPackage::Header> &header)
{
	header = std::make_shared<PADPackage::Header>();
	return std::unique_ptr<pragma::uva::ArchiveFile>(pragma::uva::ArchiveFile::Open(archiveFile, std::bind(read_header, std::placeholders::_1, header), std::bind(write_header, std::placeholders::_1, header)));
}

/////////////////////////

pragma::pad::PADPackage::Header::Header() { packageId.fill(0); }

std::unique_ptr<pragma::pad::PADPackage> pragma::pad::PADPackage::Create(const std::string &package, fsys::SearchFlags searchFlags)
{
	auto ptrPackage = std::unique_ptr<PADPackage>(new PADPackage(searchFlags));
	ptrPackage->m_packageName = package;
	if(ptrPackage->Open() == false)
		return nullptr;
	return ptrPackage;
}

pragma::pad::PADPackage::PADPackage(fsys::SearchFlags searchFlags) : Package(searchFlags), m_header(std::make_shared<Header>()) {}

const pragma::uva::ArchiveFile *pragma::pad::PADPackage::GetArchiveFile() const { return const_cast<PADPackage *>(this)->GetArchiveFile(); }
pragma::uva::ArchiveFile *pragma::pad::PADPackage::GetArchiveFile() { return m_arcFile.get(); }

const pragma::pad::PADPackage::Header &pragma::pad::PADPackage::GetHeader() const { return *m_header; }
uint32_t pragma::pad::PADPackage::GetVersion() const { return m_header->version; }
uint32_t pragma::pad::PADPackage::GetFlags() const { return m_header->flags; }
std::string pragma::pad::PADPackage::GetPackageId() const
{
	auto r = std::string(m_header->packageId.data(), m_header->packageId.size() - 1);
	if(r.find_first_not_of(' ') == std::string::npos)
		return "";
	return r;
}
util::Version pragma::pad::PADPackage::GetPackageVersion()
{
	auto &versions = m_arcFile->GetVersions();
	if(versions.empty() == true)
		return {};
	return versions.front().version;
}
void pragma::pad::PADPackage::Close() { m_arcFile = nullptr; }
bool pragma::pad::PADPackage::Open()
{
	if(m_arcFile != nullptr)
		return true;
	m_arcFile = std::unique_ptr<pragma::uva::ArchiveFile>(pragma::uva::ArchiveFile::Open(m_packageName, std::bind(read_header, std::placeholders::_1, m_header), std::bind(write_header, std::placeholders::_1, m_header)));
	return (m_arcFile != nullptr) ? true : false;
}

/////////////////////////

VFilePtr pragma::pad::open_package_file(pragma::pad::PADPackage &package, const std::string &fname, bool bBinary, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags)
{
	auto searchFlags = package.GetSearchFlags();
	if((includeFlags & searchFlags) != fsys::SearchFlags::None && ((includeFlags & fsys::SearchFlags::NoMounts) == fsys::SearchFlags::None || (includeFlags & fsys::SearchFlags::Package) == fsys::SearchFlags::Package)) {
		auto ptrPack = std::make_shared<VFilePtrInternalPack>();
		if(ptrPack->Construct(package, fname, bBinary))
			return ptrPack;
	}
	return nullptr;
}

pragma::uva::FileInfo *pragma::pad::get_file_info(pragma::pad::PADPackage &package, const std::string &fname, const fsys::SearchFlags *searchFlags)
{
	if(searchFlags == nullptr || ((*searchFlags & package.GetSearchFlags()) != fsys::SearchFlags::None && ((*searchFlags & fsys::SearchFlags::NoMounts) == fsys::SearchFlags::None || (*searchFlags & fsys::SearchFlags::Package) == fsys::SearchFlags::Package))) {
		auto *archFile = package.GetArchiveFile();
		return (archFile != nullptr) ? archFile->FindFile(fname) : nullptr;
	}
	return nullptr;
}

pragma::pad::PackageManager *pragma::pad::link_to_file_system()
{
	auto *padManager = new pragma::pad::PackageManager();
	FileManager::RegisterPackageManager("upad", std::unique_ptr<pragma::pad::PackageManager>(padManager));
	return padManager;
}

/////////////////////////

pragma::pad::PADPackage *pragma::pad::PackageManager::GetPackage(std::string package)
{
	ustring::to_lower(package);
	std::string ext;
	if(ufile::get_extension(package, &ext) == false)
		package += ".pad";
	auto it = m_packages.find(package);
	if(it == m_packages.end())
		return nullptr;
	return it->second.get();
}
fsys::Package *pragma::pad::PackageManager::LoadPackage(std::string package, fsys::SearchFlags searchMode)
{
	searchMode |= fsys::SearchFlags::Package;
	searchMode &= ~fsys::SearchFlags::Virtual;
	searchMode &= ~fsys::SearchFlags::Local;
	searchMode &= ~fsys::SearchFlags::LocalRoot;
	package = GetPackageFileName(package);
	auto ptrPackage = pragma::pad::PADPackage::Create(package, searchMode);
	if(ptrPackage == nullptr)
		return nullptr;
	auto *p = ptrPackage.get();
	m_packages.insert(std::make_pair(package, std::move(ptrPackage)));
	return p;
}

std::string pragma::pad::PackageManager::GetPackageFileName(std::string package) const
{
	ustring::to_lower(package);
	std::string ext;
	if(ufile::get_extension(package, &ext) == false)
		package += ".pad";
	return package;
}

void pragma::pad::PackageManager::ClearPackages(fsys::SearchFlags searchMode)
{
	for(auto it = m_packages.begin(); it != m_packages.end();) {
		auto &ptrPackage = it->second;
		if((ptrPackage->GetSearchFlags() & searchMode) != fsys::SearchFlags::None)
			it = m_packages.erase(it);
		else
			++it;
	}
}

void pragma::pad::PackageManager::FindFiles(const std::string &target, const std::string &path, std::vector<std::string> *resfiles, std::vector<std::string> *resdirs, bool bKeepPath, fsys::SearchFlags includeFlags) const
{
	for(auto &pair : m_packages) {
		if((includeFlags & pair.second->GetSearchFlags()) != fsys::SearchFlags::None && ((includeFlags & fsys::SearchFlags::NoMounts) == fsys::SearchFlags::None || (includeFlags & fsys::SearchFlags::Package) == fsys::SearchFlags::Package)) {
			std::vector<pragma::uva::FileInfo *> results;
			auto *archFile = pair.second->GetArchiveFile();
			if(archFile != nullptr) {
				archFile->SearchFiles(target, results);
				for(auto *fi : results) {
					auto pathName = bKeepPath == false ? fi->name : (path + fi->name);
					if(fi->IsFile() == true) {
						if(resfiles != nullptr && !HasValue(resfiles, 0, resfiles->size(), fi->name))
							resfiles->push_back(pathName);
					}
					else if(fi->IsDirectory() == true && resdirs != nullptr && !HasValue(resdirs, 0, resdirs->size(), fi->name))
						resdirs->push_back(pathName);
				}
			}
		}
	}
}

bool pragma::pad::PackageManager::GetSize(const std::string &name, uint64_t &size) const
{
	for(auto &pair : m_packages) {
		auto *fileInfo = pragma::pad::get_file_info(*pair.second, name);
		if(fileInfo == nullptr)
			continue;
		size = fileInfo->IsDirectory() ? 0 : fileInfo->sizeUncompressed;
		return true;
	}
	return false;
}

bool pragma::pad::PackageManager::Exists(const std::string &name, fsys::SearchFlags includeFlags) const
{
	for(auto &pair : m_packages) {
		if(pragma::pad::get_file_info(*pair.second, name, &includeFlags) != nullptr)
			return true;
	}
	return false;
}

bool pragma::pad::PackageManager::GetFileFlags(const std::string &name, fsys::SearchFlags includeFlags, uint64_t &flags) const
{
	for(auto &pair : m_packages) {
		auto *info = pragma::pad::get_file_info(*pair.second, name, &includeFlags);
		if(info == nullptr)
			continue;
		flags = FVFILE_READONLY | FVFILE_PACKAGE;
		if(info->IsDirectory())
			flags |= FVFILE_DIRECTORY;
		else if(info->IsCompressed())
			flags |= FVFILE_COMPRESSED;
		return true;
	}
	return false;
}
VFilePtr pragma::pad::PackageManager::OpenFile(const std::string &package, const std::string &path, bool bBinary, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags) const
{
	auto it = m_packages.find(package);
	if(it == m_packages.end())
		return nullptr;
	return pragma::pad::open_package_file(*it->second, path, bBinary, includeFlags, excludeFlags);
}

VFilePtr pragma::pad::PackageManager::OpenFile(const std::string &path, bool bBinary, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags) const
{
	for(auto &pair : m_packages) {
		if((includeFlags & pair.second->GetSearchFlags()) != fsys::SearchFlags::None && ((includeFlags & fsys::SearchFlags::NoMounts) == fsys::SearchFlags::None || (includeFlags & fsys::SearchFlags::Package) == fsys::SearchFlags::Package)) {
			auto pfile = pragma::pad::open_package_file(*pair.second, path, bBinary, includeFlags, excludeFlags);
			if(pfile != nullptr)
				return pfile;
		}
	}
	return nullptr;
}
