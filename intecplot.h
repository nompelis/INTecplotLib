
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


//
// a zone object
//

class inTec_File;  // forward declaration

class inTec_Zone {
 public:
   inTec_Zone( inTec_File* file_ );
   virtual ~inTec_Zone();

   long GetPositionInFile( void ) const;
   int SetPositionInFile( long ipos_ );
   int ParseKeywords( char *buf );
   int HandleKeyword( char *buf );

 protected:
   void InitKeywords();

 private:
   inTec_File *file;
   long ipos,ipos_data;
   std::map< std::string, std::string > keywords;

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




}
#endif

#endif

