/******************************************************************************
 Copyright (c) 2017-2020, Ioannis Nompelis
 All rights reserved.

 Redistribution and use in source and binary forms, with or without any
 modification, are permitted provided that the following conditions are met:
 1. Redistribution of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistribution in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
    must display the following acknowledgement:
    "This product includes software developed by Ioannis Nompelis."
 4. Neither the name of Ioannis Nompelis and his partners/affiliates nor the
    names of other contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 5. Redistribution or use of source code and binary forms for profit must
    have written permission of the copyright holder.
 
 THIS SOFTWARE IS PROVIDED BY IOANNIS NOMPELIS ''AS IS'' AND ANY
 EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL IOANNIS NOMPELIS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#ifndef _INTECPLOT_H_
#define _INTECPLOT_H_

#include <stdio.h>
#include <stdlib.h>

#include <list>
#include <vector>
#include <map>
#include <string>


#ifdef __cplusplus
extern "C" {
#endif


//
// a zone object
//

class inTec_File;  // forward declaration

class inTec_Zone {
 public:
   inTec_Zone( inTec_File* file_ );
   virtual ~inTec_Zone();

   int GetState( void ) const;
   int GetType( void ) const;     // return value is enum-ed
   long GetPositionInFile( void ) const;
   int NotifyPositionInFile( long ipos_ );
   long GetDataPositionInFile( void ) const;
   int NotifyDataPositionInFile( long ipos_data_ );
   int ParseKeywords( char *buf );
   int HandleKeyword( char *buf );
   int SetState_Reading( void );
   int ParseNumericData( char *buf );

   int Dump( const char *file ); 

   int WriteFileSTL( char filename_[], double rdir ) const;
   const double* GetVariablePtr( int ivar_ ) const;
   int GetNumVariables() const;
   unsigned long GetNumIndices( int idir ) const;
   unsigned long GetNumNodes() const;
   unsigned long GetNumElements() const;
   unsigned long GetNumNodesPerElement() const;
   const unsigned long* GetConnectivityPtr() const;

   void clear();

 protected:
   void InitKeywords();
   int istate;
   char *tkey;
   int ietype,idatapack;
   int num_var, parent_zone, nv;
   unsigned long im,jm,km,nodes,elems;
   std::map< int, int > ivar_loc;
   std::vector< double * > var_vec;
   unsigned long *icon,ncon;

   // old-style variables
   unsigned long iold_n, iold_e;
   int iold_et, iold_f;

 private:
   inTec_File *file;
   long ipos,ipos_data,ipos_conn;
   std::map< std::string, std::string > keywords;

   int CheckCharForNum( char c ) const;
   int ManageInternals( void );
   int HandleKeyword_T( const char *string );
   int HandleKeyword_ET( const char *string );
   int HandleKeyword_Varlocation( const char *string );
   int HandleKeyword_Datapacking( const char *string );
   int HandleKeyword_Zonetype( const char *string );
   int HandleKeyword_Varsharelist( const char *string );
   int ConsistencyCheck( void );

   // variables to aid in the parsing of numeric part
   int iparse_num;
   unsigned long node_cnt, elem_cnt;
   int var_cnt;
   unsigned long icon_cnt;

   // various methods that act as helpers
   int GetElementNodes( unsigned long n, int & no, unsigned long *icon_ );
};


//
// a file object
//

class inTec_File {
 public:
   inTec_File( char *fname_ );
   virtual ~inTec_File();

   int OpenRead();
   int Close();
   int Parse();

   int GetState( void ) const;
   FILE* GetFP( void ) const;
   int GetNumVariables( void ) const;
   const char* GetVariableName( int nvar_ ) const;
   inTec_Zone* GetZone( int i );

 protected:
   int ParseLoop();
   int IdentifyComponent( char *line_buf );
   int IsComponent( char *line_buf );
   int ParseComponent_Header( int iop, char *line_buf );
   int ParseComponent_HeaderVariables( char *line_buf );
   int ParseComponent_Zone();
   int ParseComponent_Text();
   int ParseComponent_Geometry();

 private:
   int istate;
   char name[256];    // "infinity is set to 256"
   FILE *fp;
   long nline,iline;
   long ipos;

   std::map< unsigned long, std::string > strings;
   std::vector< std::string > variables;
   std::vector< inTec_Zone * > zones;
// std::vector< inTec_Text * > texts;
// std::vector< inTec_Geometry * > geometries;

};




#ifdef __cplusplus
}
#endif

#endif

