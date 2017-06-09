#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intecplot.h"

#include "utils.h" // HACK

int main()
{
   char *fname = (char *) "data.dat";
   inTec_File f( fname );

   f.OpenRead();
   f.Parse();
   f.Close();



   return(0);
}

