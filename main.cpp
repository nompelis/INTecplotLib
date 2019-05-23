#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intecplot.h"


int main()
{
   char *fname = (char *) "data.dat";

//while(1) {

   inTec_File f( fname );
   f.OpenRead();
   f.Parse();
   f.Close();

  printf("========================= LOOP ==========================\n");
//}

   return(0);
}


