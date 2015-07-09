/*! ----------------------------------------------------------
 *
 *  \file       sca_globals.cc
 *
 *  \brief
 *      
 *
 *  \details
 *      Detailed description of file
 *      
 *  \history
 *      06/06/14 02:23:43 PDT Created By Prakash S
 *
 *  ----------------------------------------------------------*/
#include "sca_globals.hh"

FileIdMgr* FileIdMgr::instance_ = NULL;
PkgIdMgr* PkgIdMgr::instance_   = NULL;
SCA_globals* SCA_globals::instance_ = NULL;

//==============================================================================
///      \class   FileIdMgr
///      \method  insertFile
///      \brief   Inserts the file named "file_name" and returns the FileId
///               associated with it. Inturn also create the File and Package
//                object if needed
//==============================================================================
FileId 
FileIdMgr::insertFile(std::string file_name) {
  SCA::File::FileType file_type = getType(file_name);
  if(!(file_type == SCA::File::kSource || file_type == SCA::File::kHeader)) {
    return INVALID_FILE_ID;
  }
  // Split up the path in to pkg name and base name
  std::string base_name;
  std::string pkg_name;
  split_path(file_name, pkg_name, base_name); 
  if(!name_vs_fileId_.count(base_name)) {
    PkgId pkg_id = (PkgIdMgr::getPkgIdMgr()->insertPkg(pkg_name));
    FileId cur_id = file_count_++;
    name_vs_fileId_[base_name] = cur_id;
    SCA::File* file=NULL;
    if(file_type == SCA::File::kSource) { 
      file = new SCA::SourceFile(SCA_PKG(pkg_id), base_name, cur_id);
    } else {
      file = new SCA::HeaderFile(SCA_PKG(pkg_id), base_name, cur_id);
    }
    files_.push_back(file);
  } 
  return name_vs_fileId_[base_name];
}

//==============================================================================
///      \class   FileIdMgr
///      \method  getFileId
///      \brief   Get FileId from the file name
//==============================================================================
FileId 
FileIdMgr::getFileId(std::string file_name) {
  SCA::File::FileType file_type = getType(file_name);
  if(!(file_type == SCA::File::kSource || file_type == SCA::File::kHeader)) {
    return INVALID_FILE_ID;
  }
  // Split up the path in to pkg name and base name
  std::string base_name;
  std::string pkg_name;
  split_path(file_name, pkg_name, base_name); 
  return ((name_vs_fileId_.count(base_name) != 0) ? name_vs_fileId_[base_name] : INVALID_FILE_ID);
}

//==============================================================================
///      \class   FileIdMgr
///      \method  getFileName
///      \brief   Get the file name from the id
//==============================================================================
const char* 
FileIdMgr::getFileName(FileId id) {
  assert(id < file_count_);
  return files_[id]->get_full_name().c_str();
}

//==============================================================================
///      \class   FileIdMgr
///      \method  getFIle
///      \brief   Gets the actual SCA::File from the file id
//==============================================================================
SCA::File* FileIdMgr::getFile(FileId id) {
  assert(id < file_count_);
  return files_[id];
}

//==============================================================================
///      \class   PkgIdMgr
///      \method  insertPkg
///      \brief   Inserts the package in the mgr and returns the package id
//==============================================================================
PkgId PkgIdMgr::insertPkg(std::string &pkg_name) {
  if(!name_vs_pkgId_.count(pkg_name)) {
    name_vs_pkgId_[pkg_name] = pkg_count_++;
    packages_.push_back(new SCA::Package(pkg_name, pkg_count_-1));
  } 
  return name_vs_pkgId_[pkg_name];
}

//==============================================================================
///      \class   PkgIdMgr
///      \method  getPkgId
///      \brief   Gets the pkg id from the name
//==============================================================================
PkgId PkgIdMgr::getPkgId(std::string &pkg_name) {
  return ((name_vs_pkgId_.count(pkg_name) != 0) ? name_vs_pkgId_[pkg_name] : INVALID_PKG_ID);
}

//==============================================================================
///      \class   PkgIdMgr
///      \method  getPkgName
///      \brief   Gets the pkg name from the id
//==============================================================================
const char* PkgIdMgr::getPkgName(PkgId id) {
  assert(id < pkg_count_);
  return packages_[id]->name().c_str();
}

//==============================================================================
///      \class   PkgIdMgr
///      \method  getPkg
///      \brief   Gets the actual CCm::Package from the id
//==============================================================================
SCA::Package* PkgIdMgr::getPkg(PkgId id) {
  assert(id < pkg_count_);
  return packages_[id];
}

