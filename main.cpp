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
   inTec_Zone *z = f.GetZone( 0 );
   z->Dump( (char*) "CRAPCRAP.dat");//HACK

   printf("Zone type: %d \n", z->GetType() );
   printf("Zone variables: %d \n", z->GetNumVariables() );
   printf("Zone (not real) im: %ld \n",z->GetNumIndices( 1 ) );
   printf("Zone (not real) jm: %ld \n",z->GetNumIndices( 2 ) );
   printf("Zone (not real) km: %ld \n",z->GetNumIndices( 3 ) );
   printf("Zone nodes: %ld elements: %ld \n",
             z->GetNumNodes(), z->GetNumElements() );
   const unsigned long* icon = z->GetConnectivityPtr();
   printf(" %ld %ld %ld %ld \n",icon[0],icon[1],icon[2],icon[3]);
// 41 42 2 1
// 42 43 3 2
// 43 44 4 3
   const double* d = z->GetVariablePtr( 0 );
   printf(" %16.9e %16.9e %16.9e \n", d[0], d[1], d[2] );
//  5.000000000E-02 5.214617101E-02 5.437147251E-02


  printf("========================= LOOP ==========================\n");
//}

   return(0);
}


