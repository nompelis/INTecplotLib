
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils.h>

#include "intecplot.h"

#ifdef __cplusplus
extern "C" {


enum inTecFile_Component {
   Unknown = -1,

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

enum inTecZone_FEtype {
   ORDERED = 0,
   FELINESEG = 1,
   FETRIANGLE = 2,
   FEQUADRILATERAL = 3,
   FEPOLYGON = 4,
   FETETRAHEDRON = 5,
   FEBRICK = 6,
   FEPOLYHEDRAL = 7
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
   ipos_conn = 0;

   // keyword-based variables
   tkey = NULL;
   ietype = -1;
   idatapack = -1;
   im = 0;
   jm = 0;
   km = 0;
   nodes = 0;
   elems = 0;
   nv = 0;
   parent_zone = -1;

   // old-style variables
   iold_n = 0;
   iold_e = 0;
   iold_et = -1;
   iold_f = -1;

   // utility variables
   iparse_num = 0;
   node_cnt = 0;
   elem_cnt = 0;
   var_cnt = 0;
   icon_cnt = 0;

   InitKeywords();

   file = file_;
   num_var = 0;
   icon = NULL;
   ncon = 0;
   // if the zone has a parent file, get the number of variables, etc
   if( file != NULL ) {
      num_var = file->GetNumVariables();
      // default all variables to nodal
      for(int n=1;n<=num_var;++n) ivar_loc[n] = 1;
   }
}

inTec_Zone::~inTec_Zone()
{
#ifdef _DEBUG_
   printf(" i Zone object deconstructed\n");
#endif

   clear();
}

int inTec_Zone::GetState() const
{
   return( istate );
}

void inTec_Zone::clear()
{
#ifdef _DEBUG_
   printf(" i Zone object clearing\n");
#endif

   // deallocate data arrays
   std::vector< double * > :: iterator dpi;
   for( dpi = var_vec.begin() ; dpi != var_vec.end() ; ++dpi ) {
      if( (*dpi) != NULL ) {
         double *tp = (*dpi);
 //HACK  free( tp );
#warning "CLEAN THIS IF ERROR"
         free( tp );
         (*dpi) = NULL;
      }
   }
   var_vec.clear();

   // deallocate connectivity array
   if( icon != NULL ) free( icon );

#ifdef _DEBUG_
   printf(" i Keywords in zone (%ld) \n", keywords.size());
   printf("   Index  Keyword=String \n");
   std::map< std::string, std::string > :: iterator imap;
   int i=0;
   for( imap = keywords.begin(); imap != keywords.end(); ++imap ) {
      printf("   %d  ", i++ );
      printf("   [%s] = %s\n", (*imap).first.c_str(), (*imap).second.c_str() );
   }
#endif
   keywords.clear();

#ifdef _DEBUG_
   printf("   Other data in zone \n");
   printf("    - T: %s \n", tkey);
   printf("    - Nodes: %ld \n", nodes);
   printf("    - Elements: %ld \n", elems);
   printf("    - im,jm,kn: %ld, %ld, %ld \n", im,jm,km);
   printf("    - Number of variables: %d \n", num_var);
   std::map< int, int > :: iterator it;
   for( it= ivar_loc.begin(); it != ivar_loc.end(); ++it ) {
      printf("      ==> %d location %d \n", (*it).first, (*it).second );
   }
#endif
   ivar_loc.clear();

   // drop T-keyword string
   if( tkey != NULL ) free( tkey );


   // initialize variables as if it were a fresh object
   istate = 0;
   ipos = 0;
   ipos_data = 0;
   ipos_conn = 0;

   // keyword-based variables
   tkey = NULL;
   ietype = -1;
   idatapack = -1;
   im = 0;
   jm = 0;
   km = 0;
   nodes = 0;
   elems = 0;
   nv = 0;
   parent_zone = -1;

   // old-style variables
   iold_n = 0;
   iold_e = 0;
   iold_et = -1;
   iold_f = -1;

   // utility variables
   iparse_num = 0;
   node_cnt = 0;
   elem_cnt = 0;
   var_cnt = 0;
   icon_cnt = 0;

   InitKeywords();

   file = NULL;
   num_var = 0;
   icon = NULL;
   ncon = 0;
   num_var = 0;
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
   keywords["VARLOCATION"] = "(NODAL)";
// keywords["VARSHARELIST"] = "";
// keywords["NV"] = "";
// keywords["CONNECTIVITYSHAREZONE"] = "";
// keywords["STRANDID"] = "";
// keywords["SOLUTIONTIME"] = "";
// keywords["PARENTZONE"] = "";
// keywords["PASSIVEVARLIST"] = "";
// keywords["AUXDATA"] = "";
// keywords[""] = "";

   // keywords from the old format
// keywords["E"] = "";
// keywords["N"] = "";
// keywords["ET"] = "";

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
   if( istate != 0 ) {
      // this is an error code that implies prior setting of the state
      return(1);
   }

   // use whatever keywords hava been collected to manage the internal state
   int iret = ManageInternals();
   if( iret == 0 ) {
      istate = 1;
      return(0);
   } else {
      // this is an error code that should imply an inconsistency
      return(2);
   }
}

int inTec_Zone::ParseKeywords( char *buf )
{
#ifdef _DEBUG_
   printf(" i Parsing zone keywords \n");
#endif
   size_t isize=0;
   while( buf[isize] != '\0' && buf[isize] != '\n' ) ++isize;

   char *data;
   // create a duplicate buffer
   data = (char *) malloc(((size_t) (isize+1))*sizeof(char));
   if( data == NULL ) return(-1);

   // copy line to buffer and add termination character
   memcpy( data, buf, isize );
   data[isize] = '\0';
#ifdef _DEBUG3_
   printf("DATA: %s\n",data);
#endif

   // keep parsing for keywords and values
   int np=0;
   int ipar=0, ibra=0, iquote=0, ikey=0, ierror=0, iparse=0;
   char *s=data, *es=NULL;
   size_t n=0;
   while( n <= isize ) {
#ifdef _DEBUG2_
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
            if( iquote == 1 || ibra == 1 || ipar == 1 ) {
               // we continue parsing because the quote/bracket/paren. is open
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
#ifdef _DEBUG2_
            printf(" (equal sign) ");
#endif
         } else if( ikey == 2 ) {
            ierror = 1; printf("This is in error! (2b)");
         } else if( ikey == 3 ) {
            if( iquote == 1 || ibra == 1 || ipar == 1 ) {
               // we continue parsing because the quote/bracket/paren. is open
#ifdef _DEBUG2_
               printf(" (equal sign in key=3) ");
#endif
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
#ifdef _DEBUG2_
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
            if( iquote == 1 || ibra == 1 || ipar == 1 ) {
               ierror = 1;printf("This is in error! (5) (unterminated quote/bracket/paren.) ");
            } else {
               ikey = 0;
               iparse = 1;
#ifdef _DEBUG2_
               printf(" (parsing ending) ");
#endif
            }
         } else if( ikey == 2 ) {
               ierror = 1; printf("This is in error! (5b) (empty keyword) ");
         } else if( ikey == 1 ) {
               ierror = 1; printf("This is in error! (5b) (empty keyword) ");
         } else {
            // termination of parsing everything
#ifdef _DEBUG2_
            printf(" (parsing ending) ");
#endif
         }
      } else 

      // open bracket
      if( data[n] == '[' ) {
         if( ikey < 2 ) {
            ierror = 1; printf("This is in error! (6) (stray bracket) ");
         } else if( ikey == 2 ) {
            // entering data section of this keyword...
            ikey = 3;
            // ...with an open bracket
            ibra = 1;
#ifdef _DEBUG2_
            printf(" (start data of keyword with bracket) ");
#endif
         } else {
            // simply open bracket
            ibra = 1;
#ifdef _DEBUG2_
            printf(" (open bracket) ");
#endif
         }
      } else 

      // close bracket
      if( data[n] == ']' ) {
         if( ikey < 3 ) {
            ierror = 1; printf("This is in error! (7a) (stray bracket) ");
         } else {
            if( ibra != 1 ) {
               ierror = 1; printf("This is in error! (7b) (stray bracket) ");
            } else {
               // simply close bracket
               ibra = 0;
#ifdef _DEBUG2_
               printf(" (close bracket) ");
#endif
            }
         }
      } else

      // open parenthesis
      if( data[n] == '(' ) {
         if( ikey < 2 ) {
            ierror = 1; printf("This is in error! (8) (stray paren.) ");
         } else if( ikey == 2 ) {
            // entering data section of this keyword...
            ikey = 3;
            // ...with an open parenthesis
            ipar = 1;
#ifdef _DEBUG2_
            printf(" (start data of keyword with paren.) ");
#endif
         } else {
            // simply open parenthesis
            ipar = 1;
#ifdef _DEBUG2_
            printf(" (open paren.) ");
#endif
         }
      } else

      // close parenthesis
      if( data[n] == ')' ) {
         if( ikey < 3 ) {
            ierror = 1; printf("This is in error! (9) (stray paren.) ");
         } else {
            if( ipar != 1 ) {
               ierror = 1; printf("This is in error! (9b) (stray paren.) ");
            } else {
               // simply close parenthesis
               ipar = 0;
#ifdef _DEBUG2_
               printf(" (close paren.) ");
#endif
            }
         }
      } else

      // anything else
      {
         if( ikey == 0 ) {
            // check whether we are in a line of data
            if( CheckCharForNum( data[n] ) == 1 ) {
               // terminate parsing immediately; the zone header must be done
               free( data );
#ifdef _DEBUG2_
               printf(" (numerical string) \n");
#endif
               return( -100 );
            }

            // encountered keyword
            ikey = 1;
#ifdef _DEBUG2_
            printf(" (start keyword) ");
#endif
         } else if( ikey == 2 ) {
            // entering data section of this keyword
            ikey = 3;
#ifdef _DEBUG2_
            printf(" (start data of keyword) ");
#endif
         }
      }

      // provide some output
      if( iparse == 1 ) {
#ifdef _DEBUG2_
         printf(" (parsing now)");
#endif
         data[n+0] = '\0';//HACK

         int iret = HandleKeyword( s );
         // allow for the possibility of dealiing with this differently...
         if( iret == 0 ) ++np;

         iparse = 0;
      }

      ++n;
#ifdef _DEBUG2_
      printf("\n");
#endif
   }

   // drop duplicate buffer
   free( data );

   if( ierror == 0 ) {
      return( np );
   } else {
#ifdef _DEBUG_
      printf(" --- Error returned: %d \n", -10*ierror );
#endif
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
   printf(" --- Cleaned keyword: --->|%s|<--- \n", s );
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

      if( strncasecmp( s, "F", 1 ) == 0 ) {   // old keyword
         keywords["F"] = p;
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

   if( key_size == 5 ) {
      if( strncasecmp( s, "NODES", 5 ) == 0 ) {
         keywords["NODES"] = p;
         iret = 0;
      }

      if( strncasecmp( s, "FACES", 5 ) == 0 ) {
         keywords["FACES"] = p;
         iret = 0;
      }
   }

   if( key_size == 8 ) {
      if( strncasecmp( s, "ZONETYPE", 8 ) == 0 ) {
         keywords["ZONETYPE"] = p;
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

   if( key_size == 12 ) {
      if( strncasecmp( s, "VARSHARELIST", 11 ) == 0 ) {
         keywords["VARSHARELIST"] = p;
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

int inTec_Zone::ManageInternals( void )
{
#ifdef _DEBUG_
   printf(" i Inside ManageInternals() \n");
#endif
   int ierror=0,iret=0;

   // processing individual keywords
   const char *string=NULL,*string2=NULL;
   size_t is;
   int i=0;
   std::map< std::string, std::string > :: iterator imap;
   for( imap = keywords.begin(); imap != keywords.end(); ++imap ) {
      printf("   %d  ", i++ );
      string = (*imap).first.c_str();
      printf("   %s  ", string );
      string2 = (*imap).second.c_str();
      printf("   \"%s\"\n", string2 );

      // keywords by size
      is = strlen( string );

      if( is == 1 ) {
         if( strncasecmp( string, "N", 1 ) == 0 ) {
            iold_n = (unsigned long) atol( string2 );
         }

         if( strncasecmp( string, "E", 1 ) == 0 ) {
            iold_e = (unsigned long) atol( string2 );
         }

         if( strncasecmp( string, "I", 1 ) == 0 ) {
            im = (unsigned long) atol( string2 );
         }
         if( strncasecmp( string, "J", 1 ) == 0 ) {
            jm = (unsigned long) atol( string2 );
         }
         if( strncasecmp( string, "K", 1 ) == 0 ) {
            km = (unsigned long) atol( string2 );
         }
         if( strncasecmp( string, "T", 1 ) == 0 ) {
            iret = HandleKeyword_T( string2 );
            if( iret != 0 ) ierror += 1;
         }
         if( strncasecmp( string, "F", 1 ) == 0 ) {
         // will deal with this later...
         // iret = HandleKeyword_F( string2 );
            if( iret != 0 ) ierror += 1;
         }
      }

      if( is == 2 ) {
         if( strncasecmp( string, "DT", 2 ) == 0 ) {
            // will do this later...
         }

         if( strncasecmp( string, "ET", 2 ) == 0 ) {
            iret = HandleKeyword_ET( string2 );
            if( iret != 0 ) ierror += 1;
         }

         if( strncasecmp( string, "NV", 2 ) == 0 ) {
            // will do this later...
         }

      }

      if( is == 5 ) {
         if( strncasecmp( string, "NODES", 5 ) == 0 ) {
            nodes = (unsigned long) atol( string2 );
         }

      }

      if( is == 7 ) {
         if( strncasecmp( string, "ELEMENTS", 7 ) == 0 ) {
            elems = (unsigned long) atol( string2 );
         }

      }

      if( is == 8 ) {
         if( strncasecmp( string, "ZONETYPE", 8 ) == 0 ) {
            iret = HandleKeyword_Zonetype( string2 );
            if( iret != 0 ) ierror += 1;
         }
      }

      if( is == 10 ) {
         if( strncasecmp( string, "PARENTZONE", 10 ) == 0 ) {
            parent_zone = atoi( string2 );
         }
      }

      if( is == 11 ) {
         if( strncasecmp( string, "VARLOCATION", 11 ) == 0 ) {
            iret = HandleKeyword_Varlocation( string2 );
            if( iret != 0 ) ierror += 1;
         }
         if( strncasecmp( string, "DATAPACKING", 11 ) == 0 ) {
            iret = HandleKeyword_Datapacking( string2 );
            if( iret != 0 ) ierror += 1;
         }

      }

      if( is == 12 ) {
         if( strncasecmp( string, "VARSHARELIST", 12 ) == 0 ) {
            iret = HandleKeyword_Varsharelist( string2 );
            if( iret != 0 ) ierror += 1;
         }
      }

   }

   // sanity checks for the zone's collected keywords
   ierror += ConsistencyCheck();

   // final error check
   if( ierror != 0 ) {
      iret = 1;
#ifdef _DEBUG_
      printf(" e Exiting ManageInternals() in error (return %d) \n",iret);
#endif
      return( iret );
   }

   //
   // allocate memory for variables
   //

   // get a vertex and cell count
   size_t iv=0,ie=0;

   // first try the ordered zone sizes; a sanity check is built-in...
   if( ietype == ORDERED && im > 0 ) {
      if( jm > 0 ) {
         if( km > 0 ) {
            iv = (size_t) (im*jm*km);
            ie = (size_t) ((im-1)*(jm-1)*(km-1));
         } else {
            iv = (size_t) (im*jm);
            ie = (size_t) ((im-1)*(jm-1));
         }
      } else {
         iv = (size_t) im;
         ie = (size_t) (im-1);
      }
   }

   // then try the unstruct. zone sizes; a sanity check is built-in...
   if( ietype != ORDERED && nodes > 0 ) {
      iv = (size_t) nodes;
      ie = (size_t) elems;
   }

#ifdef _DEBUG_
   printf(" i Number of vertices  %ld, elements  %ld \n",
          (unsigned long) iv, (unsigned long) ie );
#endif

   // sanity check
   if( iv <= 0 || ie <= 0 ) {
      printf(" e Cannot have zero nodes or zero elements \n");
      iret = 1;
#ifdef _DEBUG_
      printf(" e Exiting ManageInternals() in error (return %d) \n",iret);
#endif
      return( iret );
   }

   // allocate memory for data arrays
   for(int n=1;n<=num_var;++n) {
      if( ivar_loc[n] == 1 ) {
         double *p = (double *) malloc(iv*sizeof(double));
         var_vec.push_back( p );
         if( p == NULL ) ierror += 1;
#ifdef _DEBUG_
         printf(" --- Vertex-data pointer at %p \n", p );
#endif
      } else if( ivar_loc[n] == 2 ) {
         double *p = (double *) malloc(ie*sizeof(double));
         var_vec.push_back( p );
         if( p == NULL ) ierror += 1;
#ifdef _DEBUG_
         printf(" --- Cell-data pointer at %p \n", p );
#endif
      } else {
         // allow for the provision to skip allocating memory for a variable
      }
   }

   // allocate memory for connectivity if needed
   if( ietype != ORDERED ) {
      if( ietype == FETRIANGLE ) ncon = 3;
      if( ietype == FEQUADRILATERAL) ncon = 4;
      if( ietype == FETETRAHEDRON) ncon = 4;
      if( ietype == FEBRICK) ncon = 8;

      size_t isize = (size_t) (elems*ncon);
      icon = (unsigned long *) malloc(isize*sizeof(unsigned long));
      if( icon == NULL ) ierror += 1;
   }

   // drop all memory on any allocation error
   if( ierror != 0 ) {
#ifdef _DEBUG_
      printf(" e There was an allocation error!\n");
#endif
      if( icon != NULL ) free( icon );

      std::vector< double * > :: iterator dpi;
      for( dpi = var_vec.begin() ; dpi != var_vec.end() ; ++dpi ) {
         if( (*dpi) != NULL ) {
#ifdef _DEBUG_
            printf(" --- Freeing memory at: %p \n", (*dpi) );
#endif
            free( (*dpi) );
         }
      }
      var_vec.clear();

      iret = 1;
#ifdef _DEBUG_
      printf(" e Exiting ManageInternals() in error (return %d) \n",iret);
#endif
   } else {
      istate = 2;    // implies a state of reading numerical values
   }

/*
   // set certain internal counters for numeric data reading (later)
   var_no_cnt = 0;
   var_cc_cnt = 0;
   for(int n=1;n<=num_var;++n) {
      if( ivar_loc[n] == 1 ) {
         var_no_cnt += 1;
      } else if( ivar_loc[n] == 2 ) {
         var_cc_cnt += 1;
      } else {
         // I have not thought about this one...
      }
   }
*///HACK

#ifdef _DEBUG_
   printf(" i Exiting ManageInternals() (return %d) \n",iret);
#endif
//printf("EXITING PREMATURELY IN MANAGE...\n");exit(1);//HACK
   return( iret );
}

int inTec_Zone::HandleKeyword_T( const char *string )
{
   // make a duplicate buffer
   size_t is=0;
   while( string[is] != '\0' && string[is] != '\n' ) ++is;
   char *buf = (char *) malloc(is*sizeof(char));
   if( buf == NULL ) return(-1);

   memcpy( buf, string, is );
   tkey = buf;

   return(0);
}

int inTec_Zone::HandleKeyword_ET( const char *string )
{
   size_t is=0;
   while( string[is] != '\0' && string[is] != '\n' ) ++is;

   // default is to place a number so that it is know ET was set
   iold_et = 999;

   if( is == 5 ) {
      if( strncasecmp( string, "BRICK", 5 ) == 0 ) iold_et = FEBRICK;
   }

   if( is == 8 ) {
      if( strncasecmp( string, "TRIANGLE", 8 ) == 0 ) iold_et = FETRIANGLE;
   }

   if( is == 11 ) {
      if( strncasecmp( string, "TETRAHEDRON", 11 ) == 0 )
         iold_et = FETETRAHEDRON;
   }

   if( is == 13 ) {
      if( strncasecmp( string, "QUADRILATERAL", 13 ) == 0 )
         iold_et = FEQUADRILATERAL;
   }

   return(0);
}


int inTec_Zone::HandleKeyword_Varlocation( const char *string )
{
#ifdef _DEBUG_
   printf(" i Inside HandleKeyword_Varlocation() \n");
#endif
   int iret=1;

   // make a duplicate buffer
   size_t is=0;
   while( string[is] != '\0' && string[is] != '\n' ) ++is;
   char *buf = (char *) malloc(is*sizeof(char));
   if( buf == NULL ) return(-1);

   // shift passed spaces
   memcpy( buf, string, is );
   char *s = buf;
   while( s[0] == ' ' ) s++;
   is = 0;
   while( s[is] != '\0' && s[is] != '\n' ) ++is;

   // the case where all variables are nodal, which is the default
   if( iret != 0 && is >= 5 ) {
      if( strncasecmp( s, "(NODAL)", 7 ) == 0 ) {
         // perhaps do something to mark all variables as nodal
         std::map< int, int > :: iterator it;
         for( it= ivar_loc.begin(); it != ivar_loc.end(); ++it ) {
            (*it).second = 1;
         }
         iret = 0;
#ifdef _DEBUG_
         printf(" --- All variables are nodal \n");
#endif
      }
   }

   // the case where variables are individualluy separated
   int ierror=0;
   if( iret != 0 && is >= 15 ) {    // the check may be different...

      std::map< int, int > itmp_loc;
      std::map< int, int > :: iterator it;
      for(int n=1;n<=num_var;++n) itmp_loc[n] = 1;

#ifdef _DEBUG_
     printf(" --- Parsing variable locations (nodal/cell-centered) \n");
     printf(" --- Number of variables in zone( from file): %d \n", num_var);
#endif
      char *s,*sr;
      char *token;
      const char *delim;

      // first pass is the parentheses and we assume it will not be seen again
#ifdef _DEBUG3_
     printf(" --- Searching for parentheses\n");
#endif
      delim = (const char *) "()";
      token = strtok_r( buf, delim, &sr );
      if( token == NULL ) {
         // this is in error
         printf(" e Malformed varlocation line!\n");
         ierror = 1;
      }
#ifdef _DEBUG3_
      printf(" --- Token in parentheses --->|%s|<---\n",token);
#endif

      int k;
      for( k=0,s=token; ierror==0 ; s=NULL,++k ) {
#ifdef _DEBUG3_
         printf("LOOP START: ");
         printf("S  -->|%s|<-- \n",s);
#endif
         // look for first delimiter (a group before an equal sign)
         delim = (const char *) "= ";
      // printf("DELIM1 -->|%s|<-- \n",delim);
         token = strtok_r( s, delim, &sr );
      // printf("TOKEN1 -->|%s|<-- \n",token);
         // this is the proper termination condition
         if( token == NULL ) break;

         // look for variable numbers
         char *s2=token,*sr2;
         token = strtok_r( s2, "[]", &sr2 );
      // printf("TOKEN+ -->|%s|<-- \n",token);
         int kk;
         for( kk=0,s2=token; ierror==0 ; s2=NULL,++kk ) {
            token = strtok_r( s2, ",", &sr2 );
            if( token == NULL ) break;
      //    printf("TOKEN++ -->|%s|<-- \n", token);

            // look for a minus sign and take action
            int isign=0;
            is=0;
            while( token[is] != '\0' ) {
               if( token[is] == '-' ) {
                  isign = 1;         // indicate this is a range group
                  token[is] = ' ';   // turn sign to a space for easy parsing
               }
               ++is;
            }
      //    printf("TOKEN++ changed as range/number -->|%s|<-- \n", token);

            // parse the number range or variable number
            if( isign == 1 ) {
               int i1=-1,i2=-1;
               is = sscanf( token, "%d %d", &i1,&i2);
#ifdef _DEBUG2_
               printf(" --- Variable range: %d - %d \n", i1,i2);
#endif
               if( is != 2 ) {
#ifdef _DEBUG2_
                  printf(" --- Failed parsing range! \n");
#endif
                  ierror = 2;
                  break;
               } else {
                  if( i1 > num_var || i2 > num_var ||
                      i1 == 0      || i2 == 0 ) {
#ifdef _DEBUG_
                     printf(" --- Variables out of range! \n");
#endif
                     ierror = 20;
                     break;
                  } else {
                     for(int n=1;n<=num_var;++n)
                        if( i1 <= n && n <= i2 ) itmp_loc[n] = 99;
                  }
               }
            } else {
               int i0=-1;
               is = sscanf( token, "%d", &i0);
#ifdef _DEBUG2_
               printf(" --- Variable: %d \n", i0);
#endif
               if( is != 1 ) {
#ifdef _DEBUG2_
                  printf(" --- Failed parsing variable slot! \n");
#endif
                  ierror = 2;
                  break;
               } else {
                  if( i0 > num_var || i0 == 0 ) {
#ifdef _DEBUG_
                     printf(" --- Variable out of range! \n");
#endif
                     ierror = 20;
                     break;
                  } else {
                     itmp_loc[i0] = 99;
                  }
               }
            }
#ifdef _DEBUG3_
            printf(" --- Inner cycling... n");
#endif
         }
         if( ierror != 0 ) break;

         // look for second delimiter (a group before a comma)
         s = NULL;
         delim = (const char *) ",";
      // printf("DELIM2 -->|%s|<-- \n",delim);
         token = strtok_r( s, delim, &sr );
         if( token == NULL ) {
            ierror = 2;
            break;
         }
      // printf("TOKEN2 -->|%s|<-- \n",token);

         // skip the first few spaces if any
         while( token[0] == ' ' || token[0] == '=' ) token++;
      // printf("CLEAN TOKEN2 -->|%s|<-- \n",token);

         // determine the "variable location" based on the second string
         int iloc=0;    // this would eventually result in an error...
         if( strlen( token ) >= 5 ) {
            if( strncasecmp( token, "NODAL", 5 ) == 0 ) iloc = 1;
         }
         if( strlen( token ) >= 12 ) {
            if( strncasecmp( token, "CELLCENTERED", 12 ) == 0 ) iloc = 2;
         }
         for( it= itmp_loc.begin(); it != itmp_loc.end(); ++it ) {
            if( (*it).second == 99 ) (*it).second = iloc;
         }

#ifdef _DEBUG3_
         printf(" --- Outer cycling... n");
#endif
      }

#ifdef _DEBUG3_
      printf(" --- Variable locations (temporary) \n");
#endif
      for( it= itmp_loc.begin(); it != itmp_loc.end(); ++it ) {
#ifdef _DEBUG3_
         printf("      ==> var. %d location %d \n", (*it).first, (*it).second );
#endif
         ivar_loc[ (*it).first ] = (*it).second;
      }

      // drop contents of this array (althought it would have been cleaned)
      itmp_loc.clear();

      // report on errors trapped
      if( ierror == 0 ) {
         iret = 0;
      } else {
         // provide some output
      }

#ifdef _DEBUG2_
      printf(" --- Variable locations \n");
      for( it= ivar_loc.begin(); it != ivar_loc.end(); ++it ) {
         printf("      ==> var. %d location %d \n", (*it).first, (*it).second );
      }
#endif
   }

#ifdef _DEBUG_
   if( iret != 0 ) printf(" --- Problematic VARLOCATION line \n");
#endif

   // drop the buffer
   free( buf );

#ifdef _DEBUG_
   printf(" i Exiting HandleKeyword_Varlocation() \n");
#endif
   return( iret );
}


int inTec_Zone::HandleKeyword_Datapacking( const char *string )
{
#ifdef _DEBUG_
   printf(" i Inside HandleKeyword_Datapacking() \n");
#endif
   int iret=1;

   size_t is=0;
   while( string[is] != '\0' && string[is] != '\n' ) ++is;
   if( is < 5 ) return(1);

   if( strncasecmp( string, "POINT", 5 ) == 0 ) {
#ifdef _DEBUG_
      printf(" --- Data is in POINT format \n");
#endif
      idatapack = 1;
      iret = 0;
   } else if( strncasecmp( string, "BLOCK", 5 ) == 0 ) {
#ifdef _DEBUG_
      printf("   --- Data is in BLOCK format \n");
#endif
      idatapack = 2;
      iret = 0;
   } else {
#ifdef _DEBUG_
      printf(" i Unrecognized datapacking format: \"%s\" \n", string );
      iret = 2;
#endif
   }

#ifdef _DEBUG_
   printf(" i Exiting HandleKeyword_Datapacking() \n");
#endif
   return( iret );
}


int inTec_Zone::HandleKeyword_Zonetype( const char *string )
{
#ifdef _DEBUG_
   printf(" i Inside HandleKeyword_Zonetype() \n");
#endif
   int iret=1;

   size_t is=0;
   while( string[is] != '\0' && string[is] != '\n' ) ++is;
   if( is < 7 ) return(1);

   if( is == 7 ) {
      if( strncasecmp( string, "ORDERED", 7 ) == 0 ) {
         ietype = ORDERED;
         iret = 0;
      }
      if( strncasecmp( string, "FEBRICK", 7 ) == 0 ) {
         ietype = FEBRICK;
         iret = 0;
      }
   }

   if( is == 9 ) {
      if( strncasecmp( string, "FELINESEG", 9 ) == 0 ) {
         ietype = FELINESEG;
         iret = 0;
      }
      if( strncasecmp( string, "FEPOLYGON", 9 ) == 0 ) {
         ietype = FEPOLYGON;
         iret = 0;
      }
   }

   if( is == 10 ) {
      if( strncasecmp( string, "FETRIANGLE", 10 ) == 0 ) {
         ietype = FETRIANGLE;
         iret = 0;
      }
   }

   if( is == 12 ) {
      if( strncasecmp( string, "FEPOLYHEDRAL", 12 ) == 0 ) {
         ietype = FEPOLYHEDRAL;
         iret = 0;
      }
   }

   if( is == 13 ) {
      if( strncasecmp( string, "FETETRAHEDRON", 13 ) == 0 ) {
         ietype = FETETRAHEDRON;
         iret = 0;
      }
   }

   if( is == 15 ) {
      if( strncasecmp( string, "FEQUADRILATERAL", 15 ) == 0 ) {
         ietype = FEQUADRILATERAL;
         iret = 0;
      }
   }

#ifdef _DEBUG_
   printf(" i Exiting HandleKeyword_Zonetype() (return %d) \n",iret);
#endif
   return( iret );
}

int inTec_Zone::HandleKeyword_Varsharelist( const char *string )
{
#ifdef _DEBUG_
   printf(" i Inside HandleKeyword_Varsharelist() \n");
#endif
   int iret=1;

   // make a duplicate buffer
   size_t is=0;
   while( string[is] != '\0' && string[is] != '\n' ) ++is;
   char *buf = (char *) malloc(is*sizeof(char));
   if( buf == NULL ) return(-1);

   // shift passed spaces
   memcpy( buf, string, is );
   char *s = buf;
   while( s[0] == ' ' ) s++;
   is = 0;
   while( s[is] != '\0' && s[is] != '\n' ) ++is;

////// need to make this later
#ifdef _DEBUG_
   printf(" --- Temporarily ignoring this keyword\n");
#endif
iret=0;

   // drop the buffer
   free( buf );

#ifdef _DEBUG_
   printf(" i Exiting HandleKeyword_Varsharelist() (return %d) \n",iret);
#endif
   return( iret );
}

int inTec_Zone::ConsistencyCheck (void )
{
#ifdef _DEBUG_
   printf(" i Inside ConsistencyCheck() \n");
#endif
   int iret=0,ierror;

#ifdef _DEBUG_
   // display all stored keywords as they were found after sanitized parsing
   int i=0;
   const char *string=NULL,*string2=NULL;
   std::map< std::string, std::string > :: iterator imap;
   for( imap = keywords.begin(); imap != keywords.end(); ++imap ) {
      printf("   %d  ", i++ );
      string = (*imap).first.c_str();
      printf("   %16s ", string );
      string2 = (*imap).second.c_str();
      printf("= \"%s\"\n", string2 );
   }
#endif

//HERE1 (consistency checks not fully tested)
   //
   // first check for old-style variables
   // (possibly set a variable indicating that OLD switches are present)
   //
#ifdef _DEBUG_
   printf(" --- Old variables: \n");
   printf("   iold_n: %ld \n", iold_n);
   printf("   iold_e: %ld \n", iold_e);
   printf("   iold_et: %d \n", iold_et);
#endif
   /// give precedence to old switches... (incomplete)
   if( iold_n > 0 ) nodes = iold_n;
   if( iold_e > 0 ) elems = iold_e;
   if( iold_et > -1 ) {
#ifdef _DEBUG_
      printf(" --- Overriding ZONETYPE keyword; the ET keyword exists \n");
#endif
      if( iold_et == FETRIANGLE ) {
         ietype = FETRIANGLE;
      }
      if( iold_et == FETETRAHEDRON ) {
         ietype = FETETRAHEDRON;
      }
      if( iold_et == FEQUADRILATERAL ) {
         ietype = FEQUADRILATERAL;
      }
      if( iold_et == FEBRICK ) {
         ietype = FEBRICK;
      }
   }


   //
   // check for consistency in "sanitized" state
   //
   ierror = 0;
   if( im > 0 && nodes > 0 ) {
      printf(" e Specified both \"im/jm/km\" and \"nodes\" in zone \n");
      ierror = 1;
      iret += 1;
   } else {
#ifdef _DEBUG_
      printf(" --- Either I/J/K or N/NODES specified exclusively; good \n");
#endif
   }

   ierror = 0;
   if( im > 0 ) {
      if( ietype != ORDERED ) {
         printf(" e Conflicting specification of ZONETYPE and data provided\n");
         ierror = 1;
         iret += 1;
      } else {
#ifdef _DEBUG_
         printf(" --- ZONETYPE is ORDERED and I is non-zero; good \n");
#endif
      }
   }

   ierror = 0;
   if( nodes > 0 && ietype == ORDERED ) {
      printf(" e Number of nodes provided for an ordered zone \n");
      ierror = 1;
      iret += 1;
   } else {
#ifdef _DEBUG_
      printf(" --- ZONETYPE is not ORDERED while NODES is non-zero; good \n");
#endif
   }


   // loop over all variable locations and if there are cell-centered values
   // the sanity check is that data cannot be point (only block)
   ierror = 0;
   std::map< int, int > :: iterator it;
   for( it = ivar_loc.begin(); it != ivar_loc.end(); ++it ) {
      if( (*it).second == 2 && idatapack != 2 ) ierror += 1;
   }
   if( ierror != 0 ) {
      printf(" e VARLOCATION is cell-centered for %d variables...\n", ierror );
      printf("   ...but DATAPACKING specified as POINT (must be BLOCK)\n");
      iret += 1;
   } else {
#ifdef _DEBUG_
      printf(" --- VARLOCATION is compatible with DATAPACKING ");
      if( idatapack == 1 ) printf("(POINT)\n");
      if( idatapack == 2 ) printf("(BLOCK)\n");
#endif
   }




#ifdef _DEBUG_
   printf(" i Exiting ConsistencyCheck() (return %d) \n",iret);
#endif
//printf("EXITING IN CONSISTENCY CHECK...\n");exit(1);//HACK
   return( iret );
}



int inTec_Zone::ParseNumericData( char *buf )
{
#ifdef _DEBUG2_
   printf(" i Parsing numeric data \n");
#endif
   // set the numeric parsing state when we first try to parse data
   if( iparse_num == 0 ) {
      iparse_num = 1;      // indicates that we are begining to parse data
   } else if( iparse_num == 1 ) {

   } else if( iparse_num == 2 ) {
      iparse_num = 3;
#ifdef _DEBUG_
   printf(" i Switched to parsing connectivity data \n");
#endif
   } else {
      // now that the check completed; reset bounds?????
   }


#ifdef _DEBUG2_
   printf(" --- String to parse: --->|%s|<---\n", buf );
#endif

   // clean-up all fortran double-precision formatting
   size_t i=0;
   while( buf[i] != '\0' && buf[i] != '\n' ) {
      if( buf[i] == 'd' || buf[i] == 'D' ) buf[i] = 'e';
   // if( buf[i] == ',' ) buf[i] = ' ';
      ++i;
   }

   // parse segments separated by spaces or commas (expecting reals/doubles)
   if( iparse_num == 1 ) {
      char *string,*token,*sr;
      for( i=0, string = buf; ; ++i, string = NULL ) {
         token = strtok_r( string, " ,", &sr );
         if( token == NULL ) break;
#ifdef _DEBUG2_
         printf(" --- token: --->|%s|<--- \n", token );
#endif

         char *c;
         double d = strtod( token, &c );
#ifdef _DEBUG2_
         printf(" --- value: %lf \n", d );
#endif

#ifdef _DEBUG2_
         // display internal counter states
         printf(" --- node_cnt: %ld  elem_cnt: %ld  var_cnt: %d (prior) \n",
                node_cnt, elem_cnt, var_cnt );
#endif

         // get pointer to data array for variable
         double *dp = var_vec[ var_cnt ];
#ifdef _DEBUG2_
         printf(" --- pointer: %p    (varloc.: %d) \n", dp,ivar_loc[var_cnt+1]);
#endif

         if( idatapack == 1 ) {   // POINT
            dp[ node_cnt ] = d;

            // update counters
            ++var_cnt;
            if( var_cnt == num_var ) {
               ++node_cnt;

               // check for a termination condition
               if( node_cnt == nodes ) {
                  node_cnt = 0;
#ifdef _DEBUG_
                  printf(" --- Parsing of variable data ending \n");
#endif
                  if( ietype != ORDERED ) {
                     iparse_num = 2;
                  } else {
                     // set state of zone to "loaded and ready" (ordered zone)
                     istate = 3;
                  }
               }

               var_cnt = 0;
            }

         } else {   // BLOCK
            int iloc = ivar_loc[ var_cnt+1 ];
            if( iloc == 1 ) {
               dp[ node_cnt ] = d;

               ++node_cnt;
               if( node_cnt == nodes ) {
                  ++var_cnt;
                  node_cnt = 0;
               }
            } else if( iloc == 2 ) {
               dp[ elem_cnt ] = d;

               ++elem_cnt;
               if( elem_cnt == elems ) {
                  ++var_cnt;
                  elem_cnt = 0;
               }
            } else {
               // I have not thought about how to do this...
            }

            // termination condition
            if( var_cnt == num_var ) {
               var_cnt = 0;
#ifdef _DEBUG_
               printf(" --- Parsing of numeric data ending \n");
#endif
               if( ietype != ORDERED ) {
                  iparse_num = 2;
               } else {
                  // set state of zone to "loaded and ready" (ordered zone)
                  istate = 3;
               }
            }
         }

      }  // strtok loop
   }  // iparse_num=1

   // parse segments separated by spaces (expecting connectivity in one line)
   if( iparse_num == 3 ) {
      unsigned long n1,n2,n3,n4,n5,n6,n7,n8;
      int ic=0;

      if( ietype == FETRIANGLE ) 
         ic = sscanf( buf, "%ld %ld %ld", &n1,&n2,&n3 );
      if( ietype == FEQUADRILATERAL ) 
         ic = sscanf( buf, "%ld %ld %ld %ld", &n1,&n2,&n3,&n4 );
      if( ietype == FETETRAHEDRON ) 
         ic = sscanf( buf, "%ld %ld %ld %ld", &n1,&n2,&n3,&n4 );
      if( ietype == FEBRICK ) 
         ic = sscanf( buf, "%ld %ld %ld %ld %ld %ld %ld %ld",
                            &n1,&n2,&n3,&n4,&n5,&n6,&n7,&n8 );

#ifdef _DEBUG2_
         printf(" --- Connectivity elements picked: %d \n", ic);
#endif
      if( ic > 0 ) {
         unsigned long n = ncon*icon_cnt;

         if( ic >= 3 ) {
            icon[ n+0 ] = n1;
            icon[ n+1 ] = n2;
            icon[ n+2 ] = n3;
         }
         if( ic >= 4 ) {
            icon[ n+3 ] = n4;
         }
         if( ic >= 8 ) {
            icon[ n+4 ] = n5;
            icon[ n+5 ] = n6;
            icon[ n+6 ] = n7;
            icon[ n+7 ] = n8;
         }

         icon_cnt += 1;
      }

      // indicate that the zone has been filled with data and is ready
      if( icon_cnt == elems ) istate = 3;
   }

   return(0);
}


int inTec_Zone::Dump( const char *filename )
{
   if( filename == NULL ) {
#ifdef _DEBUG_
      printf(" e You need to supply a non-null filename string \n");
#endif
      return(1);
   }
   if( istate != 3 ) {
#ifdef _DEBUG_
      printf(" e Zone is in the wrong state: %d \n", istate );
#endif
      return(2);
   }

#ifdef _DEBUG_
   printf(" i Dumping zone to file \"%s\"\n", filename);
#endif

   FILE *fout = fopen( filename, "w" );
   if( fout == NULL ) {
      printf(" e Could not open file for writing!\n");
      return(-1);
   }

   // write the variables first
   fprintf( fout, "VARIABLES = " );
   // (queries back the file structure/class)
   for(int n=0;n<num_var;++n) {
      const char *s = file->GetVariableName( n );
      fprintf( fout, "\"%s\" ", s );
   }
   fprintf( fout, "\n" );

   // put a zone keywords and attach the "T" keyword to it
   fprintf( fout, "ZONE " );
   std::map< std::string, std::string > :: iterator imt;
   for( imt = keywords.begin(); imt != keywords.end(); ++imt ) {
      if( strlen( (*imt).first.c_str() ) == 1 ) {
         if( strncmp( (*imt).first.c_str(), "T", 1 ) == 0 ) {
            const char *s = (*imt).second.c_str();
            if( s[0] == '\"' ) {
               fprintf( fout, "T=%s, ", s );
            } else {
               fprintf( fout, "T=\"%s\", ", s );
            }
         }
      }
   }

   // decide which essential keywords to use
   if( ietype == ORDERED ) {
      if( im > 0 ) fprintf( fout, "I=%ld, ", im );
      if( jm > 0 ) fprintf( fout, "J=%ld, ", jm );
      if( km > 0 ) fprintf( fout, "K=%ld, ", km );



   } else {
      if( nodes > 0 ) fprintf( fout, "NODES=%ld, ", nodes );
      if( elems > 0 ) fprintf( fout, "ELEMENTS=%ld, ", elems );

      if( ietype == FETRIANGLE ) 
         fprintf( fout, "ZONETYPE=FETRIANGLE, " );
      if( ietype == FEQUADRILATERAL ) 
         fprintf( fout, "ZONETYPE=FEQUADRILATERAL, " );
      if( ietype == FETETRAHEDRON ) 
         fprintf( fout, "ZONETYPE=FETETRAHEDRON, " );
      if( ietype == FEBRICK )
         fprintf( fout, "ZONETYPE=FEBRICK, " );


   // for( imt = keywords.begin(); imt != keywords.end(); ++imt ) {
   //    if( strlen( (*imt).first.c_str() ) == 11 ) {
   //       if( strncmp( (*imt).first.c_str(), "VARLOCATION", 11 ) == 0 ) {
   //          fprintf( fout, "VARLOCATION=%s ", (*imt).second.c_str() );
   //       }
   //    }
   // }

   }

   if( idatapack == 1 ) {
      fprintf( fout, " DATAPACKING=POINT,\n" );
   } else {
      fprintf( fout, " DATAPACKING=BLOCK,\n" );
   }

   for( imt = keywords.begin(); imt != keywords.end(); ++imt ) {
      if( strlen( (*imt).first.c_str() ) == 11 ) {
         const char *s = (*imt).second.c_str();
         // perhaps do some string sanitation here (capitalization, etc...)
         if( strncmp( (*imt).first.c_str(), "VARLOCATION", 11 ) == 0 ) {
            fprintf( fout, "VARLOCATION=%s ", s );
         }
      }
   }
   fprintf( fout, "\n" );


   if( ietype == ORDERED ) {
      unsigned long jmt=1,kmt=1;

      if( idatapack == 1 ) {
         if( jm > 0 ) jmt = jm;
         if( km > 0 ) kmt = km;

         for(unsigned long k=0;k<kmt;++k) {
         for(unsigned long j=0;j<jmt;++j) {
         for(unsigned long i=0;i<im;++i) {
            unsigned long n = k*jmt*im + j*im + i;
            for(int kk=0;kk<num_var;++kk) {
               fprintf( fout, " %18.11e", var_vec[kk][n] );
            }
            fprintf( fout, "\n");
         }}}
      } else {
         for(int kk=0;kk<num_var;++kk) {
            if( ivar_loc[kk+1] == 1 ) {   // nodal
               if( jm > 0 ) jmt = jm;
               if( km > 0 ) kmt = km;

               for(unsigned long k=0;k<kmt;++k) {
               for(unsigned long j=0;j<jmt;++j) {
               for(unsigned long i=0;i<im;++i) {
                  unsigned long n = k*jmt*im + j*im + i;
                  fprintf( fout, " %18.11e \n", var_vec[kk][n] );
               }}}
            } else {  // cell-centered
               if( jm > 0 ) jmt = jm-1;
               if( km > 0 ) kmt = km-1;

               for(unsigned long i=0;i<im-1;++i) {
               for(unsigned long j=0;j<jmt;++j) {
               for(unsigned long k=0;k<kmt;++k) {
                  unsigned long n = k*jmt*(im-1) + j*(im-1) + i;
                  fprintf( fout, " %18.11e \n", var_vec[kk][n] );
               }}}
            }
         }
      } // datapack
   } else {  // unstructured data
      if( idatapack == 1 ) {
         for(unsigned long n=0;n<nodes;++n) {
            for(int kk=0;kk<num_var;++kk) {
               fprintf( fout, " %18.11e", var_vec[kk][n] );
            }
            fprintf( fout, " \n" );
         }
      } else {
         for(int kk=0;kk<num_var;++kk) {
            if( ivar_loc[kk+1] == 1 ) {   // nodal
               for(unsigned long n=0;n<nodes;++n) {
                  fprintf( fout, " %18.11e \n", var_vec[kk][n] );
               }
            } else {  // cell-centered
               for(unsigned long n=0;n<elems;++n) {
                  fprintf( fout, " %18.11e \n", var_vec[kk][n] );
               }
            }
         }
      } // datapack

      // dump connectivity
      for(unsigned long i=0;i<elems;++i) {
         for(unsigned long j=0;j<ncon;++j) {
            fprintf( fout, " %ld", icon[ i*ncon + j ] );
         }
         fprintf( fout, "\n" );
      }
   }

   // close the file
   fflush( fout );
   fclose( fout );

#ifdef _DEBUG_
   printf(" i Finished dumping zone to file \"%s\"\n", filename);
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

   // drop all contents of all zones attached to this file
#ifdef _DEBUG_
      printf(" --- Number of zones to clear: %d \n", (int) zones.size() );
#endif
   for(int n=0;n<zones.size();++n) {
#ifdef _DEBUG_
      printf(" --- Clearing zone %d \n", n );
#endif
   // zones[n]->clear();
      delete zones[n];
   }
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

#ifdef _DEBUG_
   printf(" i Number of variables after clear %ld \n", variables.size());
   printf(" i Number of strings after clear %ld \n", strings.size());
#endif
}


int inTec_File::OpenRead()
{
#ifdef _DEBUG_
   printf(" i Opening file object \"%s\" \n", name );
#endif

   if( istate != 0 ) {
      printf(" e File object is not in the right state (%d)\n", istate);
      return(1);
   }

   fp = fopen( name, "r" );
   if( fp == NULL ) {
      printf(" e Could not open file \"%s\"\n", name );
      return(2);
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

   if( istate == 0 || istate == 1000 ) {
      return(0);
   }

   fclose( fp );

   istate = 1000;
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

int inTec_File::GetNumVariables( void ) const
{
   int i = variables.size();
   return( i );
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

   if( iret != 0 ) {
      return(2);
   }

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
#ifdef _DEBUG3_
      printf("IC= %d, STRING: \"%s\"\n", ic, buf );
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
#endif

   if( buf == NULL ) return(-1);
#ifdef _DEBUG3_
   printf("BUF: -->|%s", buf );
#endif

   size_t isize=0;
   while( buf[isize] != '\n' && buf[isize] != '\0' ) ++isize;
#ifdef _DEBUG3_
   printf(" i Size of buffer: %d \n", (int) isize);
#endif

   if( buf[0] == '\n' ) return(0);

   if( buf[0] == '#' ) return( Comment );

   if( isize >= 9 ) {
      if( strncasecmp( "VARIABLES", buf, 9 ) == 0 ) return( Variables );

   }

   if( isize >= 8 ) {
      if( strncasecmp( "FILETYPE", buf, 8 ) == 0 ) return( Filetype );
      if( strncasecmp( "GEOMETRY", buf, 8 ) == 0 ) return( Geometry );
   }

   if( isize >= 5 ) {
      if( strncasecmp( "TITLE", buf, 5 ) == 0 ) return( Title );
   }

   if( isize >= 4 ) {
      if( strncasecmp( "ZONE", buf, 4 ) == 0 ) return( Zone );
      if( strncasecmp( "TEXT", buf, 4 ) == 0 ) return( Text );
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
    case(1):   // title

#ifdef _DEBUG_
      printf(" i Parsing header \"title\" line \n");
#endif

    break;
    case(2):   // filetype

#ifdef _DEBUG_
      printf(" i Parsing header \"filetype\" line \n");
#endif

    break;
    case(3):   // variables

#ifdef _DEBUG_
      printf(" i Parsing header \"variables\" line \n");
#endif
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
#ifdef _DEBUG3_
   printf("---> stok: |%s| \n",token);
   printf("---> data: |%s| \n",data);
#endif

   // individual variables parsing
   s1 = NULL;
#ifdef _DEBUG4_
   printf("Inner loop\n");
#endif
   for(int k=1; ; s1 = NULL, ++k) {
      token = strtok_r( s1, " ,", &sr );   // search delim either ' ' or ','
      if( token == NULL ) break;
#ifdef _DEBUG3_
      printf("---> starting inner stok: |%s| \n",token);
#endif

      if( token[0] == '\"' ) {
#ifdef _DEBUG4_
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
#ifdef _DEBUG4_
         printf("   NEW STRING POINTER AT -->%s<-- \n", s1 );
#endif
         token = strtok_r( s1, "\"", &sr);
         if( token == NULL ) break;
//       if( token[0] == '\0' ) break;  // HACK
      }
#ifdef _DEBUG4_
      printf("Instance number %d: |%s|\n", k,token);
#endif

      // deal with string-token here...
      std::string sdum = token;
      variables.push_back( sdum );

#ifdef _DEBUG4_
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
   // check for allocation error
   if( zone == NULL ) {
      printf(" e Could not allocate zone! \n");
      return(-1);
   }

   // set the position in the file for this zone
   int iret = zone->SetPositionInFile( ipos );
   if( iret != 0 ) {
      printf(" e Error setting zone's position: %ld \n", ipos);
      printf("   This should never happen (zone rejects only when new)\n");
      delete zone;
      return(1);
   }

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
#ifdef _DEBUG2_
      printf(" i Position in file at %ld bytes, line %ld \n", ipos, iline+1 );
#endif

      // prepare the buffer
      memset( buf, '\0', ibuf_size );
      // read a line of data; get count of bytes read
      ic = inUtils_ReadTextline( &ibuf_size, &buf, fp );
      // check error code
      if( ic < 0 ) {
         printf(" e There was an external library error (inUtils): %d\n", ic);
         return(-2);    // this error code may have to change
      }

      ++iline;

      // sanitize the line
      inUtils_TrimLeadingSpaceTextline( buf );
      size_t isize=0;
      while( isize < ibuf_size && buf[isize] != '\n' ) ++isize;
      if( buf[isize] == '\n' ) buf[isize] = '\0';
#ifdef _DEBUG3_
      printf("STRING (%p, %ld):%s\n", buf, isize, buf );
#endif

      // attempt to identify this line as a new component to terminate parsing
      int iparse_line=1;    // default is to parse the line
      iret = IdentifyComponent( buf );
#ifdef _DEBUG3_
printf("FOUND iret=%d idone_zone=%d \n", iret,idone_zone);
#endif
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

            // possibly deal with a "false positive" of a zone...
// 
//  This fails when finding leading "ZONETYPE" for example!!!
//  What I need to do is to find a better trap for this
// 
            if( strlen( buf ) >= 8 ) {
               if( strncasecmp( buf, "ZONETYPE", 8 ) == 0 ) {

               } else {
                  // increment termination condition variable
                  ++idone_zone;
               }
            }

            // will not parse new components
            if( idone_zone > 1) {
               iparse_line = 0;
#ifdef _DEBUG_
               printf(" --- New zone encountered; terminating this zone \n");
#endif
            }
         }
      } else {
#ifdef _DEBUG2_
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
#ifdef _DEBUG_
               printf(" i Finished reading zone header (at line: %ld)\n",iline);
#endif

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
               iret = zone->SetDataPositionInFile( ipos );
               // (here we must put the zone in data-reading particular state)
               if( zone->SetState_Reading() != 0 ) {
                  // respond to the possibility of the zone being mis-used...
                  return(5);   // this error code may change
               }

            } else if( np < 0 ) {
               // an error in parsing keywords was trapped
               // (This section will have to be changed based on how we want to
               // handle bad keywords, etc.)
               printf(" e Error parsing zone keywords \n");
               free( buf );
               return(4);
            }
         } else { // we have parsed the header completely; now parsing data
#ifdef _DEBUG2_
            printf(" --- Collecting zone data \n");
#endif
            // push the buffer to the zone for data extraction
            // (we must have set the zone to a data-reading state earlier)
            iret = zone->ParseNumericData( buf );
            // possibly respond to errors returned by the zone...
            if( iret < 0 ) {
               printf(" e Error parsing zone data \n");
               free( buf );
               return(5);
            }

         }

      } // (end of iparse_line=1)
   }

   // drop the buffer
   free( buf );

   // rewind this line
   iret = fseek( fp, ipos, SEEK_SET );
   if( iret != 0 ) {
      printf(" e Error seeking to parsing position at end: %ld \n", ipos);
      return(3);
   }
   --iline;


printf("GOT HERE (zone istate=%d) \n",zone->GetState());
  zone->Dump( (const char *) "crap.dat" );
#ifdef _DEBUG_
// temporarily drop the zone object
  printf("HACK dropping the zone object\n");
//delete zone; //HACK
  zones.push_back( zone );
#endif
//printf("EXITING PREMATURELY IN ParseComponent_Zone() \n");exit(1);//HACK

#ifdef _DEBUG_
// printf(" i *** Skipping parsing of ZONE component *** \n");
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
#ifdef _DEBUG3_
      printf("STRING (%p, %ld):%s\n", buf, isize, buf );
#endif

      // attempt to identify this line as a new component to terminate parsing
      int iparse_line=1;    // default is to parse the line
      iret = IdentifyComponent( buf );
#ifdef _DEBUG3_
printf("FOUND iret=%d idone_text=%d \n", iret,idone_text);
#endif
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
#ifdef _DEBUG_
            printf(" i Added comment to strings line %ld \n", iline );
#endif
            // show the line
            printf(" [%ld]  %s", iline, buf );
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
#ifdef _DEBUG3_
      printf("STRING (%p, %ld):%s\n", buf, isize, buf );
#endif

      // attempt to identify this line as a new component to terminate parsing
      int iparse_line=1;    // default is to parse the line
      iret = IdentifyComponent( buf );
#ifdef _DEBUG3_
printf("FOUND iret=%d idone_geom=%d \n", iret,idone_geom);
#endif
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


const char* inTec_File::GetVariableName( int nvar_ ) const
{
   if( nvar_ < 0 ) return( NULL );

   if( (unsigned int) nvar_ >= variables.size() ) return( NULL );

   return( variables[ nvar_ ].c_str() );
}


}
#endif

