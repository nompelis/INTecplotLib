/******************************************************************************
 Copyright (c) 2017, Ioannis Nompelis
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Items have been trimmed from this header file from the original.
// This is to allow for distribution of a "trimmed" version of my utilities,
// and in this case only the ones presently relevant to this project.
// IN 2019/05/23
//

#include "utils.h"
#include "utils_proto.h"


/*
 * Function to read a line of data from an ASCII file similar to and intended
 * to replace fgets(). It takes a pointer to a pointer to char and the size that
 * is associated with the memory pointed to by that pointer to which the pointer
 * is pointing... The pointer to a null pointer can be passed. The idea is to
 * keep augmenting the memory as the text lines encountered in the file get
 * larger; the user does not care about providing a large enough buffer. Error
 * trapping is done by checking the return value; a negative value means failure
 * to re-allocate mamory.
 * Ioannis Nompelis <nompelis@nobelware.com>       Created: 20170517
 * Ioannis Nompelis <nompelis@nobelware.com> Last modified: 20170517
 */

int inUtils_ReadTextline( size_t *isize, char **buf, FILE *fp )
{
   const size_t iseg_size = 256;  // this is arbitrary but a good guess
   const size_t isafe = 1;        // this is to allow stupid external queries...
   char *tbuf = NULL;
   size_t i;
   int idone = 0;


   // check if we have a valid stream open for reading
   if( fp == NULL ) {
      printf("Not a valid pointer to stream\n");
      return(0);
   }

#ifdef _DEBUG_
   printf("Buffer incoming at: %p \n", *buf );
#endif

   // keep reading until an end-of-line is encountered
   // allocate memory as needed
   i = 0;
   while( idone == 0 ) {
      int ic;
      char c;

      // allocate memory if there is inadequate space
      if( i == (*isize) ) {
         size_t itmp = (*isize) + iseg_size + isafe;
         tbuf = (char *) realloc( *buf, itmp*sizeof(char) );
         if( tbuf == NULL ) {
            printf("Could not (re)allocate buffer\n");
            return(-1);
         }
         // assign buffer pointer
         *buf = tbuf;
         // set temporary pointer to new segment of the buffer
         tbuf = (char *) &((*buf)[ (*isize) ]);
         // clean-up the newly allocated memory
         memset( tbuf, '\0', iseg_size+isafe );
         // update size of buffer
         *isize += iseg_size;

#ifdef _DEBUG_
         printf("Buffer allocated (size: %ld) at %p / %p\n",
                (unsigned long) (*isize), *buf, tbuf );
#endif
      }

      // get a single character
      ic = fgetc( fp );

      // check for end-of-file
      if( ic == EOF ) {
#ifdef _DEBUG_
         printf("End-of-file encountered\n");
#endif
         (*buf)[i] = '\0';
         idone = 1;
      } else {
         // put byte in buffer
         c = (char) ic;
         (*buf)[i] = c;
         ++i;
#ifdef _DEBUG2_
         printf(" Got \"%c\" (%d) \n", c, (int) c);
#endif
      }

      // terminate on newline character
      if( c == '\n' ) {
         idone = 1;
      }
   }

   // return count of bytes read
   return( (int) i );
}


/*
 * Function to trim leading spaces from a string that is properly terminated
 * Ioannis Nompelis <nompelis@nobelware.com>       Created: 20170517
 * Ioannis Nompelis <nompelis@nobelware.com> Last modified: 20170517
 */

int inUtils_TrimLeadingSpaceTextline( char *buf )
{
   size_t i,ie=0,is=0;


   if( buf == NULL ) return(1);

   while( buf[is] == ' ' ) ++is;
   while( buf[ie] != '\0' ) ++ie;

   if( is == 0 ) return(0);

   for(i=0;i<ie-is;++i) {
      buf[i] = buf[i+is];
      buf[i+is] = '\0';
   }

   return(0);
}

