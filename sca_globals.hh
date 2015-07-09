/*! ----------------------------------------------------------
 *
 *  \file       sca_globals.hh
 *
 *  \brief
 *      Singleton class to hold all the globals
 *
 *  \details
 *      Detailed description of file
 *
 *  \history
 *      05/15/14 02:42:21 PDT Created by Prakash S
 *
 *  ----------------------------------------------------------*/

#ifndef SCA_GLOBALS_H
#define SCA_GLOBALS_H

/*--------------------------------------------------------------
 *
 *      Includes
 */
#include "sca_int.hh"
#include "sca_utils.hh"

#define SCA_GET_WRITE_PTR() \
  (SCA_globals::getGlobals()->get_write_descriptor())
 
#define SCA_GETFILEFROMID(sourceId) \
   (SourceIdConvertor::getFileIdFromSourceId(sourceId))

#define SCA_GETLINEFROMID(sourceId) \
   (SourceIdConvertor::getLineIdFromSourceId(sourceId))

#define SCA_FILEID(str) \
  (FileIdMgr::getFileIdMgr()->getFileId(str))

#define SCA_FILE(id) \
  (FileIdMgr::getFileIdMgr()->getFile(id))

#define SCA_PKGID(str) \
  (PkgIdMgr::getPkgIdMgr()->getPkgId(str))
  
#define SCA_PKG(id) \
  (PkgIdMgr::getPkgIdMgr()->getPkg(id))

#define SCA_FILEEXIST(str) \
  (FileIdMgr::getFileIdMgr()->getFileId(str) != INVALID_FILE_ID)

#define SCA_PKGEXIST(str) \
  (PkgIdMgr::getPkgIdMgr()->getPkgId(str) != INVALID_PKG_ID)

// ============================================================================
///        \class  SCA_globals
///        \brief  Holds all the global vars used by SCA during file parsing
// ============================================================================
class SCA_globals {
  public:
    /// static accessor to get the singleton object
    static SCA_globals* getGlobals(void) {
      if(instance_ == NULL) {
        instance_ = new SCA_globals;
      }
      return instance_;
    }
    // Set the file that is being parsed
    void set_parse_id(FileId id) {
      parse_id_ = id;
    }
    /// Get the file that is being parsed currently
    FileId parse_id(void) const {
      return parse_id_;
    }
    void clear(void) {
      class_hash_.clear();
      function_hash_.clear();
      if(fp_) {
        fclose(fp_);
      }
    }
     /// Class hash accesssor
    DoubleHash<SCA::Class>& class_hash(void) { return class_hash_; }
    /// Function hash accessor
    DoubleHash<SCA::Function>& function_hash(void) { return function_hash_; }
    /// Set the cwd
    void set_cwd(std::string cwd) {
      cwd_ = cwd;
    }
    /// Get the cwd
    std::string cwd(void) const {
      return cwd_;
    }
    void set_write_descriptor(FILE *fp) {
      fp_ = fp;
    }
    FILE* get_write_descriptor(void) const {
      return fp_;
    }
    // Static instance
    static SCA_globals* instance_;
  private:
    /// Private constructor
    SCA_globals() {
      fp_             = NULL;
      parse_id_       = INVALID_FILE_ID;
    }
    /// File being parsed currently
    FileId parse_id_;
    /// DoubleHas for class
    DoubleHash<SCA::Class> class_hash_;
    ///Double hash for function
    DoubleHash<SCA::Function> function_hash_;
    /// The current working directory
    std::string cwd_;
    /// The file pointer for the writing .o
    FILE *fp_;
};

// ============================================================================
///        \class  FileIdMgr
///        \brief  File Id manager. Assings a unique ID for each file and provides
//                 lookup of <FileId, SCA::File*>
// ============================================================================
class FileIdMgr {
  public:
    /// Static accessor to get the singleton object
    static FileIdMgr* getFileIdMgr(void) {
      if(instance_ == NULL) {
        instance_ = new FileIdMgr();
      }
      return instance_;
    }
    /// Inserts the file named "file_name" and returns the FileId
    ///  associated with it. Inturn also create the File and Package object 
    ///  if needed
    FileId insertFile(std::string file_name);
    /// Get the fileId assocaited with the "file_name"
    FileId getFileId(std::string file_name);
    /// Get the file name from the FIleId
    const char* getFileName(FileId id);
    /// Get the actual SCA::File from the fileId
    SCA::File* getFile(FileId id);
    /// Total number of files
    FileId cFiles(void) const { return file_count_; }
    /// Clear all the contents
    void clear(void) {
      for(unsigned int i=0; i<file_count_; i++) {
        delete files_[i];
      }
      delete instance_;
      instance_ = NULL;
    }

  public:
    static FileIdMgr *instance_;

  private:
    /// Private constructor
    FileIdMgr(void) {
      file_count_ = 0;
    }
    /// Map of <file_name, FileId> 
    std::map<std::string, FileId> name_vs_fileId_;
    /// Array of SCA::File indexed with FileId
    std::vector<SCA::File*> files_;
    /// Total file count
    U file_count_;
    PkgId pkgId_;
};

// ============================================================================
///        \class  PkgIdMgr
///        \brief  pacakge Id manger. Assigns a unique ID for each package
// ============================================================================
class PkgIdMgr {
  public:
    /// Static accessor to get the singleton object
    static PkgIdMgr* getPkgIdMgr(void) {
      if(instance_ == NULL) {
        instance_ = new PkgIdMgr();
      }
      return instance_;
    }
    /// Inserts the package in the manager and returns the pkgId. Also 
    //  creates the SCA::Package if needed internally
    PkgId insertPkg(std::string &pkg_name);
    /// Get PkgId from the name
    PkgId getPkgId(std::string &pkg_name);
    /// Get the package name from the id
    const char* getPkgName(PkgId id);
    /// Get the actual SCA::Package from id
    SCA::Package* getPkg(PkgId id);
    /// Total number of pacakges with mgr
    PkgId cPkgs(void) const { return pkg_count_; }
    /// Clear all the contents
    void clear(void) {
      for(unsigned int i=0; i<pkg_count_; i++) {
        delete packages_[i];
      }
      delete instance_;
      instance_ = NULL;
    }

  public:
    static PkgIdMgr *instance_;

  private:
    /// Private constructor
    PkgIdMgr(void) {
      pkg_count_ = 0;
    }
    /// Map of <pkg_name, PkgId> 
    std::map<std::string, PkgId> name_vs_pkgId_;
    /// Array of SCA::Package indexed with PkgId
    std::vector<SCA::Package*> packages_;
    /// Total pkg count
    U pkg_count_;
};

#endif    /* SCA_GLOBALS_H */

