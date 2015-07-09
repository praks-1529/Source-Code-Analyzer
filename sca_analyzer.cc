/*! ----------------------------------------------------------
 *
 *  \file       sca_analyzer.cc
 *
 *  \brief
 *      The analyzer of the SCA
 *
 *  \details
 *      Detailed description of file
 *      
 *  \history
 *      06/09/14 08:48:19 PDT Created By Prakash S
 *
 *  ----------------------------------------------------------*/
#include "sca_int.hh"
#include "sca_utils.hh"
#include "sca_globals.hh"

//==============================================================================
///      \method recursive_depth_width 
///      \brief Recursive function to calculate the depth
//==============================================================================
int recursive_depth_width(SCA::Class *obj, int depth) {
  SCA::ClassList &inherited_to = obj->inherited_to();
  /// Set the width first
  obj->set_width(inherited_to.size());
  /// Now recursively find the depth
  int max_depth = depth;
  for(SCA::ClassList::iterator it=inherited_to.begin(); it!=inherited_to.end(); it++) {
    int cur_depth = recursive_depth_width((*it), (depth+1)); 
    if(cur_depth > max_depth) {
      max_depth = cur_depth;
    }
  }
  return max_depth;
}

//==============================================================================
///      \method calculate_width_depth 
///      \brief  Calculate the inheritance depth of all the class
//               TODO: Highly inefficient code. Will revisit it later
//==============================================================================
void 
calculate_width_depth(void) {
  std::map<SourceId, SCA::Class*> &all_classes = SCA_globals::getGlobals()->class_hash().container();  
  std::map<SourceId, SCA::Class*>::iterator it;
  for(it=all_classes.begin(); it!= all_classes.end(); it++) {
    SCA::Class *cur_obj = it->second;;
    if(cur_obj->depth() == -1) {
      int depth = recursive_depth_width(cur_obj, 0);
      cur_obj->set_depth(depth);
    }
  }
}

//==============================================================================
///      \method sca_analyzer 
///      \brief  Do all the globals analysis after parsing 
//               1. Caculate the depth/width of inheritance tree
//==============================================================================
void sca_analyzer(void) {
  /// Calculate the width/depth of the inheritance tree
  calculate_width_depth();
  const char* client_root = getenv("SCA_CLIENT_ROOT");
  std::vector<SCA::Package*> interested_packages;
  for(PkgId id=0; id < PkgIdMgr::getPkgIdMgr()->cPkgs(); id++) {
     std::string pkg_name = SCA_PKG(id)->name();
     if(client_root) {
       if(std::string::npos == pkg_name.find(client_root, 0)) {
         continue;
       }
     }
     interested_packages.push_back(SCA_PKG(id));
  }
  /// Dump the output
  fprintf(SCA_GET_WRITE_PTR(), "{\n");
  fprintf(SCA_GET_WRITE_PTR(), "  \"pkg\" : [\n");
  for(int i=0; i<interested_packages.size(); i++) {
    interested_packages[i]->dump(i == (interested_packages.size()-1));
  }
  fprintf(SCA_GET_WRITE_PTR(), "            ]\n");
  fprintf(SCA_GET_WRITE_PTR(), "}\n");
}
