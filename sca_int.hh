/*! ----------------------------------------------------------
 *
 *  \file       sca_int.h
 *
 *  \brief
 *      Internal header file
 *
 *  \details
 *      Detailed description of file
 *
 *  \history
 *      05/13/14 21:29:46 PDT Created by Prakash S
 *
 *  ----------------------------------------------------------*/
#ifndef SCA_INT_H
#define SCA_INT_H

/*--------------------------------------------------------------
 *
 *      Includes
 */
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <set>

typedef unsigned int U;
typedef unsigned long UL;
typedef unsigned long long ULL;
typedef long long LL;
typedef U FileId;
typedef U PkgId;
typedef U LineId;

#define INVALID_FILE_ID (U)(-1)
#define INVALID_PKG_ID (U)(-1)
#define INVALID_CLASS_ID (U)(-1)
#define INVALID_FUNC_ID (U)(-1)
#define INVALID_LINE_ID (U)(-1)

#endif    /* SCA_INT_H */
