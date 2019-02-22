/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifdef UPAD_TEST

#include "util_pad.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_string.h>
#include <util_versioned_archive.hpp>

static void print_file_info(uva::FileInfo &fi,bool bPrintName,const std::string &t="")
{
	if(bPrintName == true)
		std::cout<<t<<fi.name;
	if(fi.IsDirectory())
		std::cout<<" (Directory)";
	else if(fi.IsFile())
		std::cout<<" (File)";
	std::cout<<": "<<util::get_pretty_bytes(fi.size)<<" ("<<util::get_pretty_bytes(fi.sizeUncompressed)<<" uncompressed); CRC: "<<fi.crc<<std::endl;
}

int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		std::cout<<"No file specified! Aborting..."<<std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return EXIT_SUCCESS;
	}
	auto launchParams = util::get_launch_parameters(argc,argv);
	auto version = util::Version{0,0,0};
	auto itVersion = launchParams.find("-version");
	if(itVersion != launchParams.end())
		version = util::Version::FromString(itVersion->second);
	std::string file = argv[1];
	auto path = ufile::get_path_from_filename(file);
	std::string ext;
	if(ufile::get_extension(file,&ext) == true && ustring::compare(ext,"pad",false) == true)
	{
		auto itPrintHierarchy = launchParams.find("-hierarchy");
		if(itPrintHierarchy != launchParams.end())
		{
			auto archFile = upad::open(file);
			if(archFile == nullptr)
				std::cout<<"Unable to open archive file '"<<file<<"'!"<<std::endl;
			else
			{
				auto &versions = archFile->GetVersions();
				if(itVersion != launchParams.end())
				{
					if(versions.empty() == true)
						std::cout<<"No versions found!"<<std::endl;
					else
					{
						VersionInfo *info = nullptr;
						if(version == util::Version{})
							info = &versions.front();
						else
						{
							auto it = std::find_if(versions.begin(),versions.end(),[&version](const VersionInfo &infoOther) {
								return (infoOther.version == version) ? true : false;
							});
							if(it == versions.end())
							{
								std::cout<<"Version "<<version.ToString()<<" not found in archive!"<<std::endl;
								std::this_thread::sleep_for(std::chrono::seconds(5));
								return EXIT_SUCCESS;
							}
							else
								info = &(*it);
						}
						if(info != nullptr)
						{
							if(info->files.empty())
							{
								std::cout<<"No files in version "<<version.ToString()<<"!"<<std::endl;
								std::this_thread::sleep_for(std::chrono::seconds(5));
								return EXIT_SUCCESS;
							}
							else
							{
								std::cout<<info->files.size()<<" contained in version "<<version.ToString()<<":"<<std::endl;
								for(auto fileId : info->files)
								{
									auto fi = archFile->GetByIndex(fileId);
									auto *fii = archFile->FindFileIndexInfo(*fi);
									if(fii == nullptr)
										std::cout<<"Invalid file '"<<fi->name<<"'!"<<std::endl;
									else
									{
										std::cout<<archFile->GetFullPath(fii);
										print_file_info(*fi,false);
									}
								}
							}
						}
					}
					char c;
					std::cin>>c;
					return EXIT_SUCCESS;
				}
				std::function<void(uva::ArchiveFile::FileIndexInfo&,const std::string&)> fPrintHierarchy = nullptr;
				fPrintHierarchy = [&fPrintHierarchy,&archFile](uva::ArchiveFile::FileIndexInfo &fii,const std::string &t) {
					auto fi = archFile->GetByIndex(fii.index);
					print_file_info(*fi,true,t);
					for(auto &child : fii.children)
						fPrintHierarchy(*child,t +'\t');
				};
				fPrintHierarchy(archFile->GetRoot(),"");
			}
			char c;
			std::cin>>c;
			return EXIT_SUCCESS;
		}
		std::cout<<"Extracting archive..."<<std::endl;
		FileManager::CreateSystemDirectory((path +"extracted").c_str());
		upad::extract(file,path +"extracted");
		std::cout<<"Complete!"<<std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return EXIT_SUCCESS;
	}
	auto fname = ufile::get_file_from_filename(file);
	ufile::remove_extension_from_filename(fname);
	upad::compose(version,file,path +fname +".pad");
	std::this_thread::sleep_for(std::chrono::seconds(5));
	return EXIT_SUCCESS;
}

#endif
