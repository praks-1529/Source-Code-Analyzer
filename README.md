# Source Code Analyzer

Overview
--------------------------------------------------------------------------------------------------
Source Code Analyzer (SCA) is a static C/C++ code analyzer that can be used to identify the areas of the code that violate the coding guidelines set by the dev team. 
This tool can be runas a post checkin tool, that can identify any violations caused by the recent checkin. This tool can also be run on a 
nightly basis to track the overall complexity and mantainibility of the code base. 

SCA collects below information about the code

- Cyclomatic complexity of a function/method
- Time complexity of a function/method
- Number of lines in a function/method
- Number of argument to a function
- Maximum nesting present in a function
- Number of TODO, FIXME comments in the code base
- Comment density of source files

This tool uses CLANG (libclang) which is a front-end to LLVM compiler. This is a beta code and you may face issues. Please report it to me and would love to fix them for you. 

How to run SCA
-----------------------------------------------------------------------------------------------------------
It's very simple!! You can use SCA like other compiler (ex gcc/g++,clang). All it takes is to replace your compiler with SCA in your build system. SCA supports most of the gcc/g++ options and hence there won't be any modifications needed in most of the cases. If you specify -o option it dumps the output in the file specified, if not it'll 
dump the output on stdout. Lets take an example (foo.cc) and try to get some data out of this file

foo.cc

````
int main(int argc, char **argv) {
  int x, y;
  for(int x=0; x<10; x++) {
    if(x > 5) {
      for(y=0; y<20; y++) {
        if(y > 10) {
          //Do something here
        }
      }
    }
  }
}
  ````
  To get the data out of this file foo.cc, we use SCA tool like below we use below command
  ````
  $ sca foo.cc
  ````
  Output
------------------------------------------------------------------------------------------------------------------------------------
  The output of SCA is a JSON file that would list the calculated data for each package, functions, methods and class found in the 
  source file. For the above foo.cc, the output is as below
  ````
  
  {
  "pkg" : [
    {
       "ssca_PkgName" : "/remote/srm406/prakashn/tools/sca/src",
       "ssca_sFile" : [
          {
            "ssca_FileName": "foo.cc",
            "ssca_funct" : [
              {
                "ssca_FunctionName": "main",
                "ssca_i"   : "foo.cc:18",
                "ssca_cyc" : 5,
                "ssca_cmp" : 2,
                "ssca_mn"  : 4,
                "ssca_nl"  : 12,
                "ssca_ps"  : 2
              }
                       ],
            "ssca_cls" : [
                    ]
          }
               ],
       "ssca_hFile" : [
               ]
    }
            ]
}
````
Below table indicates the meaning of each tags in the above JSON file

- pkg: An array of all the directories and subdirectories that constitue a product.
- ssca_PkgName    : The name of one directory inside the product
- ssca_sFile      : An array of the source files present inside the package "ssca_PkgName"
- ssca_sHile      : An array of the header files present inside the package "ssca_PkgName"
- ssca_FileName   : Name of the source/header file
- ssca_funct      : An aray of all the functions defined inside "ssca_FileName"
- ssca_FunctionName : Name of the function
- ssca_i          : The id of the function <source_file>:line_num
- ssca_cyc        : Cyclomatic complexity of the function
- ssca_cmp        : Time complexity of the function
- ssca_mn         : Max nesting of the function
- ssca_nl         : Number of lines in a function
- ssca_ps         : Number of input parameters to the function
