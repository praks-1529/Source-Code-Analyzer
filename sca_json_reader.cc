/*! ----------------------------------------------------------
 *
 *  \file       sca_json_reader.cc
 *
 *  \brief
 *     Implementation of the JSON parser 
 *
 *  \details
 *      Detailed description of file
 *      
 *  \history
 *      07/23/14 22:45:57 PDT Created By Prakash S
 *
 *  ----------------------------------------------------------*/
#include <string>
#include <string.h>
#include <stdio.h>
#include <json.h>
#include "sca_json_reader.hh"

CurParseObj JsonParser::cur_context_;
std::stack<SCA::Obj*> JsonParser::obj_stack_;

int JsonParser::sca_callback(void *userdata, 
                            int type, 
                            const char *data, 
                            uint32_t length) { 
//  FILE *output = (FILE*)((userdata) ? userdata : stdout);
  FILE *output = stdout;
  char value[1024];
  switch (type) {
    case JSON_OBJECT_BEGIN:
      //printf("Begin object: %s\n", data);
      break;
    case JSON_OBJECT_END:
      //printf("Ending object: %s\n", data);
      break;
    case JSON_ARRAY_BEGIN: {
      //printf("Begin array : %s\n", data);
      if(cur_context_.cur_obj_) {
        obj_stack_.push(cur_context_.cur_obj_);
      }
      break;
    }
    case JSON_ARRAY_END: {
      //printf("Ending array : %s\n", data);
      if(obj_stack_.size()) {
        cur_context_.cur_obj_ = obj_stack_.top();
        obj_stack_.pop();
        cur_context_.cur_obj_type_ = cur_context_.cur_obj_->type();
      }
      break;
    }
    case JSON_KEY:
    case JSON_STRING:
    case JSON_INT:
    case JSON_FLOAT: {
      //printf("Object : %s\n", data);
      sprintf(value, "%*s", length, data);
      //NOTE: If you add any new type it must be handled here
      if(!strcmp(value, "pkg")) {
        cur_context_.cur_obj_type_ = SCA::Obj::kPackage; return 0;
      } else if(!strcmp(value, PackageSourceFile)) {
        cur_context_.cur_obj_type_ = SCA::Obj::kSourceFile; return 0;
      } else if(!strcmp(value, PackageHeaderFile)) {
        cur_context_.cur_obj_type_ = SCA::Obj::kHeaderFile; return 0;
      } else if(!strcmp(value, FileFunction)) {
        cur_context_.cur_obj_type_ = SCA::Obj::kFunction; return 0;
      } else if(!strcmp(value, FileClass)) {
        cur_context_.cur_obj_type_ = SCA::Obj::kClass; return 0;
      } else if(!strcmp(value, ClassMethod)) {
        cur_context_.cur_obj_type_ = SCA::Obj::kMethod; return 0;
      } else  {
        if(cur_context_.nvpair_.name_.empty()) {
          cur_context_.nvpair_.name_ = value;
        } else {
          assert(cur_context_.nvpair_.value_.empty());
          cur_context_.nvpair_.value_ = value;
          //Since we have name-vale pair now, commit it
          commit_all(cur_context_.nvpair_);
          //Clear the nvpair to cache next nvpair
          cur_context_.nvpair_.clear();
        }
      }
      break;
    }
  }
  return 0;
}

void JsonParser::commit_all(NameValuePair &a_nv_pair) {
  switch(cur_context_.cur_obj_type_) {
     case(SCA::Obj::kPackage):
       commit_package(a_nv_pair);
       break;
     case(SCA::Obj::kSourceFile):
       commit_sFile(a_nv_pair);
       break;
     case(SCA::Obj::kHeaderFile):
       commit_hFile(a_nv_pair);
       break;
     case(SCA::Obj::kFunction):
       commit_function(a_nv_pair);
       break;
     case(SCA::Obj::kClass):
       commit_class(a_nv_pair);
       break;
     case(SCA::Obj::kMethod):
       commit_method(a_nv_pair);
       break;
  }
}

void JsonParser::commit_package(NameValuePair &a_nvpair) {
  if(!a_nvpair.name_.compare(PackageName)) {
    PkgId pkg_id = (PkgIdMgr::getPkgIdMgr()->insertPkg(a_nvpair.value_)); 
    cur_context_.cur_obj_ = cur_context_.cur_package_ = PkgIdMgr::getPkgIdMgr()->getPkg(pkg_id);
  } else {
    // If this gets hit, the probably you hvae added a new member in class Package
    assert(0);
  }
}

void JsonParser::commit_sFile(NameValuePair &a_nvpair) {
  if(!a_nvpair.name_.compare(FileName)) {
    std::string pkg_name = cur_context_.cur_package_->name();
    pkg_name += "/" + a_nvpair.value_;
    FileId fileId = FileIdMgr::getFileIdMgr()->insertFile(pkg_name); 
    cur_context_.cur_obj_ = cur_context_.cur_file_ = FileIdMgr::getFileIdMgr()->getFile(fileId); 
  } else {
    // If this gets hit, the probably you hvae added a new member in class Package
    assert(0);
  }
}

void JsonParser::commit_hFile(NameValuePair &a_nvpair) {
  if(!a_nvpair.name_.compare(FileName)) {
    std::string pkg_name = cur_context_.cur_package_->name();
    pkg_name += "/" + a_nvpair.value_;
    FileId fileId = FileIdMgr::getFileIdMgr()->insertFile(pkg_name);
    cur_context_.cur_obj_ = cur_context_.cur_file_ = FileIdMgr::getFileIdMgr()->getFile(fileId); 
  } else {
    // If this gets hit, the probably you hvae added a new member in class Package
    assert(0);
  }
}

void JsonParser::commit_function(NameValuePair &a_nvpair) {
  if(!a_nvpair.name_.compare(FunctionName)) {
     cur_context_.cur_obj_name_ = a_nvpair.value_;
  } else if(!a_nvpair.name_.compare(FunctionId)) {
    size_t pos = a_nvpair.value_.find_first_of(':');
    std::string file_name = (a_nvpair.value_.substr(0, pos));
    LineId lineid = atoi((a_nvpair.value_.substr(pos+1, a_nvpair.value_.size()-1)).c_str());
    SourceId func_id(file_name, lineid);
    cur_context_.cur_obj_ = cur_context_.cur_function_ =
      SCA_globals::getGlobals()->function_hash()[func_id];
    if(NULL == cur_context_.cur_function_) {
      cur_context_.cur_obj_ = cur_context_.cur_function_ = 
        new SCA::Function(cur_context_.cur_file_,
                          cur_context_.cur_obj_name_,
                          func_id); 
       SCA_globals::getGlobals()->function_hash().insert(func_id, cur_context_.cur_function_);
    }
  } else if(!a_nvpair.name_.compare(FunctionCyclomatic)) {
    cur_context_.cur_function_->set_cyclomatic(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionComplexity)) {
    cur_context_.cur_function_->set_complexity(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionMaxNesting)) {
    cur_context_.cur_function_->set_max_nesting(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionNumLines)) {
    cur_context_.cur_function_->set_num_lines(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionParamSize)) {
    cur_context_.cur_function_->set_param_size(atoi(a_nvpair.value_.c_str()));
  } else {
    // If this gets hit, the probably you hvae added a new member in class Package
    assert(0);
  }
}

void JsonParser::commit_class(NameValuePair &a_nvpair) {
  if(!a_nvpair.name_.compare(ClassName)) {
    cur_context_.cur_obj_name_ = a_nvpair.value_;
  } else if(!a_nvpair.name_.compare(ClassIdentity)) {
    size_t pos = a_nvpair.value_.find_first_of(':');
    std::string file_name = (a_nvpair.value_.substr(0, pos));
    LineId lineid = atoi((a_nvpair.value_.substr(pos+1, a_nvpair.value_.size()-1)).c_str());
    SourceId class_id(file_name, lineid);
    cur_context_.cur_obj_ = cur_context_.cur_class_ = 
      SCA_globals::getGlobals()->class_hash()[class_id];
    if(NULL == cur_context_.cur_class_) {
      cur_context_.cur_obj_ = cur_context_.cur_class_ = 
        new SCA::Class((SCA::File*)cur_context_.cur_file_,
                       cur_context_.cur_obj_name_,
                       class_id); 
       SCA_globals::getGlobals()->class_hash().insert(class_id, cur_context_.cur_class_);
    }
  } else if(!a_nvpair.name_.compare(ClassWidth)) {
    cur_context_.cur_class_->set_width(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassDepth)) {
    cur_context_.cur_class_->set_depth(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisAggregate)) {
    cur_context_.cur_class_->set_isAggregate(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisCLike)) {
    cur_context_.cur_class_->set_isCLike(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisEmpty)) {
    cur_context_.cur_class_->set_isEmpty(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisPolymorphic)) {
    cur_context_.cur_class_->set_isPolymorphic(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisAbstract)) {
    cur_context_.cur_class_->set_isAbstract(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisTrivial)) {
    cur_context_.cur_class_->set_isTrivial(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(ClassisLiteral)) {
    cur_context_.cur_class_->set_isLiteral(atoi(a_nvpair.value_.c_str()));
  } else {
    // If this gets hit, the probably you hvae added a new member in class Package
    assert(0);
  }
}

void JsonParser::commit_method(NameValuePair &a_nvpair) {
  if(!a_nvpair.name_.compare(MethodName)) {
    cur_context_.cur_obj_name_ = a_nvpair.value_;
  } else if(!a_nvpair.name_.compare(MethodId)) {
    size_t pos = a_nvpair.value_.find_first_of(':');
    std::string file_name = (a_nvpair.value_.substr(0, pos));
    LineId lineid = atoi((a_nvpair.value_.substr(pos+1, a_nvpair.value_.size()-1)).c_str());
    SourceId method_id(file_name, lineid);
    cur_context_.cur_obj_ = cur_context_.cur_method_ = 
      static_cast<SCA::Method*>(SCA_globals::getGlobals()->function_hash()[method_id]); 
    if(NULL == cur_context_.cur_method_) {
      cur_context_.cur_obj_ = cur_context_.cur_method_ = 
        new SCA::Method(cur_context_.cur_class_,
                        cur_context_.cur_obj_name_,
                        method_id); 
      SCA_globals::getGlobals()->function_hash().insert(method_id, cur_context_.cur_method_);
    }
  } else if(!a_nvpair.name_.compare(MethodisConstant)) {
    cur_context_.cur_method_->set_isConst(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(MethodisStatic)) {
    cur_context_.cur_method_->set_isStatic(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(MethodisVirtual)) {
    cur_context_.cur_method_->set_isVirtual(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionCyclomatic)) {
    cur_context_.cur_method_->set_cyclomatic(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionComplexity)) {
    cur_context_.cur_method_->set_complexity(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionMaxNesting)) {
    cur_context_.cur_method_->set_max_nesting(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionNumLines)) {
    cur_context_.cur_method_->set_num_lines(atoi(a_nvpair.value_.c_str()));
  } else if(!a_nvpair.name_.compare(FunctionParamSize)) {
    cur_context_.cur_method_->set_param_size(atoi(a_nvpair.value_.c_str()));
  } else {
    // If this gets hit, the probably you hvae added a new member in class Package
    assert(0);
  }
}

int JsonParser::parse(std::string &file) {
  json_parser parser;
  FILE *fp = fopen(file.c_str(), "r");
  if(!fp) return kFileError;
  bool inside_json=false;
  bool found_atleast_one_json_object = false;
  int line_num=0;
  while (fgets(sca_json_string_, 1024, fp))
  {
    line_num++;
    if(!inside_json && sca_json_string_[0] == '{') {
      inside_json = true;
      if (json_parser_init(&parser, 
                           NULL, 
                           JsonParser::sca_callback, //Call back 
                           this)) { //User data
        return kParseInitError;
      }
    }
    if(inside_json) {
      ret_ = json_parser_string(&parser, 
          sca_json_string_, 
          strlen(sca_json_string_), 
          NULL);
      if(ret_) {
        return kParseSyntaxError;
      }
      found_atleast_one_json_object = true;
      if(sca_json_string_[0] == '}') {
        inside_json = false;
      }
    }
  }
  if(found_atleast_one_json_object) {
    return kSuccess;
  } else {
    return kNotAJSONFile;
  }
}

bool isJsonFile(std::string &file_name) {
  JsonParser parser;
  if(parser.parse(file_name) != JsonParser::kSuccess) {
    return false;
  }
  return true;
}
