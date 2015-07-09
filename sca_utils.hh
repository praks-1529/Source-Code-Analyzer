/*! ----------------------------------------------------------
 *
 *  \file       sca_utils.hh
 *
 *  \brief
 *      All the internal DS used by SCA
 *
 *  \details
 *      Detailed description of file
 *
 *  \history
 *      05/28/14 06:27:43 PDT Created by Prakash S
 *
 *  ----------------------------------------------------------*/
#ifndef SCA_UTILS_H
#define SCA_UTILS_H

#include "sca_int.hh"
#include <assert.h>
#include <clang-c/Index.h>

//--------------------------------------------------------------------------------------
///     \brief Splits the path in to dir and base name
//--------------------------------------------------------------------------------------
void split_path(const std::string &ip, 
                std::string &dir_name, 
                std::string &base_name);

//--------------------------------------------------------------------------------------
///      \brief Tells if the file is a header file/source file/object file
//--------------------------------------------------------------------------------------
bool isFileOfInterest(std::string &file_name);

/*--------------------------------------------------------------
 *
 *      Class Declarations
 */

// ============================================================================
///        \class  SourceId
///        \brief  A combination of file name + line number
// ============================================================================
class SourceId {
  public:
    SourceId() { 
      file_name_.clear();
      line_no_ = INVALID_LINE_ID; 
    }
    SourceId(std::string &file_name, LineId line_id) {
      std::string dir_name, base_name;
      split_path(file_name, dir_name, base_name);
      file_name_ = base_name;
      line_no_   = line_id;
    }
    SourceId(const char* c_file_name, LineId line_id) {
      std::string file_name(c_file_name);
      std::string dir_name, base_name;
      split_path(file_name, dir_name, base_name);
      file_name_ = base_name;
      line_no_   = line_id;
    }
    SourceId(const SourceId& rhs) {
      file_name_ = rhs.file_name_;
      line_no_   = rhs.line_no_;
    }
    bool operator<(const SourceId& rhs) const {
      if(file_name_ < rhs.file_name_) {
        return true;
      } else if(file_name_ > rhs.file_name_) {
        return false;
      } else {
        if(line_no_ < rhs.line_no_) {
          return true;
        } else {
          return false;
        }
      }
    }
    std::string file_name(void) const { return file_name_; }
    LineId line_no(void) const { return line_no_; }
  private:
    std::string file_name_;
    LineId line_no_;
};

typedef SourceId FuncId;
typedef SourceId ClassId;

namespace SCA {

// JSON attribie for Package
#define PackageName "ssca_PkgName"
#define PackageSourceFile  "ssca_sFile"
#define PackageHeaderFile "ssca_hFile"

#define FileName "ssca_FileName"
#define FileFunction  "ssca_funct"
#define FileClass  "ssca_cls"

#define ClassName "ssca_ClassName"
#define ClassIdentity   "ssca_i"
#define ClassWidth "ssca_w"
#define ClassDepth "ssca_d"
#define ClassisPOD ""
#define ClassisCLike "ssca_isC"
#define ClassisEmpty "ssca_isE"
#define ClassisPolymorphic "ssca_isP"
#define ClassisAbstract "ssca_isAb"
#define ClassisTrivial "ssca_isT"
#define ClassisLiteral "ssca_isL"
#define ClassisAggregate "ssca_isAg"
#define ClassisEmpty "ssca_isE"
#define ClassMethod "ssca_method"

#define FunctionName "ssca_FunctionName"
#define FunctionId  "ssca_i"
#define FunctionCyclomatic "ssca_cyc"
#define FunctionComplexity "ssca_cmp"
#define FunctionMaxNesting "ssca_mn"
#define FunctionNumLines "ssca_nl"
#define FunctionParamSize "ssca_ps"

#define MethodName "ssca_MethodName"
#define MethodId  "ssca_i"
#define MethodisConstant "ssca_isC"
#define MethodisStatic "ssca_isS"
#define MethodisVirtual "ssca_isV"

class Obj; //Forward declaration
class Package; 
class File; 
class Class;

typedef std::list<Obj*> ObjList;
typedef std::vector<Obj*> ObjVector;
typedef std::list<Class*> ClassList;
//
//=============================================================================
///       \class  Obj
///       \brief  Base class of all the util classes
//=============================================================================
//
class Obj {
  public:
    /// Enum to tell what kind of object
    enum ObjType {
      kClass=0,
      kFile,
      kHeaderFile,
      kSourceFile,
      kPackage,
      kFunction,
      kMethod,
      kLast // Ensure thsi is the last one always
    };
    ///Ctor
    Obj(Obj* a_parent, Obj::ObjType type, const std::string &name)
      : parent_(a_parent), type_(type), name_(name) {
        if(parent_) {
          parent_->addChild(this);
        }
    }
    /// Gets the parent of current node
    Obj* parent(void) const { return parent_; }
    /// Gets the type of the current nodes
    Obj::ObjType type(void) const { return type_; }
    /// Get all the childs of current node
    const ObjList& getChilds(void) const { return childs_; }
    /// Add a child to the current node
    void addChild(Obj* child);
    /// Get the name of the class
    std::string name(void) const { return name_; }
    /// DUmp the file info
    virtual void dump(bool isLast=false) {
      ObjList::iterator it;
      int tot_size = childs_.size();
      int i=1;
      for(it=childs_.begin(); it!= childs_.end(); it++, i++) {
        (*it)->dump((i == tot_size));
      }
    }
    virtual void dump(ObjType a_type, bool isLast=false) {
      ObjList::iterator it;
      int tot_size = 0;
      for(it=childs_.begin(); it!= childs_.end(); it++) {
        if((*it)->type() == a_type) {
          tot_size++;
        }
      }
      int i=1;
      for(it=childs_.begin(); it!= childs_.end(); it++) {
        if((*it)->type() == a_type) {
          (*it)->dump(a_type, (i == tot_size));
          i++;
        }
      }
    }
    /// Gets the child that matches "name" and "type"
    virtual Obj* getChildOfType(Obj::ObjType type, const std::string &name);
    /// The name of the file
    std::string name_;
  private:
    /// Type of object
    ObjType type_;
    /// Parent object
    Obj* parent_;
    // Child objects
    ObjList childs_;
};

// =============================================================================
///        \class  Class
///        \brief  Structure that holds class definition. 
///                Hierarchy : Package->File->Class
// =============================================================================
class Class : public Obj {
  public:
    //Ctor1
    Class(FileId &id, const std::string &a_name, ClassId &cid);
    ///Ctor2
    Class(File *file, const std::string &a_name, ClassId &cid);
    /// Get tge id of the class
    ClassId id(void) const { return id_; }
    /// Set the Id
    void set_id(ClassId& id) {
      id_ = id;
    }
    /// Get the file id where the class is defined
    FileId fileId(void);
    /// The dump routine
    virtual void dump(Obj::ObjType type, bool isLast=false);
    /// Add a class to inherited to
    void addInheritedTo(Class *a_class) {
      inherited_to_.push_back(a_class);
    }
    ClassList& inherited_to(void) {
      return inherited_to_;
    }
    /// Set the inheritance width_;
    void set_width(int a_width) {
      width_ = a_width;
    }
    /// Set the inheritance depth_;
    void set_depth(int a_depth) {
      depth_ = a_depth;
    }
    /// Get the depth
    int depth(void) const {
      return depth_;
    }
    /// Get the width
    int width(void) const {
      return width_;
    }
    /// Determine whether class is aggregate
    bool isAggregate (void) const {
      return isAggregate_;
    }
    /// Whether this class has any in-class initializers for non-static data 
    //  members (including those in anonymous unions or structs). 
    bool hasInClassInitializer (void) const {
      return hasInClassInitializer_;
    }
    /// Whether this class or any of its subobjects has any members of reference 
    //  type which would make value-initialization ill-formed. 
    bool hasUninitializedReferenceMember (void) const {
      return hasUninitializedReferenceMember_;
    }
    /// Whether this class is a POD-type 
    bool isPOD (void) const {
      return isPOD_;
    }
    /// True if this class is C-like, without C++-specific features, e.g. it 
    ///  contains only public fields, no bases, tag kind is not 'class', etc. 
    bool isCLike (void) const {
      return isCLike_;
    }
    /// Determine whether this is an empty class
    bool isEmpty (void) const {
      return isEmpty_;
    }
    /// Is the class a polymorphic, which means whether it 
    ///  inherits a virtual function
    bool  isPolymorphic (void) const {
      return isPolymorphic_;
    }
    /// Determine whether this class has a pure virtual function. 
    bool  isAbstract (void) const {
      return isAbstract_;
    }
    /// Determine whether this class has standard layout
    bool  isStandardLayout (void) const {
      return isStandardLayout_;
    }
    /// Determine whether this class, or any of its class subobjects, 
    /// contains a mutable field. 
    bool  hasMutableFields (void) const {
      return hasMutableFields_;
    }
    /// Determine whether this class has any variant members. 
    bool  hasVariantMembers (void) const {
      return hasVariantMembers_;
    }
    /// Determine whether this class has a trivial default constructor 
    bool  hasTrivialDefaultConstructor (void) const {
      return hasTrivialDefaultConstructor_;
    }
    /// Determine whether this class has a non-trivial default constructor 
    bool  hasNonTrivialDefaultConstructor (void) const {
      return hasNonTrivialDefaultConstructor_;
    }
    /// Determine whether this class has at least one constexpr constructor 
    /// other than the copy or move constructors. 
    bool  hasConstexprNonCopyMoveConstructor (void) const {
      return hasConstexprNonCopyMoveConstructor_;
    }
    /// Determine whether a defaulted default constructor for this class 
    /// would be constexpr. 
    bool  defaultedDefaultConstructorIsConstexpr (void) const {
      return defaultedDefaultConstructorIsConstexpr_;
    }
    /// Determine whether this class has a constexpr default constructor. 
    bool  hasConstexprDefaultConstructor (void) const {
      return hasConstexprDefaultConstructor_;
    }
    /// Determine whether this class has a trivial copy constructor 
    bool  hasTrivialCopyConstructor (void) const {
      return hasTrivialCopyConstructor_;
    }
    /// Determine whether this class has a non-trivial copy constructor 
    bool  hasNonTrivialCopyConstructor (void) const {
      return hasNonTrivialCopyConstructor_;
    }
    /// Determine whether this class has a trivial move constructor 
    bool  hasTrivialMoveConstructor (void) const {
      return hasTrivialMoveConstructor_;
    }
    /// Determine whether this class has a non-trivial move constructor 
    bool  hasNonTrivialMoveConstructor (void) const {
      return hasNonTrivialMoveConstructor_;
    }
    /// Determine whether this class has a trivial copy assignment operator 
    bool  hasTrivialCopyAssignment (void) const {
      return hasTrivialCopyAssignment_;
    }
    /// Determine whether this class has a non-trivial copy assignment operator 
    bool  hasNonTrivialCopyAssignment (void) const {
      return hasNonTrivialCopyAssignment_;
    }
    /// Determine whether this class has a trivial move assignment operator 
    bool  hasTrivialMoveAssignment (void) const {
      return hasTrivialMoveAssignment_;
    }
    /// Determine whether this class has a non-trivial move assignment operator 
    bool  hasNonTrivialMoveAssignment (void) const {
      return hasNonTrivialMoveAssignment_;
    }
    /// Determine whether this class has a trivial destructor 
    bool  hasTrivialDestructor (void) const {
      return hasTrivialDestructor_;
    }
    /// Determine whether this class has a non-trivial destructor 
    bool  hasNonTrivialDestructor (void) const {
      return hasNonTrivialDestructor_;
    }
    /// Determine whether this class has a destructor which has no semantic 
    /// effect. 
    bool  hasIrrelevantDestructor (void) const {
      return hasIrrelevantDestructor_;
    }
    /// Determine whether this class has a non-literal or/ volatile type 
    /// non-static data member or base class. 
    bool  hasNonLiteralTypeFieldsOrBases (void) const {
      return hasNonLiteralTypeFieldsOrBases_;
    }
    // Determine whether this class is considered trivially copyable per 
    bool  isTriviallyCopyable (void) const {
      return isTriviallyCopyable_;
    }
    /// Determine whether this class is considered trivial. 
    bool  isTrivial (void) const {
      return isTrivial_;
    }
    ///  Determine whether this class is a literal type. 
    bool  isLiteral (void) const {
      return isLiteral_;
    }
    void set_isAggregate(int is_true) {
      isAggregate_ = is_true;
    }
    void set_isCLike(int is_true) {
      isCLike_ = is_true; 
    }
    void set_isEmpty(int is_true) {
      isEmpty_ = is_true; 
    }
    void set_isPolymorphic(int is_true) {
      isPolymorphic_ = is_true; 
    }
    void set_isAbstract(int is_true) {
      isAbstract_ = is_true; 
    }
    void set_isTrivial(int is_true)  {
      isTrivial_ = is_true; 
    }
    void set_isLiteral(int is_true) {
      isLiteral_ = is_true; 
    }
  private:
    /// Id of the class
    ClassId id_;
    /// This class is the direct base to how many other class
    ClassList inherited_to_;
    /// Width of the inheritance tree
    int width_;
    /// Depth of the inheritance tree
    int depth_;
    // Flags. NOTE: Only 64 flags possible
    U isAggregate_:1,
      hasInClassInitializer_:1,
      hasUninitializedReferenceMember_:1,
      isPOD_:1,
      isCLike_:1,
      isEmpty_:1,
      isPolymorphic_:1,
      isAbstract_:1,
      isStandardLayout_:1,
/*10*/hasMutableFields_:1,
      hasVariantMembers_:1,
      hasTrivialDefaultConstructor_:1,
      hasNonTrivialDefaultConstructor_:1,
      hasConstexprNonCopyMoveConstructor_:1,
      defaultedDefaultConstructorIsConstexpr_:1,
      hasConstexprDefaultConstructor_:1,
      hasTrivialCopyConstructor_:1,
      hasNonTrivialCopyConstructor_:1,
      hasTrivialMoveConstructor_:1,
/*20*/hasNonTrivialMoveConstructor_:1,
      hasTrivialCopyAssignment_:1,
      hasNonTrivialCopyAssignment_:1,
      hasTrivialMoveAssignment_:1,
      hasNonTrivialMoveAssignment_:1,
      hasTrivialDestructor_:1,
      hasNonTrivialDestructor_:1,
      hasIrrelevantDestructor_:1,
      hasNonLiteralTypeFieldsOrBases_:1,
      isTriviallyCopyable_:1,
/*30*/isTrivial_:1,
      isLiteral_:1;
};

// =============================================================================
///        \class  File
///        \brief  Sturcture that holds file info (both source/header files)
///                 Hierarchy : Package->File
// =============================================================================
class File : public Obj {
  public:
    /// Type of files in C++
    enum FileType {
      kHeader=0,
      kSource,
      kObject,
      kArchive,
      kUnknown
    };
    /// Ctor1
    File(PkgId &pkg_id, 
         const std::string &a_file_name, 
         const FileId a_id, 
         const Obj::ObjType a_type);
    ///Ctor2
    File(Package *pkg, 
         const std::string &a_file_name, 
         const FileId a_id, 
         const Obj::ObjType a_type);
    /// Get the id of the file
    FileId id(void) const { return fileId_; }
    /// Get the file type header or Source ?
    Obj::ObjType fileType(void) const { return type_; }
    /// Get the package id where this file belong
    PkgId pkgId(void) const;
    /// Insert the file as direct include of current file
    void insert_direct_include(FileId id);
    /// Get the full name of the file
    std::string get_full_name(void);
    /// DUmp the file info
    virtual void dump(bool isLast=false);
  protected:
    /// Id of the file
    FileId fileId_;
    /// Type of the file header/source
    ObjType type_;
    /// List of direct includes
    std::set<FileId> direct_includes_;
};
 
// =============================================================================
///        \class  SourceFile
///        \brief  Structure that holds the Source file info
// =============================================================================
class SourceFile : public File {
  public:
    ///Ctor1
    SourceFile(PkgId &pkg_id, 
               const std::string &a_file_name, 
               const FileId &a_id) :
      File(pkg_id, a_file_name, a_id, Obj::kSourceFile) {
    }
    ///Ctor2
    SourceFile(Package *pkg, 
               const std::string &a_file_name, 
               const FileId &a_id) :
      File(pkg, a_file_name, a_id, Obj::kSourceFile) {
    }
    /// DUmp the file info
    virtual void dump(Obj::ObjType type, bool isLast=false);
};

// =============================================================================
///        \class  HeaderFile
///        \brief  Structure that holds the header file info
// =============================================================================
class HeaderFile : public File {
  public:
    ///Ctor1
    HeaderFile(PkgId &pkg_id, 
               const std::string &a_file_name, 
               const FileId &a_id) :
      File(pkg_id, a_file_name, a_id, Obj::kHeaderFile) {
    }
    ///Ctor2
    HeaderFile(Package *pkg, 
               const std::string &a_file_name, 
               const FileId &a_id) :
      File(pkg, a_file_name, a_id, Obj::kHeaderFile) {
    }
    /// DUmp the file info
    virtual void dump(Obj::ObjType type, bool isLast=false);
};

// =============================================================================
///        \class  Package
///        \brief  Structure that holds info about the packages aka modules
///                Hierarchy : NULL->Package
// =============================================================================
class Package  : public Obj {
  public:
    /// Ctor
    Package(const std::string &a_name, 
            const PkgId &a_id) 
       : Obj(NULL, Obj::kPackage, a_name) {
      pkgId_ = a_id;
    }
    /// Id of the package
    PkgId id(void) const { return pkgId_; }
    /// DUmp the pkg info
    virtual void dump(bool isLast=false);
  private:
    /// Id of the package
    PkgId pkgId_;
    /// All direct dependent packages
    std::set<PkgId> dependent_packages_;
};

// ============================================================================
///        \class  Function
///        \brief  Represents a C function
// ============================================================================
class Function : public Obj {
  public:
   Function(Obj *a_parent, 
            const std::string &a_name,
            const FuncId &id,
            Obj::ObjType type=Obj::kFunction) 
     : Obj(a_parent, type, a_name), id_(id) {
     cyclomatic_ = 0;
     complexity_ = 0;
     max_nesting_= 0;
     num_lines_  = 0; 
   }
   void set_id(FuncId& id) {
     id_ = id;
   }
   /// Get the cyclomatic complexity of the function
   U cyclomatic(void) const { return cyclomatic_; }
   /// Get the complexity of the function
   U complexity(void) const { return complexity_; }
   /// Get the max nesting inside the function
   U max_nesting(void) const { return max_nesting_; }
   /// Get the number of lines in this function
   U num_lines(void) const { return num_lines_; }
   /// Get the number of parameter to the function
   U param_size() const { return param_size_; }
   /// Set the cyclomatic complexity of the function
   void set_cyclomatic(U cyclomatic) { 
     cyclomatic_ = cyclomatic; 
   }
   /// Set the complexity of the function
   void set_complexity(U complexity) { 
     complexity_ = complexity; 
   }
   /// Set the max nesting inside the function
   void set_max_nesting(U max_nesting) {
     max_nesting_ = max_nesting;
   } 
   /// Set the number of lines in this function
   void set_num_lines(U num_lines) { 
     num_lines_ = num_lines; 
   }
   void set_param_size(U param_size) {
     param_size_ = param_size;
   }
   void addArg(const char *type, const char *var) {
     
   }
   /// Dump the function
   virtual void dump(bool isLast=false); 
   virtual void dump(Obj::ObjType type, bool isLast=false); 

  protected:
   /// Function id
   FuncId id_;

  private:
   /// Cyclomatic complexity of the func
   U cyclomatic_;
   /// Complexity of the func
   U complexity_;
   /// Max nesting in the func
   U max_nesting_;
   /// Number of lines of function
   U num_lines_;
   /// Number of params to the method
   U param_size_;
};

// ============================================================================
///        \class  Method
///        \brief  Represents a C++ method
// ============================================================================
class Method : public Function {
  public:
    Method(Obj* a_parent, 
        const std::string &a_name,
        const FuncId &id) :
      Function(a_parent, a_name, id, Obj::kMethod) {
      isStatic_ = 0;
      isInstance_ = 0;
      isConst_ = 0;
      isVolatile_ = 0;
      isVirtual_ = 0;
    }
    bool isStatic () const {
      return isStatic_;
    }
    bool isInstance () const {
      return isInstance_;
    }
    bool isConst () const {
      return isConst_;
    }
    bool isVolatile () const {
      return isVolatile_;
    }
    bool isVirtual () const {
      return isVirtual_;
    }
    void set_isStatic(int is_true) {
      isStatic_ = is_true;
    }
    void set_isInstance(int is_true) {
      isInstance_ = is_true;
    }
    void set_isConst(int is_true) {
       isConst_ = is_true;
    }
    void set_isVolatile(int is_true) {
       isVolatile_ = is_true;
    }
    void set_isVirtual(int is_true) {
       isVirtual_ = is_true;
    }
    virtual void dump(Obj::ObjType type, bool isLast=false); 
  private:
    /// Flags for the class
    int isStatic_:1,
        isInstance_:1,
        isConst_:1,
        isVolatile_:1,
        isVirtual_:1;
};

class Context {
  public:
    Context() {
      max_nesting_      = 0;
      max_cyclomatic_   = 0;
      max_complexity_   = 0;
    }
    int max_nesting_;
    int cur_nesting_;
    int max_cyclomatic_;
    int max_complexity_;
    int cur_complexity_;
    SCA::Function* cur_func_;
    SCA::Class* cur_class_;
};

} // END SCA namespace

// ============================================================================
///        \class  DoubleHash
///        \brief  Template class to hash objects based on SourceId for quick 
//                 lookup
// ============================================================================
template <typename T>
class DoubleHash {
  public:
    DoubleHash(void) { }
    void insert(SourceId &id, T* obj);
    T* operator[](const SourceId &id);
    void clear(void);
    std::map<SourceId, T*>& container(void) { return container_; }
  private:
    /// The actual container
    std::map<SourceId, T*> container_;
};

//==============================================================================
///      \class   DoubleHash
///      \method  insert
///      \brief   Inserts an element in to the double hash
//==============================================================================
template <typename T>
void DoubleHash<T>::insert(SourceId &id, T* obj) {
  container_[id] = obj;
}

//==============================================================================
///      \class   DoubleHash 
///      \method  operator[]
///      \brief   Retrieves an elemnt from the double hash
//==============================================================================
template <typename T>
T* DoubleHash<T>::operator[](const SourceId &id) {
  if(container_.count(id) == 0) return NULL;
  return container_[id];
}

//==============================================================================
///      \class   DoubleHash 
///      \method  clear
///      \brief   Deletes all the elements in the hash
//==============================================================================
template <typename T>
void DoubleHash<T>::clear(void) {
  typename std::map<SourceId, T*>::iterator it;
  for(it=container_.begin(); it!=container_.end(); it++) {
    delete (it->second);
  }
  container_.clear();
}

//--------------------------------------------------------------------------------------
///      \brief Get absoulte path from relative path
//--------------------------------------------------------------------------------------
void getAbsolutePathFromRelativePath(const char* relative, char* absolute);

//--------------------------------------------------------------------------------------
///      \brief Get SCA::File from CXCursor
//--------------------------------------------------------------------------------------
SCA::File* getFileFromCursor(CXCursor cursor);

//--------------------------------------------------------------------------------------
///      \brief Geti SourceId from the cursor
//--------------------------------------------------------------------------------------
SourceId getSourceIdFromSourceLocation(CXSourceLocation sl);

//--------------------------------------------------------------------------------------
///      \brief Geti SourceId from the cursor
//--------------------------------------------------------------------------------------
SourceId getSourceIdFromCursor(CXCursor cursor);

//--------------------------------------------------------------------------------------
///      \brief Get number of lines defined by the cursor
//--------------------------------------------------------------------------------------
U getLineScopeFromCursor(CXCursor cursor);

//--------------------------------------------------------------------------------------
///      \brief Get whether the given file is source or header 
//--------------------------------------------------------------------------------------
SCA::File::FileType getType(std::string &file_name);

#endif    /* SCA_UTILS_H */

