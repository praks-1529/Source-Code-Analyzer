/*! ----------------------------------------------------------
 *
 *  \file       test.c
 *
 *  \brief
 *      
 *  ----------------------------------------------------------*/
#include <assert.h>
#include <clang-c/Index.h>
#include "sca_int.hh"
#include "sca_globals.hh"
#include "sca_utils.hh"
#include "sca_json_reader.hh"
#include "sca_analyzer.hh"

CXChildVisitResult FunctionDefinition_visitor(CXCursor cursor,
                                              CXCursor parent,
                                              SCA::Context *cntxt);

//==============================================================================
///      \brief If the statement pointed by the cursor is a loop stmt
//==============================================================================
bool isStmtALoop(CXCursor cursor) {
  CXCursorKind kind = clang_getCursorKind(cursor);
  if(kind == CXCursor_ForStmt ||
      kind == CXCursor_WhileStmt ||
      kind == CXCursor_DoStmt) {
    return true;
  } else {
    return false;
  }
}

//==============================================================================
///      \brief If the statement pointed by the cursor is a branch stmt
//==============================================================================
bool isStmtABranch(CXCursor cursor) {
  CXCursorKind kind = clang_getCursorKind(cursor);
  if(kind == CXCursor_IfStmt ||
      kind == CXCursor_CaseStmt ||
      isStmtALoop(cursor))  {
    return true;
  } else {
    return false;
  }
}

//==============================================================================
///      \brief Visits a statement
//==============================================================================
CXChildVisitResult Stmt_visitor(CXCursor cursor, 
                                  CXCursor parent, 
                                  CXClientData client_data) {
  SCA::Context *cntxt = static_cast<SCA::Context*>(client_data);
  if(isStmtABranch(cursor)) {
    cntxt->max_cyclomatic_++;
    int old_nesting_value       = cntxt->cur_nesting_;
    int old_complexity_value    = cntxt->cur_complexity_;
    cntxt->cur_nesting_++;
    if(isStmtALoop(cursor)) {
      cntxt->cur_complexity_++;
    }
    // Recursively call this function
    clang_visitChildren(cursor, Stmt_visitor, (void*)cntxt);
    if(cntxt->cur_nesting_ > cntxt->max_nesting_) {
      cntxt->max_nesting_ = cntxt->cur_nesting_;
    }
    if(cntxt->cur_complexity_ > cntxt->max_complexity_) {
      cntxt->max_complexity_ = cntxt->cur_complexity_;
    }
    cntxt->cur_nesting_         = old_nesting_value;
    cntxt->cur_complexity_      = old_complexity_value;
    return CXChildVisit_Continue;
  }
  return CXChildVisit_Recurse;
}

//==============================================================================
///      \brief Visits the body of the function 
//==============================================================================
CXChildVisitResult FunctionBody_visitor(CXCursor cursor, 
                                        CXCursor parent, 
                                        CXClientData client_data) {
  SCA::Context *cntxt = static_cast<SCA::Context*>(client_data);
  if(CXCursor_ParmDecl == clang_getCursorKind(cursor)) {
    CXType type = clang_getCursorType(cursor);
    CXString str = clang_getTypeSpelling(type);
    // TODO:Get the type and arg here 
    cntxt->cur_func_->addArg(clang_getCString(str), 
                            clang_getCString(clang_getCursorSpelling(cursor)));
    return CXChildVisit_Recurse;
  }
  if(isStmtABranch(cursor)) {
    cntxt->max_cyclomatic_++;
    cntxt->cur_nesting_ =  1;
    if(isStmtALoop(cursor)) {
      cntxt->cur_complexity_ = 1;
    } else {
      cntxt->cur_complexity_ = 0;
    }
    clang_visitChildren(cursor, Stmt_visitor, (void*)cntxt);
    return CXChildVisit_Continue;
  }
  return CXChildVisit_Recurse;
}

//==============================================================================
///      \brief  Visit the class definition
//==============================================================================
void InclusionDirective_visitor(CXCursor cursor,
                                CXCursor parent,
                                SCA::Context *cntxt) {
  const char* ifile=  clang_getCString(clang_getFileName(clang_getIncludedFile(cursor)));
  if(ifile) {
    char real_path[1024];
    getAbsolutePathFromRelativePath(ifile, real_path);
    FileIdMgr::getFileIdMgr()->insertFile(std::string(real_path)); 
  }
}

//==============================================================================
///      \brief Visits the body of the class
//==============================================================================
CXChildVisitResult ClassBody_visitor(CXCursor cursor, 
                                     CXCursor parent, 
                                     CXClientData client_data) {
  SCA::Context *cntxt = static_cast<SCA::Context*>(client_data);
  if((CXCursor_CXXMethod == clang_getCursorKind(cursor)) &&
      clang_isCursorDefinition(cursor)) {
    return FunctionDefinition_visitor(cursor, parent, cntxt);
  }
  if((CXCursor_CXXBaseSpecifier == clang_getCursorKind(cursor))) {
    CXCursor class_decl = clang_getCursorReferenced(cursor);
    if(CXCursor_ClassDecl != clang_getCursorKind(class_decl)) {
     return CXChildVisit_Continue;
    }
    //assert((CXCursor_ClassDecl == clang_getCursorKind(class_decl)));
    ClassId class_id = static_cast<ClassId>(getSourceIdFromCursor(class_decl));  
    // TODO: Add a SCA error here
    SCA::Class *inherited_from = SCA_globals::getGlobals()->class_hash()[class_id];
    if(NULL == inherited_from) {
      return CXChildVisit_Recurse;
    }
    inherited_from->addInheritedTo(cntxt->cur_class_);
  }
  return CXChildVisit_Recurse;
}

//==============================================================================
///      \brief  Visit the class definition
//==============================================================================
CXChildVisitResult ClassDefinition_visitor(CXCursor cursor,
                                           CXCursor parent,
                                           SCA::Context *cntxt) {
  assert((CXCursor_ClassDecl == clang_getCursorKind(cursor)));
  ClassId class_id = static_cast<ClassId>(getSourceIdFromCursor(cursor));
  SCA::File *file = getFileFromCursor(cursor);
  if(SCA_globals::getGlobals()->class_hash()[class_id] || !file) {
    // Already this function is processed. This function is present in a 
    // header file and hence being parsed once again
  } else {
    SCA::Class* cur_class = new SCA::Class(file,
        clang_getCString(clang_getCursorSpelling(cursor)), 
        class_id);
    cntxt->cur_class_ = cur_class;
    SCA_globals::getGlobals()->class_hash().insert(class_id, cur_class);
    // Visit the class body
    clang_visitChildren(cursor, ClassBody_visitor, (void*)cntxt); 
  }
  return CXChildVisit_Continue;
}

//==============================================================================
///      \brief  Visit the function/Method definition
//==============================================================================
CXChildVisitResult FunctionDefinition_visitor(CXCursor cursor,
                                              CXCursor parent,
                                              SCA::Context *cntxt) {
  assert((CXCursor_CXXMethod == clang_getCursorKind(cursor)) ||
         (CXCursor_FunctionDecl == clang_getCursorKind(cursor)));
  FuncId func_id = static_cast<FuncId>(getSourceIdFromCursor(cursor));
  SCA::File *file = getFileFromCursor(cursor);
  if(SCA_globals::getGlobals()->function_hash()[func_id] || !file) {
    // Already this function is processed. This function is present in a 
    // header file and hence being parsed once again
    return CXChildVisit_Continue;
  }
  SCA::Function* cur_func = NULL;
  if(CXCursor_FunctionDecl == clang_getCursorKind(cursor)) {
    // C-function
    cur_func = new SCA::Function(file, // Parent: SCA::File
                                 clang_getCString(clang_getCursorSpelling(cursor)), // Function name
                                 func_id); // Function Id
    SCA_globals::getGlobals()->function_hash().insert(func_id, cur_func);
  } else {
    // C++ method
    CXCursor class_decl = clang_getCursorSemanticParent(cursor);
    if(CXCursor_ClassDecl != clang_getCursorKind(class_decl)) {
     return CXChildVisit_Continue;
    }
    ClassId class_id = static_cast<ClassId>(getSourceIdFromCursor(class_decl));  
    SCA::Class *parent_class = SCA_globals::getGlobals()->class_hash()[class_id];
#if 0
    if(NULL == parent_class) {
       // Create the class if not present
       ClassDefinition_visitor(class_decl, parent, cntxt);
    }
    parent_class = SCA_globals::getGlobals()->class_hash()[class_id];
#endif
    if(NULL == parent_class) {
      return CXChildVisit_Continue;
    }
    assert(parent_class);
    SCA::Method *cur_method = new SCA::Method(parent_class, 
                                              clang_getCString(clang_getCursorSpelling(cursor)),
                                              func_id);
    cur_method->set_isStatic(clang_CXXMethod_isStatic(cursor));
    cur_method->set_isVirtual(clang_CXXMethod_isVirtual(cursor));
    cur_func = cur_method;
  }
  // Reset all the metrics that would be cauluated for this function
  cntxt->cur_func_           = cur_func;
  cntxt->max_nesting_        = 0;
  cntxt->max_cyclomatic_     = 1;
  cntxt->max_complexity_     = 0;
  cur_func->set_num_lines(getLineScopeFromCursor(cursor));
  cur_func->set_param_size(clang_Cursor_getNumArguments(cursor));
  // Visit the function body
  clang_visitChildren(cursor, FunctionBody_visitor, (void*)cntxt); 
  // Use the calculated metrics 
  cur_func->set_max_nesting(cntxt->max_nesting_);
  cur_func->set_cyclomatic(cntxt->max_cyclomatic_);
  cur_func->set_complexity(cntxt->max_complexity_);
  return CXChildVisit_Continue;
}

//==============================================================================
///      \brief Visits the source file
//==============================================================================
CXChildVisitResult SourceFile_visitor(CXCursor cursor, 
                                      CXCursor parent, 
                                      CXClientData client_data) {
  SCA::Context *cntxt = static_cast<SCA::Context*>(client_data);
  // Visit the function/Method definition
  if((CXCursor_FunctionDecl == clang_getCursorKind(cursor) || 
        CXCursor_CXXMethod == clang_getCursorKind(cursor)) &&
      clang_isCursorDefinition(cursor)) {
    return FunctionDefinition_visitor(cursor, parent, cntxt);
  }

  if(CXCursor_ClassDecl == clang_getCursorKind(cursor) &&
      clang_isCursorDefinition(cursor)) {
    return ClassDefinition_visitor(cursor, parent, cntxt);
  }

  if(CXCursor_InclusionDirective == clang_getCursorKind(cursor)) {
    InclusionDirective_visitor(cursor, parent, cntxt);
    return CXChildVisit_Recurse;
  }
  return CXChildVisit_Recurse;
}

//==============================================================================
///      \brief Cleans up the global vars
//==============================================================================
void clean_up(void) {
  SCA_globals::getGlobals()->clear();
  FileIdMgr::getFileIdMgr()->clear();
  PkgIdMgr::getPkgIdMgr()->clear();
}

int main(int argc, char* argv[]) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stdout, "getcwd() error");
    exit(0);
  }
  bool contains_atleast_one_file = false;
  for(int i=0; i<argc; i++) {
    if(!strcmp(argv[i], "-o")) {
      // The parsed info will be written to this file
      FILE *fp = fopen(argv[i+1], "w");
      if(fp == NULL) {
        exit(0);
      }
      SCA_globals::getGlobals()->set_write_descriptor(fp); 
      i++;
    } else {
      std::string cur_file = std::string(argv[i]);
      if(isFileOfInterest(cur_file) && isJsonFile(cur_file)) {
        fprintf(stdout, "Found a JSON file whiile compiling %s\n", cur_file.c_str());
        JsonParser parser;
        parser.parse(cur_file);  
      } else {
        if(isFileOfInterest(cur_file)) {
          contains_atleast_one_file = true;
        }
      }
    }
  }
  //  If user has not specified -o then set it to stdout
  if(NULL == SCA_GET_WRITE_PTR()) {
    SCA_globals::getGlobals()->set_write_descriptor(stdout);
  }
  // If user has passed atleast one source file then proceed
  if(contains_atleast_one_file) {
    /* Create an index */
    CXIndex Idx;
    Idx = clang_createIndex(0, 1);
    /* Create a translation unit */
    CXTranslationUnit TU;
    TU = clang_createTranslationUnitFromSourceFile(Idx, NULL, argc, argv, 0, 0); 
    /* Get the cursor */
    CXCursor cursor = clang_getTranslationUnitCursor(TU);
    // Create a context
    SCA::Context cntxt;
    // Parse the file
    clang_visitChildren(cursor, SourceFile_visitor, (void*)&cntxt);
    /* Clean up */
    clang_disposeTranslationUnit(TU);
    clang_disposeIndex(Idx);
  }
  sca_analyzer();
  clean_up();
}
