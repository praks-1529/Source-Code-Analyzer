/*! ----------------------------------------------------------
 *
 *  \file       sca_json_reader.h
 *
 *  \brief
 *      Reads a JSON file and creates SCA DS
 *
 *  \details
 *      Detailed description of file
 *
 *  \history
 *      07/23/14 22:42:08 PDT Created by Prakash S
 *
 *  ----------------------------------------------------------*/

#ifndef SCA_JSON_READER_H
#define SCA_JSON_READER_H
#include <stdint.h>
#include <stack>
#include <string>
#include <sca_utils.hh>
#include <sca_globals.hh>

bool isJsonFile(std::string &file_name);

class NameValuePair  {
  public:
    void clear(void) {
      name_.clear();
      value_.clear();
    }
    std::string name_;
    std::string value_;
};

class CurParseObj {
  public:
    SCA::Package* cur_package_;
    SCA::Function *cur_function_;
    SCA::Class *cur_class_;
    SCA::Method *cur_method_;
    SCA::Obj *cur_file_;
    SCA::Obj *cur_obj_;
    SCA::Obj::ObjType cur_obj_type_; 
    NameValuePair nvpair_;
    std::string cur_obj_name_;
};

class JsonParser {
  public:
    enum ERROR {
      kSuccess=0,
      kNotAJSONFile,
      kFileError,
      kParseInitError,
      kParseSyntaxError
    };
    int parse(std::string &file);
  private:
    static int sca_callback(void *userdata, 
                            int type, 
                            const char *data, 
                            uint32_t length);
    //NOTE: If you add any new type it must be handled here
    //All commit API's
    static void commit_all(NameValuePair &a_nvpair);
    static void commit_package(NameValuePair &a_nvpair);
    static void commit_sFile(NameValuePair &a_nvpair);
    static void commit_hFile(NameValuePair &a_nvpair);
    static void commit_function(NameValuePair &a_nvpair);
    static void commit_class(NameValuePair &a_nvpair);
    static void commit_method(NameValuePair &a_nvpair);
    static std::stack<SCA::Obj*> obj_stack_;
    char sca_json_string_[1024];
    static CurParseObj cur_context_;
    int ret_;
};

#endif    /* SCA_JSON_READER_H */
