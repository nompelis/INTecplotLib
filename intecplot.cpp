
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils.h>

#include "intecplot.h"

#ifdef __cplusplus
extern "C" {


enum inTecFile_Component {
   Unknown = 0,

   Title = 11,
   Filetype = 12,
   FiletypeFull = 12001,
   FiletypeGrid = 12002,
   FiletypeSolution = 12003,
   Variables = 13,

   Zone   = 21,

   Text   = 31,

   Geometry = 41,

   Custom_Label = 51,
   Dataset_Auxilary = 61,
   Variable_Auxilary = 71,

   Comment = 100
};


//
// the zone object methods
//

inTec_Zone::inTec_Zone( inTec_File* file_ )
{
#ifdef _DEBUG_
   printf(" i Zone object instantiated\n");
#endif
   istate = 0;
   ipos = 0;
   ipos_data = 0;
   InitKeywords();

   file = file_;
}

inTec_Zone::~inTec_Zone( )
{
#ifdef _DEBUG_
   printf(" i Zone object deconstructed\n");
#endif

#ifdef _DEBUG_
   printf(" i Keywords in zone (%ld) \n", keywords.size());
   printf("   Index  Keyword=String \n");
   std::map< std::string, std::string > :: iterator im;
   int i=0;
   for( im = keywords.begin(); im != keywords.end(); ++im ) {
      printf("   %d  ", i++ );
      printf("   [%s] = %s\n", (*im).first.c_str(), (*im).second.c_str() );
   }
#endif
   keywords.clear();
}

void inTec_Zone::InitKeywords()
{
   // add TecPlot keywords with default values
// keywords["T"] = "";
   keywords["ZONETYPE"] = "ORDERED";
// keywords["I"] = "";
// keywords["J"] = "";
// keywords["K"] = "";
// keywords["NODES"] = "";
// keywords["ELEMENTS"] = "";
// keywords["FACES"] = "";
// keywords["TOTALNUMFACENODES"] = "";
// keywords["NUMCONNECTEDBOUNDARYFACES"] = "";
// keywords["TOTALNUMBOUNDARYCONNECTIONS"] = "";
// keywords["FACENEIGBORMODE"] = "LOCALONETOONE";
// keywords["FACENEIGHBORCONNECTIONS"] = "";
   keywords["DT"] = "SINGLE";
   keywords["DATAPACKING"] = "BLOCK";
   keywords["VARLOCATION"] = "NODAL";
// keywords["VARSHARELIST"] = "";
// keywords["NV"] = "";
// keywords["CONNECTIVITYSHAREZONE"] = "";
// keywords["STRANDID"] = "";
// keywords["SOLUTIONTIME"] = "";
// keywords["PARENTZONE"] = "";
// keywords["PASSIVEVARLIST"] = "";
// keywords["AUXDATA"] = "";
// keywords[""] = "";
}


long inTec_Zone::GetPositionInFile( void ) const
{
   return( ipos );
}

int inTec_Zone::SetPositionInFile( long ipos_ )
{
   // reject reseting of position if it is already set
   if( ipos != 0 ) {
      return(1);
   } else {   // this is for when a zone is created but not read from a file
      ipos = ipos_;
      return(0);
   }
}

long inTec_Zone::GetDataPositionInFile( void ) const
{
   return( ipos_data );
}

int inTec_Zone::SetDataPositionInFile( long ipos_data_ )
{
   // reject reseting of position if it is already set
   if( ipos_data != 0 ) {
      return(1);
   } else {   // this is for when a zone is created but not read from a file
      ipos_data = ipos_data_;
#ifdef _DEBUG_
      printf(" --- Zone's data position at: %ld \n", ipos_data );
#endif
      return(0);
   }
}

int inTec_Zone::SetState_Reading( void )
{
   if( istate == 0 ) {
      istate = 1;
      return(0);
   }

   return(1);
}

int inTec_Zone::ParseKeywords( char *buf )
{
#ifdef _DEBUG_
   printf(" i Parsing zone keywords \n");
#endif

/******* this is to make it seem like it parsed a number of items
static int itime = 0;
++itime;
if(itime < 2) return(1);
*/

   size_t isize=0;
   while( buf[isize] != '\0' && buf[isize] != '\n' ) ++isize;

   char *data;
   // create a duplicate buffer
   data = (char *) malloc(((size_t) (isize+1))*sizeof(char));
   if( data == NULL ) return(-1);

   // copy line to buffer and add termination character
   memcpy( data, buf, isize );
   data[isize] = '\0';
#ifdef _DEBUG2_
   printf("DATA: %s\n",data);
#endif

   // keep parsing for keywords and values
   int np=0;
   int iquote=0, ikey=0, ierror=0, iparse=0;
   char *s=data, *es=NULL;
   size_t n=0;
   while( n <= isize ) {
#ifdef _DEBUG_
      printf(" (key: %d, quote: %d)  ", ikey,iquote);
      printf("  %c   s: %c   ", data[n],s[0]);
  //  printf("  %c   s: %c    (key: %d, quote: %d)  ", data[n],s[0],ikey,iquote);
#endif
      // space or comma as keyword delimeter
      if( data[n] == ' ' || data[n] == ',' ) {
         if( ikey == 0 ) {
              s = &( data[n+1] );
         } else if( ikey == 1 ) {
            if( data[n] == ',' ) { ierror = 1;printf("This is in error! (1)"); }
         } else if( ikey == 2 ) {
            if( data[n] == ',' ) { ierror = 1;printf("This is in error! (1b)");}
         } else if( ikey == 3 ) {
            if( iquote == 1 ) {
               // we continue parsing because the quote is open
            } else {
               // this terminates the keyword
               ikey = 0;
               // parsing should take place
               iparse = 1;
            }
         }

      } else 

      // equal sign
      if( data[n] == '=' ) {
         if( ikey == 0 ) {
            ierror = 1; printf("This is in error! (2)");
         } else if( ikey == 1 ) {
            // we are moving to the second stage
            es = &( data[n] );
            ikey = 2;
#ifdef _DEBUG_
            printf(" (equal sign) ");
#endif
         } else if( ikey == 2 ) {
            ierror = 1; printf("This is in error! (2b)");
         } else if( ikey == 3 ) {
            if( iquote == 1 ) {
               // we continue to swallow characters
            } else {
               ierror = 1; printf("This is in error! (3)");
            }
         }
      } else

      // double quote
      if( data[n] == '\"' ) {
         if( ikey < 2 ) {
            ierror = 1; printf("This is in error! (4) (stray quote) ");
         } else if( ikey == 2 ) {
            // entering data section of this keyword
            ikey = 3;
#ifdef _DEBUG_
            printf(" (start data of keyword) ");
#endif
            // open quote
            iquote = 1;
         } else {
            if( data[n-1] == '\\' ) {
               // simply pick it up as a character
            } else {
               // it is a functional quote (marker)
               if( iquote == 1 ) {
                  // close the quote
                  iquote = 0;
             //   // indicate that we can parse
             //   ikey = 0;
             //   iparse = 1;
                  // let the next encounter terminate the parsing
               } else {
                  ierror = 1; printf("This is in error! (4b) (stray quote) ");
               }
            }
         }
      } else 

      // newline or null
      if( data[n] == '\n' || data[n] == '\0' ) {
         if( ikey == 3 ) {
            if( iquote == 1 ) {
               ierror = 1;printf("This is in error! (5) (unterminated quote) ");
            } else {
               ikey = 0;
               iparse = 1;
#ifdef _DEBUG_
               printf(" (parsing ending) ");
#endif
            }
         } else if( ikey == 2 ) {
               ierror = 1; printf("This is in error! (5b) (empty keyword) ");
         } else if( ikey == 1 ) {
               ierror = 1; printf("This is in error! (5b) (empty keyword) ");
         } else {
            // termination of parsing everything
#ifdef _DEBUG_
            printf(" (parsing ending) ");
#endif
         }
      } else 

      // anything else
      {
         if( ikey == 0 ) {
            // check whether we are in a line of data
            if( CheckCharForNum( data[n] ) == 1 ) {
               // terminate parsing immediately; the zone header must be done
               free( data );
#ifdef _DEBUG_
               printf(" (numerical string) \n");
#endif
               return( -100 );
            }

            // encountered keyword
            ikey = 1;
#ifdef _DEBUG_
            printf(" (start keyword) ");
#endif
         } else if( ikey == 2 ) {
            // entering data section of this keyword
            ikey = 3;
#ifdef _DEBUG_
            printf(" (start data of keyword) ");
#endif
         }
      }

      // provide some output
      if( iparse == 1 ) {
         printf(" (parsing now)");
         data[n+0] = '\0';//HACK HERE

         int iret = HandleKeyword( s );
         // allow for the possibility of dealiing with this differently...
         if( iret == 0 ) ++np;

         iparse = 0;
      }

      ++n;
#ifdef _DEBUG_
      printf("\n");
#endif
   }

   // drop duplicate buffer
   free( data );

   if( ierror == 0 ) {
      return( np );
   } else {
      return( -10*ierror );
   }
}


int inTec_Zone::HandleKeyword( char *buf )
{
#ifdef _DEBUG_
   printf("\n i Handling keyword \n");
#endif
#ifdef _DEBUG2_
   printf(" --- keyword -->|%s|<-- \n", buf );
#endif

   // shift start of buffer to beginning of the keyword
   char *s = buf;
   while( s[0] == ' ' || s[0] == ',' ) s++;
#ifdef _DEBUG_
   printf(" --- start keyword -->|%s|<-- \n", s );
#endif

   // size to equal sign
   size_t i=0,key_size=0;
   while( s[ i ] != '=' ) {
      // nullify spaces as you go
      if( s[ i ] != ' ' ) {
         ++key_size;
      } else {
         s[key_size] = '\0';
      }
      ++i;
   }
#ifdef _DEBUG_
   printf(" --- Equal size at character %ld \n", i );
#endif
   s[i] = '\0';
#ifdef _DEBUG_
   printf(" --- Clean keyword: --->|%s|<--- \n", s );
#endif

   // need to "shift" past the equal sign and to the beginning of the payload
   char *p = &( s[ i+1] );
   while( p[0] == ' ' && p[0] != '\0' ) p++;
#ifdef _DEBUG_
   printf(" --- Payload: --->|%s|<--- \n", p );
#endif

   // trials to match keyword
   int iret=1;
   if( key_size == 1 ) {
      if( strncasecmp( s, "T", 1 ) == 0 ) {
         keywords["T"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "I", 1 ) == 0 ) {
         keywords["I"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "J", 1 ) == 0 ) {
         keywords["J"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "K", 1 ) == 0 ) {
         keywords["K"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "E", 1 ) == 0 ) {   // old keyword
         keywords["E"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "N", 1 ) == 0 ) {   // old keyword
         keywords["N"] = p;
         iret = 0;
      }
   }

   if( key_size == 2 ) {
      if( strncasecmp( s, "ET", 2 ) == 0 ) {   // old keyword
         keywords["ET"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "DT", 2 ) == 0 ) {
         keywords["DT"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "NV", 2 ) == 0 ) {
         keywords["NV"] = p;
         iret = 0;
      }
   }

   if( key_size == 4 ) {
      if( strncasecmp( s, "NODES", 5 ) == 0 ) {
         keywords["NODES"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "FACES", 5 ) == 0 ) {
         keywords["FACES"] = p;
         iret = 0;
      }
   }

   if( key_size == 11 ) {
      if( strncasecmp( s, "DATAPACKING", 11 ) == 0 ) {
         keywords["DATAPACKING"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "VARLOCATION", 11 ) == 0 ) {
         keywords["VARLOCATION"] = p;
         iret = 0;
      }
   }

   return( iret );
}

int inTec_Zone::CheckCharForNum( char c ) const
{
   if( c == '0' ) {
      return(1);
   } else if( c == '1' ) {
      return(1);
   } else if( c == '2' ) {
      return(1);
   } else if( c == '3' ) {
      return(1);
   } else if( c == '4' ) {
      return(1);
   } else if( c == '5' ) {
      return(1);
   } else if( c == '6' ) {
      return(1);
   } else if( c == '7' ) {
      return(1);
   } else if( c == '8' ) {
      return(1);
   } else if( c == '9' ) {
      return(1);
   } else if( c == '-' ) {
      return(1);
   } else if( c == '+' ) {
      return(1);
   } else if( c == '.' ) {
      return(1);
   } else {
      return(0);
   }
}

int inTec_Zone::ParseData( char *buf )
{
#ifdef _DEBUG_
   printf(" i Parsing zone data \n");
#endif
#ifdef _DEBUG_
   printf(" STRING: --->|%s|<---\n", buf );
#endif



   return(0);
}


//
// the file object methods
//

inTec_File::inTec_File( char *fname_ )
{
#ifdef _DEBUG_
   printf("File object instantiated \"%s\" \n", fname_ );
#endif

   memset( name, '\0', 256 );
   sprintf( name, "%s", fname_ );
   fp = NULL;
   istate = 0;
   nline = 0;
   iline = 0;
   ipos = 0;

}

inTec_File::~inTec_File()
{
#ifdef _DEBUG_
   printf(" i File object deconstructing \"%s\"\n", name);
#endif

   zones.clear();

#ifdef _DEBUG_
   printf(" i Variable names in file (%ld) follow \n", variables.size());
   for(unsigned int n=0;n<variables.size();++n) {
      printf("   %d \"%s\"\n", n, variables[n].c_str() );
   }

   printf(" i Strings in file (%ld) follow \n", strings.size());
   printf("   Index  Line  String \n");
   std::map< unsigned long, std::string > :: iterator im;
   int i=0;
   for( im = strings.begin(); im != strings.end(); ++im ) {
      printf("   %d  ", i++ );
      printf("   [%ld]   |%s", (*im).first, (*im).second.c_str() );
   }
#endif
   variables.clear();
   strings.clear();

#ifdef _DEBUG2_
   printf(" i Number of variables after clear %ld \n", variables.size());
   printf(" i Number of strings after clear %ld \n", strings.size());
#endif
}


int inTec_File::OpenRead()
{
#ifdef _DEBUG_
   printf(" i Opening file object \"%s\" \n", name );
#endif

   fp = fopen( name, "r" );
   if( fp == NULL ) {
      printf(" e Could not open file \"%s\"\n", name );
      return(1);
   }

   istate = 1;
   iline = 0;

   return(0);
}

int inTec_File::Close()
{
#ifdef _DEBUG_
   printf(" i Closing file object \"%s\" \n", name );
#endif
   fclose( fp );

   istate = 0;
   iline = 0;
   ipos = 0;

   return(0);
}

int inTec_File::GetState( void ) const
{
   return( istate );
}

FILE* inTec_File::GetFP( void ) const
{
   return( fp );
}


int inTec_File::Parse()
{
#ifdef _DEBUG_
   printf(" i Parsing file \"%s\" \n", name );
#endif

   if( istate != 1 ) {
      printf(" e File not in a reading state\n");
      return(1);
   }

   int iret = ParseLoop();



   return(0);
}

int inTec_File::ParseLoop()
{
#ifdef _DEBUG_
   printf(" i Inside \"ParseLoop()\" \n");
#endif

   if( istate != 1 ) {
      printf(" e File not in a reading state\n");
      return(1);
   }

   char *buf=NULL;
   size_t ibuf_size=0;
   char *buf2=NULL;
   size_t ibuf_size2=0;

   while( !feof( fp ) ) {
      int ic;

      // information about the position in the file
      ipos = ftell( fp );
#ifdef _DEBUG_
      printf(" i Position in file at %ld bytes\n", ipos );
#endif

      // prepare the buffer
      memset( buf, '\0', ibuf_size );
      // read a line of data; get count of bytes read
      ic = inUtils_ReadTextline( &ibuf_size, &buf, fp );
      //// check for errors (negative numbers)
#ifdef _DEBUG2_
      printf("IC= %d, STRING: \"%s\"\n", ic, buf );//HACK
#endif

      // copy the string
      if( ibuf_size > ibuf_size2 ) {
//// need a temporary variable here in order to free the memory...
         buf2 = (char *) realloc( buf2, ibuf_size );
         ///// check for errors
         ibuf_size2 = ibuf_size;
      }
      memcpy( buf2, buf, ibuf_size );

      // store string
      if( ic > 0 ) {
         // increase line count
         ++iline;

         // store string in strings map
         // (Presently making the "line number" the key of hte mapping, but
         // it may be more useful to put the position in the file in bytes.)
         strings[ iline ] = buf;
#ifdef _DEBUG2_
         printf(" i Added to strings line %ld \n", iline );
#endif
         // show the line
         printf(" [%ld]  %s", iline, buf );
      }

      // sanitize the line
      inUtils_TrimLeadingSpaceTextline( buf2 );

      // determine what this line signifies
      int itype = IdentifyComponent( buf2 );
      int iret;
#ifdef _DEBUG_
      if( itype == Comment ) printf(" i Component is a comment line \n");
      if( itype == Title ) printf(" i Component is a title line \n");
      if( itype == Filetype ) printf(" i Component is a filetype line \n");
      if( itype == Variables ) printf(" i Component is a variables line \n");
      if( itype == Zone ) printf(" i Component is a zone \n");
      if( itype == Text ) printf(" i Component is a text \n");
      if( itype == Geometry ) printf(" i Component is a geometry \n");
#endif
      if( itype == Title ) iret = ParseComponent_Header( 1, buf2 );
      if( itype == Filetype ) iret = ParseComponent_Header( 2, buf2 );
      if( itype == Variables ) iret = ParseComponent_Header( 3, buf2 );
      if( itype == Zone ) iret = ParseComponent_Zone();
      if( itype == Text ) iret = ParseComponent_Text();
      if( itype == Geometry ) iret = ParseComponent_Geometry();

      // check return code (iret)


   }


   // drop the buffer
   free( buf );
   free( buf2 );

   return(0);
}

int inTec_File::IdentifyComponent( char *buf )
{
#ifdef _DEBUG2_
   printf(" i Inside \"IdentifyComponent()\" \n");
#ifdef _DEBUG2_
   printf("%s", buf );
#endif
#endif

   if( buf == NULL ) return(-1);

   size_t isize=0;
   while( buf[isize] != '\n' && buf[isize] != '\0' ) ++isize;
#ifdef _DEBUG2_
   printf(" i Size of buffer: %d \n", (int) isize);
#endif

   if( buf[0] == '\n' ) return(0);

   if( buf[0] == '#' ) return( Comment );

   if( isize >= 4 ) {
      if( strncasecmp( "ZONE", buf, 4 ) == 0 ) return( Zone );
      if( strncasecmp( "TEXT", buf, 4 ) == 0 ) return( Text );
   }

   if( isize >= 5 ) {
      if( strncasecmp( "TITLE", buf, 5 ) == 0 ) return( Title );
   }

   if( isize >= 8 ) {
      if( strncasecmp( "FILETYPE", buf, 8 ) == 0 ) return( Filetype );
      if( strncasecmp( "GEOMETRY", buf, 8 ) == 0 ) return( Geometry );
   }

   if( isize >= 9 ) {
      if( strncasecmp( "VARIABLES", buf, 9 ) == 0 ) return( Variables );

   }




   return(0);
}


int inTec_File::IsComponent( char *buf )
{
   return( IdentifyComponent( buf ) );
}


int inTec_File::ParseComponent_Header( int iop, char *buf )
{
#ifdef _DEBUG_
   if( iop == 1 ) printf(" i Inside \"ParseComponent_Header(Title)\" \n");
   if( iop == 2 ) printf(" i Inside \"ParseComponent_Header(Filetype)\" \n");
   if( iop == 3 ) printf(" i Inside \"ParseComponent_Header(Variables)\" \n");
#endif

   switch( iop ) {
    case(1):


    break;
    case(2):


    break;
    case(3):

      printf(" i Parsing header \"variables\" line \n");
      return( ParseComponent_HeaderVariables( buf ) );

    break;

    default:
      printf(" e Error parsing header component\n");
      return(1);
   }

   return(0);
}


int inTec_File::ParseComponent_HeaderVariables( char *buf )
{
#ifdef _DEBUG_
   printf(" i Inside \"ParseComponent_HeaderVariables\" \n");
#endif
   size_t isize=0;
   while( buf[isize] != '\0' && buf[isize] != '\n' ) ++isize;

   char *data,*s1,*token,*sr;

   // create a duplicate buffer
   data = (char *) malloc(((size_t) (isize+1))*sizeof(char));
   if( data == NULL ) return(-1);

   // copy line to buffer and add termination character
   memcpy( data, buf, isize );
   data[isize] = '\0';

   // first trap the equal sign
   s1 = data;
   token = strtok_r( s1, "=", &sr );
#ifdef _DEBUG2_
   printf("---> stok: |%s| \n",token);
   printf("---> data: |%s| \n",data);
#endif

   // individual variables parsing
   s1 = NULL;
#ifdef _DEBUG2_
printf("Inner loop\n");
#endif
   for(int k=1; ; s1 = NULL, ++k) {
      token = strtok_r( s1, " ", &sr );
      if( token == NULL ) break;
#ifdef _DEBUG2_
      printf("---> starting inner stok: |%s| \n",token);
#endif

      if( token[0] == '\"' ) {
#ifdef _DEBUG2_
         int n=1,inum=1;
         while( token[n] != ' ' && token[n] != '\0' && token[n] != '\n' ) {
            if( token[n] == '\"' ) ++inum;
            ++n;
         }
         printf("   NUMBER OF DOUBLE-QUOTES FOUND: %d \n", inum);
         // show us where parsing stopped
      // sr[-1] = '?';   // put an actual marker
         printf("   TOKEN (temp)          -->%s<-- \n", token );
#endif
         // reset the changes...
         if( sr[-1] != '\"' )   // only in the 
            sr[-1] = ' ';       // reset farthest bound
         s1 = token;   //  s1 = &(token[1]);
#ifdef _DEBUG2_
         printf("   NEW STRING POINTER AT -->%s<-- \n", s1 );
#endif
         token = strtok_r( s1, "\"", &sr);
         if( token == NULL ) break;
//       if( token[0] == '\0' ) break;  // HACK
      }
#ifdef _DEBUG2_
      printf("Instance number %d: |%s|\n", k,token);
#endif

      // deal with string-token here...
      std::string sdum = token;
      variables.push_back( sdum );

#ifdef _DEBUG2_
printf("\n"); usleep(100000);
#endif
   }

   // drop duplicate buffer
   free( data );

   return(0);
}


int inTec_File::ParseComponent_Zone()
{
#ifdef _DEBUG_
   printf(" i Inside \"ParseComponent_Zone()\" \n");
#endif

   // instantiate a zone object
   inTec_Zone *zone = new inTec_Zone( this );
   // check for allocation error... return(-1)

   // set the position in the file for this zone
   int iret = zone->SetPositionInFile( ipos );
   if( iret != 0 ) {
      printf(" e Error setting zone's position: %ld \n", ipos);
      printf("   This should never happen (zone rejects only when new)\n");
      delete zone;
      return(1);
   }

#warning "WORKING HERE"
//----------
   // seek back to the begining of the line that got us here so we can re-parse
   iret = fseek( fp, ipos, SEEK_SET );
   if( iret != 0 ) {
      printf(" e Error seeking to proper parsing position: %ld \n", ipos);
      return(2);
   }
   // retract line-number as the line will be re-read
   --iline;

   // Looking to parse multiple keywords and over multiple lines
   // T= <string>
   // I=
   // J=
   // K=
   // N=  NODES=
   // E=  ELEMENTS=


   // The idea here is to keep ingesting in lines until we find a new component
   // We ingest comments that can interrupt the geometry component
   // Once a new component is detected, we wrap up the process

   char *buf=NULL;
   size_t ibuf_size=0;

   int idone_zone = 0;   // flag for present zone component termination
   int igot_head = 0;    // flag for whether we have a description of the zone
   while( !feof( fp ) && idone_zone <= 1 ) {
      int ic;

      // information about the position in the file
      ipos = ftell( fp );
#ifdef _DEBUG_
      printf(" i Position in file at %ld bytes, line %ld \n", ipos, iline+1 );
#endif

      // prepare the buffer
      memset( buf, '\0', ibuf_size );
      // read a line of data; get count of bytes read
      ic = inUtils_ReadTextline( &ibuf_size, &buf, fp );
      // check error code

      ++iline;

      // sanitize the line
      inUtils_TrimLeadingSpaceTextline( buf );
      size_t isize=0;
      while( isize < ibuf_size && buf[isize] != '\n' ) ++isize;
      if( buf[isize] == '\n' ) buf[isize] = '\0';
#ifdef _DEBUG_
      printf("STRING (%p, %ld):%s\n", buf, isize, buf );
#endif

      // attempt to identify this line as a new component to terminate parsing
      int iparse_line=1;    // default is to parse the line
      iret = IdentifyComponent( buf );
printf("FOUND iret=%d idone_zone=%d \n", iret,idone_zone);//HACK
      // check if we have jumped to a new component under certain conditions...
      if( iret > 0 ) {             // we found a component

         // only allow "stray" comment components to pass through
         if( iret == Comment ) {
#ifdef _DEBUG_
            printf(" --- Found comment (inside Zone component) \n");
#endif
            // we will not be parsing this line
            iparse_line = 0;
            // store comment in strings
            buf[ strlen(buf) ] = '\n';
            strings[ iline ] = buf;
#ifdef _DEBUG_
            printf(" i Added comment to strings line %ld \n", iline );
#endif
            // show the line
            printf(" [%ld]  %s", iline, buf );
         } else {
#ifdef _DEBUG_
            printf(" --- Found component (at _or_ after Zone component)\n");
#endif
            // over-write the "zone" keyword marker if this is the first pass
            if( idone_zone == 0 ) {
               buf[0] = ' '; buf[1] = ' '; buf[2] = ' '; buf[3] = ' ';
            }
            // increment termination condition variable
            ++idone_zone;
            // will not parse new components
            if( idone_zone > 1) iparse_line = 0;
         }
      } else {
#ifdef _DEBUG_
         printf(" --- The line is part of the Zone component \n");
#endif
         iparse_line = 1;
      }

      // parse the line only if we are still parsing the current zone field
      if( iparse_line == 1 ) {
         if( igot_head == 0 ) {
#ifdef _DEBUG_
            printf(" --- Looking for Zone keywords \n");
#endif
            int np = zone->ParseKeywords( buf );
            if( np >= 0 ) {
#ifdef _DEBUG_
               printf(" --- Parsed %d item(s) \n",np);
#endif
               buf[ strlen(buf) ] = '\n';
               strings[ iline ] = buf;
#ifdef _DEBUG2_
               printf(" i Added to strings line %ld \n", iline );
#endif
            }
            if( np == -100 ) {     // special return value!
               // we have parsed through possibly multiple "zone" lines...
               // ...and encountered data
               igot_head = 1;

               // rewind this line
               iret = fseek( fp, ipos, SEEK_SET );
               if( iret != 0 ) {
                  printf(" e Error seeking to parsing position: %ld \n", ipos);
                  free( buf );
                  return(3);
               }
               // retract line-number as the line will be re-read
               --iline;

               // set the data position for this zone
               zone->SetDataPositionInFile( ipos );
               // (here we must put the zone in data-reading particular state)
               if( zone->SetState_Reading() != 0 ) {
                  // respond to the possibility of the zone being mis-used...

               }

            } else if( np < 0 ) {
               // an error in parsing keywords was trapped
               // (This section will have to be changed based on how we want to
               // handle bad keywords, etc.)
            }
         } else { // we have parsed the header completely; now parsing data
#ifdef _DEBUG_
            printf(" --- Collecting Zone data \n");
#endif
            // push the buffer to the zone for data extraction
            // (we must have set the zone to a data-reading state earlier)
            zone->ParseData( buf );
            // possibly respond to errors returned by the zone...

         }

      } // (end of iparse_line=1)
   }

   // drop the buffer
   free( buf );
//----------

// temporarily drop the zone object
#ifdef _DEBUG_
printf("HACK dropping the zone object\n");
#endif
delete zone; //HACK
//printf("EXITING....\n");exit(0);//HACK

#ifdef _DEBUG_
   printf(" i *** Skipping parsing of ZONE component *** \n");
   // we do this by returning control to the main parsing loop
#endif

   return(0);
}


int inTec_File::ParseComponent_Text()
{
#ifdef _DEBUG_
   printf(" i Inside \"ParseComponent_Text()\" \n");
#endif

   // seek back to the begining of the line that got us here so we can re-parse
   int iret = fseek( fp, ipos, SEEK_SET );
   if( iret != 0 ) {
      printf(" e Error seeking to proper parsing position: %ld \n", ipos);
      return( iret );
   }
   // retract line-number as the line will be re-read
   --iline;

   // Looking to parse multiple keywords and possibly over multiple lines
   // T= <string>
   // ZN= <zone to associate the string>
   // X=
   // Y=
   // Z=
   // R=



   // The idea here is to keep ingesting in lines until we find a new component
   // We ingest comments that can interrupt the text component
   // Once a new component is detected, we wrap up the process

   char *field=NULL;
   size_t ifield_size=0,ifield_pos=0;

   char *buf=NULL;
   size_t ibuf_size=0;

   int idone_text = 0;   // flag for present text component termination
   while( !feof( fp ) && idone_text <= 1 ) {
      int ic;

      // information about the position in the file
      ipos = ftell( fp );
#ifdef _DEBUG_
      printf(" i Position in file at %ld bytes, line %ld \n", ipos, iline+1 );
#endif

      // prepare the buffer
      memset( buf, '\0', ibuf_size );
      // read a line of data; get count of bytes read
      ic = inUtils_ReadTextline( &ibuf_size, &buf, fp );
      // check error code

      // sanitize the line
      inUtils_TrimLeadingSpaceTextline( buf );
      size_t isize=0;
      while( isize < ibuf_size && buf[isize] != '\n' ) ++isize;
      if( buf[isize] == '\n' ) buf[isize] = '\0';
#ifdef _DEBUG2_
      printf("STRING (%p, %ld):%s\n", buf, isize, buf );
#endif

      // attempt to identify this line as a new component to terminate parsing
      int iparse_line=1;    // default is to parse the line
      iret = IdentifyComponent( buf );
//printf("FOUND iret=%d idone_text=%d \n", iret,idone_text);//HACK
      // check if we have jumped to a new component under certain conditions...
      if( iret > 0 ) {             // we found a component
         // only allow "stray" comment components to pass through
         if( iret == Comment ) {
#ifdef _DEBUG_
            printf(" --- Found comment (inside Text component) \n");
#endif
            ++iline;
            // we will not be parsing this line
            iparse_line = 0;
            // store comment in strings
            buf[ strlen(buf) ] = '\n';
            strings[ iline ] = buf;
         } else {
#ifdef _DEBUG2_
            printf(" --- Found component (at _or_ after Text component) \n");
#endif
            // increment termination condition variable
            ++idone_text;
            // will not parse new components
            if( idone_text > 1) iparse_line = 0;
         }
      } else {
#ifdef _DEBUG2_
         printf(" --- The line is part of the Text component \n");
#endif
        iparse_line = 1;
      }

      // parse the line only if we are still parsing the current text field
      if( iparse_line == 1 ) {
         ++iline;

         // keep enlarging the "field" block
         ifield_size += isize;
         char *tmpf = (char *) realloc(field, (ifield_size+1)*sizeof(char));
         if( tmpf == NULL ) {
            free( field );
            free( buf );
            return(-2);
         }
         field = tmpf;
         tmpf = &( field[ifield_pos] );
         memset(tmpf, '\0', isize+1);
         for(size_t n=0;n<isize;++n) field[ ifield_pos + n ] = buf[n];
         ifield_pos = ifield_size;
         field[ ifield_pos ] = '\0';

#ifdef _DEBUG2_
         printf(" --- The field so far: |%s|\n", field);
#endif
      }
   }

   // drop the buffer
   free( buf );


   // set all parameters of the text component


   // drop the text storage
   free( field );


   // seek back to the begining of this line and terminate this operation
   iret = fseek( fp, ipos, SEEK_SET );
   if( iret != 0 ) {
      printf(" e Error seeking to proper parsing position: %ld \n", ipos);
      return(2);
   }


#ifdef _DEBUG_
   printf(" i *** Skipping storing of TEXT component *** \n");
   // we do this by returning control to the main parsing loop
#endif

   return(0);
}


int inTec_File::ParseComponent_Geometry()
{
#ifdef _DEBUG_
   printf(" i Inside \"ParseComponent_Geometry()\" \n");
#endif

   // seek back to the begining of the line that got us here so we can re-parse
   int iret = fseek( fp, ipos, SEEK_SET );
   if( iret != 0 ) {
      printf(" e Error seeking to proper parsing position: %ld \n", ipos);
      return(1);
   }
   // retract line-number as the line will be re-read
   --iline;

   // Looking to parse multiple keywords and over multiple lines
   // T= <string>
   // ZN= <zone to associate the string>
   // X=
   // Y=
   // Z=
   // R=



   // The idea here is to keep ingesting in lines until we find a new component
   // We ingest comments that can interrupt the geometry component
   // Once a new component is detected, we wrap up the process

   char *field=NULL;
   size_t ifield_size=0,ifield_pos=0;

   char *buf=NULL;
   size_t ibuf_size=0;

   int idone_geom = 0;   // flag for present geometry component termination
   while( !feof( fp ) && idone_geom <= 1 ) {
      int ic;

      // information about the position in the file
      ipos = ftell( fp );
#ifdef _DEBUG2_
      printf(" i Position in file at %ld bytes, line %ld \n", ipos, iline+1 );
#endif

      // prepare the buffer
      memset( buf, '\0', ibuf_size );
      // read a line of data; get count of bytes read
      ic = inUtils_ReadTextline( &ibuf_size, &buf, fp );
      // check error code

      // sanitize the line
      inUtils_TrimLeadingSpaceTextline( buf );
      size_t isize=0;
      while( isize < ibuf_size && buf[isize] != '\n' ) ++isize;
      if( buf[isize] == '\n' ) buf[isize] = '\0';
#ifdef _DEBUG2_
      printf("STRING (%p, %ld):%s\n", buf, isize, buf );
#endif

      // attempt to identify this line as a new component to terminate parsing
      int iparse_line=1;    // default is to parse the line
      iret = IdentifyComponent( buf );
//printf("FOUND iret=%d idone_geom=%d \n", iret,idone_geom);//HACK
      // check if we have jumped to a new component under certain conditions...
      if( iret > 0 ) {             // we found a component
         // only allow "stray" comment components to pass through
         if( iret == Comment ) {
#ifdef _DEBUG_
            printf(" --- Found comment (inside Geometry component) \n");
#endif
            ++iline;
            // we will not be parsing this line
            iparse_line = 0;
            // store comment in strings
            buf[ strlen(buf) ] = '\n';
            strings[ iline ] = buf;
         } else {
#ifdef _DEBUG2_
            printf(" --- Found component (at _or_ after Geometry component)\n");
#endif
            // increment termination condition variable
            ++idone_geom;
            // will not parse new components
            if( idone_geom > 1) iparse_line = 0;
         }
      } else {
#ifdef _DEBUG2_
         printf(" --- The line is part of the Geometry component \n");
#endif
        iparse_line = 1;
      }

      // parse the line only if we are still parsing the current geometry field
      if( iparse_line == 1 ) {
         ++iline;

         // keep enlarging the "field" block
         ifield_size += isize;
         char *tmpf = (char *) realloc(field, (ifield_size+1)*sizeof(char));
         if( tmpf == NULL ) {
            free( field );
            free( buf );
            return(-2);
         }
         field = tmpf;
         tmpf = &( field[ifield_pos] );
         memset(tmpf, '\0', isize+1);
         for(size_t n=0;n<isize;++n) field[ ifield_pos + n ] = buf[n];
         ifield_pos = ifield_size;
         field[ ifield_pos ] = '\0';

#ifdef _DEBUG2_
         printf(" --- The field so far: |%s|\n", field);
#endif
      }
   }

   // drop the buffer
   free( buf );



   // set all parameters of the geometry component


   // drop the text storage
   free( field );

   // seek back to the begining of this line and terminate this operation
   iret = fseek( fp, ipos, SEEK_SET );
   if( iret != 0 ) {
      printf(" e Error seeking to proper parsing position: %ld \n", ipos);
      return(2);
   }

#ifdef _DEBUG_
   printf(" i *** Skipping storing of GEOMETRY component *** \n");
   // we do this by returning control to the main parsing loop
#endif

   return(0);
}




}
#endif

