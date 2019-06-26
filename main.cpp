#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intecplot.h"


int main( int argc, char *argv[] )
{
   char *fname = (char *) (argv[1]);

//while(1) {

   inTec_File f( fname );
   f.OpenRead();
   f.Parse();
   f.Close();
   inTec_Zone *z = f.GetZone( 1 );

  printf("========================= LOOP ==========================\n");
//}

   return(0);
}


