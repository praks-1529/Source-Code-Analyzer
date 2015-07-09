/*! ----------------------------------------------------------
 *
 *  \file       sca_utils.cc
 *
 *  \brief
 *      
 *
 *  \details
 *      Detailed description of file
 *      
 *  \history
 *      06/06/14 03:08:27 PDT Created By Prakash S
 *
 *  ----------------------------------------------------------*/
#include "sca_utils.hh"
#include "sca_globals.hh"

using namespace SCA;

//==============================================================================
///      \brief Split the given path in fo dir and base names
//==============================================================================
void split_path(const std::string &ip, 
                std::string &dir_name, 
                std::string &base_name) {
  char delim_char = '/';
  int delim_index = -1;
  for(int i=ip.size(); i>=0; i--) {
    if(ip[i] == delim_char) {
      delim_index = i;
      break;
    }
  }
  if(delim_index == -1) {
    base_name = ip;
    return;
  }
  // Directory name
  dir_name.resize(delim_index);
  int i;
  for(i=0; i<delim_index; i++) {
    dir_name[i] = ip[i]; 
  }
  i++;
  // Base name
  base_name.resize(ip.size()-delim_index-1);
  for(; i<ip.size(); i++) {
    base_name[i-delim_index-1] = ip[i];
  } 
}

//==============================================================================
///     \brief Tells if it is a header/source file
//==============================================================================
File::FileType getType(std::string &file_name) {
  if(file_name[file_name.size() - 4] == '.') {
    // Of the form .cxx, .hxx, .c++, .h++, .cpp, .hpp
    if(file_name[file_name.size() - 3] == 'c') {
      return SCA::File::kSource;
    } else if(file_name[file_name.size() - 3] == 'h') {
      return SCA::File::kHeader;
    } else {
      return SCA::File::kUnknown;
    }
  } else if(file_name[file_name.size() - 3] == '.') {
    // Of the form .cc, .hh
    if(file_name[file_name.size() - 2] == 'c') {
      return SCA::File::kSource;
    } else if(file_name[file_name.size() - 2] == 'h') {
      return SCA::File::kHeader;
    } else {
      return SCA::File::kUnknown;
    }
  } else if(file_name[file_name.size() - 2] == '.') {
    // Of the form .c, .h
    if(file_name[file_name.size() - 1] == 'c') {
      return SCA::File::kSource;
    } else if(file_name[file_name.size() - 1] == 'h') {
      return SCA::File::kHeader;
    } else if(file_name[file_name.size() - 1] == 'o') {
      return SCA::File::kObject;
    } else if(file_name[file_name.size() - 1] == 'a') {
      return SCA::File::kArchive;
    } else {
      return SCA::File::kUnknown;
    }
  } else {
    return SCA::File::kUnknown;
  }
}

//--------------------------------------------------------------------------------------
///      \brief Get absoulte path from relative path
//--------------------------------------------------------------------------------------
void getAbsolutePathFromRelativePath(const char* relative, char* absolute){
  if(relative) {
    realpath(relative, absolute); 
  }
}

//--------------------------------------------------------------------------------------
///      \brief Get SCA::File from CXCursor
//--------------------------------------------------------------------------------------
SCA::File* getFileFromCursor(CXCursor cursor) {
  CXSourceLocation sl = clang_getCursorLocation(cursor);
  CXFile file;
  unsigned line_num, col_num, offset;
  clang_getFileLocation(sl, &file, &line_num, &col_num, &offset);
  const char* c_file = clang_getCString(clang_getFileName(file));    
  if(c_file) {
    char real_path[1024];
    getAbsolutePathFromRelativePath(c_file, real_path);
    FileId file_id = FileIdMgr::getFileIdMgr()->insertFile(std::string(real_path)); 
    if(file_id != INVALID_FILE_ID) {
      return SCA_FILE(file_id);
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

//--------------------------------------------------------------------------------------
///      \brief Geti SourceId from the SourceLocation
//--------------------------------------------------------------------------------------
SourceId getSourceIdFromSourceLocation(CXSourceLocation sl) {
  CXFile file;
  unsigned line_num, col_num, offset;
  clang_getFileLocation(sl, &file, &line_num, &col_num, &offset);
  return SourceId(clang_getCString(clang_getFileName(file)), line_num);
}

//--------------------------------------------------------------------------------------
///      \brief Geti SourceId from the CXCursor
//--------------------------------------------------------------------------------------
SourceId getSourceIdFromCursor(CXCursor cursor) {
  CXSourceLocation sl = clang_getCursorLocation(cursor);
  return getSourceIdFromSourceLocation(sl);
}

//--------------------------------------------------------------------------------------
///      \brief Get number of lines defined by the cursor
//--------------------------------------------------------------------------------------
U getLineScopeFromCursor(CXCursor cursor) {
  CXSourceRange sr = clang_getCursorExtent(cursor);
  SourceId start_id = getSourceIdFromSourceLocation(clang_getRangeStart(sr));
  SourceId end_id = getSourceIdFromSourceLocation(clang_getRangeEnd(sr));
  return (end_id.line_no() - start_id.line_no() + 1);
}

//==============================================================================
///      \brief Tells if the file is a header file/source file/object file
//==============================================================================
bool isFileOfInterest(std::string &file_name) {
  return (getType(file_name) != SCA::File::kUnknown);
}
 
//==============================================================================
///      \class   Obj
///      \method  addChild
///      \brief  Adds the child to the parent
//==============================================================================
void Obj::addChild(Obj* child) {
  childs_.push_back(child);
}

//==============================================================================
///      \class   Obj
///      \method  getChildOfType
///      \brief   Gets the child that matches "name" and "type"
//==============================================================================
Obj* Obj::getChildOfType(Obj::ObjType type, const std::string &name) {
  if(this->type() == type && name_ == name) {
    return this;
  } 
  ObjList::iterator it;
  for(it=childs_.begin(); it!= childs_.end(); it++) {
     Obj* ret=NULL;
     if(ret=(*it)->getChildOfType(type, name)) {
       return ret;
     }
  }
  return NULL;
}

//==============================================================================
///      \class   Class
///      \method   
///      \brief   Constructor
//==============================================================================
Class::Class(FileId &id, const std::string &a_name, ClassId &cid) :
  Obj(SCA_FILE(id), Obj::kClass, a_name) {
  id_    = cid;
  width_ = -1;
  depth_ = -1;
  inherited_to_.clear();
}

//==============================================================================
///      \class   Class
///      \method  
///      \brief   Constructor
//==============================================================================
Class::Class(File *file, const std::string &a_name, ClassId &cid) :
  Obj(file, Obj::kClass, a_name) {
  id_    = cid;
  width_ = -1;
  depth_ = -1;
  inherited_to_.clear();
}

//==============================================================================
///      \class   Class
///      \method  fileId
///      \brief   Get the file id where the class is defined
//==============================================================================
FileId Class::fileId(void) {
  assert(this->parent()->type() == Obj::kFile);
  File *file = static_cast<File*>(this->parent());
  return file->id();
}

//==============================================================================
///      \class   File
///      \method  
///      \brief  Constructor
//==============================================================================
File::File(PkgId &pkg_id, 
           const std::string &a_file_name, 
           const FileId a_id, 
           const Obj::ObjType a_type) : 
      Obj(SCA_PKG(pkg_id), a_type, a_file_name), 
          fileId_(a_id), 
          type_(a_type) { 
}

//==============================================================================
///      \class   File
///      \method  
///      \brief   Constrcutor2
//==============================================================================
File::File(Package *pkg, 
           const std::string &a_file_name, 
           const FileId a_id, 
           const Obj::ObjType a_type) : 
      Obj(pkg, a_type, a_file_name), 
      fileId_(a_id), 
      type_(a_type) { 
}

//==============================================================================
///      \class   File 
///      \method  pkgId
///      \brief   Gets the Package to which the file belongs
//==============================================================================
PkgId File::pkgId(void) const { 
  assert(this->parent()->type() == Obj::kPackage);
  Package *pkg = static_cast<Package*>(this->parent());
  return pkg->id();
}

//==============================================================================
///      \class   File
///      \method  insert_direct_include
///      \brief   Inserts the file "id" as dircet include of current file
//==============================================================================
void File::insert_direct_include(FileId id) {
  direct_includes_.insert(id); 
}

//==============================================================================
///      \class   File
///      \method  get_full_name
///      \brief   Gets the full name of the file i.e pkg_name + file_name
//==============================================================================
std::string File::get_full_name(void) {
  std::string full_name = SCA_PKG(this->pkgId())->name();
  if(full_name.size()) {
    full_name += "/";
  }
  full_name += name_.c_str();
  return full_name;
}

//#############################################################################
//ALL DUMP ROUTINES FOLLOW THIS LINE 
//#############################################################################
//
//==============================================================================
///      \class   File
///      \method  dump
///      \brief   dump the File info on stdout
//==============================================================================

void Package::dump(bool isLast) {
  fprintf(SCA_GET_WRITE_PTR(), "    {\n");
  fprintf(SCA_GET_WRITE_PTR(), "       \"%s\" : \"%s\",\n", PackageName, name_.c_str());
  fprintf(SCA_GET_WRITE_PTR(), "       \"%s\" : [\n", PackageSourceFile);
  Obj::dump(Obj::kSourceFile); // Array of surce files
  fprintf(SCA_GET_WRITE_PTR(), "               ],\n");
  fprintf(SCA_GET_WRITE_PTR(), "       \"%s\" : [\n", PackageHeaderFile);
  Obj::dump(Obj::kHeaderFile); // Array of header files
  fprintf(SCA_GET_WRITE_PTR(), "               ]\n");
  fprintf(SCA_GET_WRITE_PTR(), "    }%s\n", (isLast ? "" : ","));
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void File::dump(bool isLast) {
  fprintf(SCA_GET_WRITE_PTR(), "          {\n");
  fprintf(SCA_GET_WRITE_PTR(), "            \"%s\": \"%s\",\n", FileName, name_.c_str());
  fprintf(SCA_GET_WRITE_PTR(), "            \"%s\" : [\n", FileFunction);
  Obj::dump(Obj::kFunction); //Array of functions
  fprintf(SCA_GET_WRITE_PTR(), "                       ],\n");
  fprintf(SCA_GET_WRITE_PTR(), "            \"%s\" : [\n", FileClass);
  Obj::dump(Obj::kClass);  // Array of classes
  fprintf(SCA_GET_WRITE_PTR(), "                    ]\n");
  fprintf(SCA_GET_WRITE_PTR(), "          }%s\n", (isLast ? "" : ","));
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void SourceFile::dump(Obj::ObjType type, bool isLast) {
  File::dump(isLast);
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void HeaderFile::dump(Obj::ObjType type, bool isLast) {
    File::dump(isLast);
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void Class::dump(Obj::ObjType type, bool isLast) {
  fprintf(SCA_GET_WRITE_PTR(), "              {\n");
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\": \"%s\",\n", ClassName, name_.c_str());
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"   : \"%s:%lld\",\n", ClassIdentity, id_.file_name().c_str(), id_.line_no());
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"   : %d,\n", ClassWidth, width_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"   : %d,\n", ClassDepth, depth_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\": %d,\n", ClassisAggregate, isAggregate_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", ClassisCLike, isCLike_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", ClassisEmpty, isEmpty_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", ClassisPolymorphic, isPolymorphic_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\": %d,\n", ClassisAbstract, isAbstract_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", ClassisTrivial, isTrivial_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", ClassisLiteral, isLiteral_);
  fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : [\n", ClassMethod);
  Obj::dump(Obj::kMethod);
  fprintf(SCA_GET_WRITE_PTR(), "                         ]\n");
  fprintf(SCA_GET_WRITE_PTR(), "              }%s\n", (isLast ? "" : ","));
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void Function::dump(bool isLast) {
  if(this->type() == Obj::kFunction) {
    fprintf(SCA_GET_WRITE_PTR(), "              {\n");
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\": \"%s\",\n", FunctionName, name_.c_str());
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"   : \"%s:%lld\",\n", FunctionId, id_.file_name().c_str(), id_.line_no());
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", FunctionCyclomatic, cyclomatic_);
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\" : %d,\n", FunctionComplexity, complexity_);
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"  : %d,\n", FunctionMaxNesting, max_nesting_);
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"  : %d,\n", FunctionNumLines, num_lines_);
    fprintf(SCA_GET_WRITE_PTR(), "                \"%s\"  : %d\n", FunctionParamSize, param_size_);
    fprintf(SCA_GET_WRITE_PTR(), "              }%s\n", (isLast ? "" : ","));
  } else {
    fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\" : %d,\n", FunctionCyclomatic, cyclomatic_);
    fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\" : %d,\n", FunctionComplexity, complexity_);
    fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\"  : %d,\n", FunctionMaxNesting, max_nesting_);
    fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\"  : %d,\n", FunctionNumLines, num_lines_);
    fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\"  : %d\n", FunctionParamSize, param_size_);
  }
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void Function::dump(Obj::ObjType type, bool isLast) {
   Function::dump(isLast);
}

//==============================================================================
///      \class   
///      \method  
///      \brief  
//==============================================================================
void Method::dump(Obj::ObjType type, bool isLast) {
  fprintf(SCA_GET_WRITE_PTR(), "                  { \n");
  fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\": \"%s\",\n",MethodName, name_.c_str());
  fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\"   : \"%s:%lld\",\n", MethodId, id_.file_name().c_str(), id_.line_no());
  fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\": %d,\n", MethodisConstant, isConst_);
  fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\": %d,\n", MethodisStatic, isStatic_);
  fprintf(SCA_GET_WRITE_PTR(), "                    \"%s\": %d,\n", MethodisVirtual, isVirtual_);
  Function::dump(isLast);
  fprintf(SCA_GET_WRITE_PTR(), "                  }%s\n", (isLast ? "" : ","));
}
