/*========================================================================
 * main.c  --  Command-line interface for Tsunami client.
 *
 * This is the main module for the Tsunami file transfer CLI client.
 *
 * Written by Mark Meiss (mmeiss@indiana.edu).
 * Copyright (C) 2002 The Trustees of Indiana University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1) All redistributions of source code must retain the above
 *    copyright notice, the list of authors in the original source
 *    code, this list of conditions and the disclaimer listed in this
 *    license;
 *
 * 2) All redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the disclaimer
 *    listed in this license in the documentation and/or other
 *    materials provided with the distribution;
 *
 * 3) Any documentation included with all redistributions must include
 *    the following acknowledgement:
 *
 *      "This product includes software developed by Indiana
 *      University`s Advanced Network Management Lab. For further
 *      information, contact Steven Wallace at 812-855-0960."
 *
 *    Alternatively, this acknowledgment may appear in the software
 *    itself, and wherever such third-party acknowledgments normally
 *    appear.
 *
 * 4) The name "tsunami" shall not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission from Indiana University.  For written permission,
 *    please contact Steven Wallace at 812-855-0960.
 *
 * 5) Products derived from this software may not be called "tsunami",
 *    nor may "tsunami" appear in their name, without prior written
 *    permission of Indiana University.
 *
 * Indiana University provides no reassurances that the source code
 * provided does not infringe the patent or any other intellectual
 * property rights of any other entity.  Indiana University disclaims
 * any liability to any recipient for claims brought by any other
 * entity based on infringement of intellectual property rights or
 * otherwise.
 *
 * LICENSEE UNDERSTANDS THAT SOFTWARE IS PROVIDED "AS IS" FOR WHICH
 * NO WARRANTIES AS TO CAPABILITIES OR ACCURACY ARE MADE. INDIANA
 * UNIVERSITY GIVES NO WARRANTIES AND MAKES NO REPRESENTATION THAT
 * SOFTWARE IS FREE OF INFRINGEMENT OF THIRD PARTY PATENT, COPYRIGHT,
 * OR OTHER PROPRIETARY RIGHTS.  INDIANA UNIVERSITY MAKES NO
 * WARRANTIES THAT SOFTWARE IS FREE FROM "BUGS", "VIRUSES", "TROJAN
 * HORSES", "TRAP DOORS", "WORMS", OR OTHER HARMFUL CODE.  LICENSEE
 * ASSUMES THE ENTIRE RISK AS TO THE PERFORMANCE OF SOFTWARE AND/OR
 * ASSOCIATED MATERIALS, AND TO THE PERFORMANCE AND VALIDITY OF
 * INFORMATION GENERATED USING SOFTWARE.
 *========================================================================*/

#include <ctype.h>        /* for the isspace() routine             */
#include <stdlib.h>       /* for *alloc() and free()               */
#include <string.h>       /* for standard string routines          */
#include <unistd.h>       /* for standard Unix system calls        */

#include <tsunami.h>
#include <tsunami-client.h>
#ifdef VSIB_REALTIME
#include "vsibctl.h"
#endif

/*------------------------------------------------------------------------
 * Function prototypes (module scope).
 *------------------------------------------------------------------------*/

void parse_command(command_t *command, char *buffer);


/*------------------------------------------------------------------------
 * MAIN PROGRAM
 *------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
    command_t        command;                           /* the current command being processed */
    char             command_text[MAX_COMMAND_LENGTH];  /* the raw text of the command         */
    ttp_session_t   *session = NULL;
    ttp_parameter_t  parameter;
   
    int argc_curr       = 1;                            /* command line argument currently to be processed */
    char *ptr_command_text = &command_text[0];
   
    /* reset the client */
    memset(&parameter, 0, sizeof(parameter));
    reset_client(&parameter);

    /* show version / build information */
    #ifdef VSIB_REALTIME
    fprintf(stderr, "Tsunami Realtime Client for protocol rev %X\nRevision: %s\nCompiled: %s %s\n"
                    "   /dev/vsib VSIB accesses mode is %d, gigabit=%d, 1pps embed=%d, sample skip=%d\n",
            PROTOCOL_REVISION, TSUNAMI_CVS_BUILDNR, __DATE__ , __TIME__,
            vsib_mode, vsib_mode_gigabit, vsib_mode_embed_1pps_markers, vsib_mode_skip_samples);
    #else
    fprintf(stderr, "Tsunami Client for protocol rev %X\nRevision: %s\nCompiled: %s %s\n",
            PROTOCOL_REVISION, TSUNAMI_CVS_BUILDNR, __DATE__ , __TIME__);    
    #endif
    
    /* while the command loop is still running */   
    while (1) {

      /* retrieve the user's commands */
      if (argc<=1 || argc_curr>=argc) {
         
         /* present the prompt */
         fprintf(stdout, "tsunami> ");
         fflush(stdout);
         /* read next command */
         
         if (fgets(command_text, MAX_COMMAND_LENGTH, stdin) == NULL) {
            error("Could not read command input");
         }
         
      } else {
         
         // severe TODO: check that command_text appends do not over flow MAX_COMMAND_LENGTH...
         
         /* assemble next command from command line arguments */
         for ( ; argc_curr<argc; argc_curr++) {
            // zero argument commands
            if (!strcasecmp(argv[argc_curr], "close") || !strcasecmp(argv[argc_curr], "quit") 
                || !strcasecmp(argv[argc_curr], "exit") || !strcasecmp(argv[argc_curr], "bye")
                || !strcasecmp(argv[argc_curr], "help") || !strcasecmp(argv[argc_curr], "dir")) {
               strcpy(command_text, argv[argc_curr]);
               argc_curr += 1;
               break; 
            } 
            // single argument commands
            if (!strcasecmp(argv[argc_curr], "connect")) {
               if (argc_curr+1 < argc) {
                  strcpy(ptr_command_text, argv[argc_curr]);
                  strcat(command_text, " ");
                  strcat(command_text, argv[argc_curr+1]);
               } else {
                  fprintf(stderr, "Connect: no host specified\n"); 
                  exit(1);
               }
               argc_curr += 2;
               break;
            }
            if (!strcasecmp(argv[argc_curr], "get")) {
               if (argc_curr+1 < argc) {
                  strcpy(ptr_command_text, argv[argc_curr]);
                  strcat(command_text, " ");
                  strcat(command_text, argv[argc_curr+1]);
               } else {
                  fprintf(stderr, "Get: no file specified\n"); 
                  exit(1);
               }
               argc_curr += 2;
               break;
            }
            // double argument commands
            if (!strcasecmp(argv[argc_curr], "set")) {
               if (argc_curr+2 < argc) {
                  strcpy(ptr_command_text, argv[argc_curr]);
                  strcat(command_text, " ");
                  strcat(command_text, argv[argc_curr+1]);
                  strcat(command_text, " ");
                  strcat(command_text, argv[argc_curr+2]);
               } else {
                  fprintf(stderr, "Connect: no host specified\n"); 
                  exit(1);
               }
               argc_curr += 3;
               break;
            }
            // unknown commands, skip
            fprintf(stderr, "Unsupported command console command: %s\n", argv[argc_curr]);
         }
         
      }

      /* parse the command */
      parse_command(&command, command_text);

      /* make sure we have at least one word */
      if (command.count == 0)
         continue;
         
      /* dispatch on the command type */
           if (!strcasecmp(command.text[0], "close"))             command_close  (&command, session);
      else if (!strcasecmp(command.text[0], "connect")) session = command_connect(&command, &parameter);
      else if (!strcasecmp(command.text[0], "get"))               command_get    (&command, session);
      else if (!strcasecmp(command.text[0], "dir"))               command_dir    (&command, session);
      else if (!strcasecmp(command.text[0], "help"))              command_help   (&command, session);
      else if (!strcasecmp(command.text[0], "quit"))              command_quit   (&command, session);
      else if (!strcasecmp(command.text[0], "exit"))              command_quit   (&command, session);
      else if (!strcasecmp(command.text[0], "bye"))               command_quit   (&command, session);
      else if (!strcasecmp(command.text[0], "set"))               command_set    (&command, &parameter);
      else
          fprintf(stderr, "Unrecognized command: '%s'.  Use 'HELP' for help.\n\n", command.text[0]);
    }

    /* if we're here, we shouldn't be */
    return 1;
}


/*------------------------------------------------------------------------
 * void parse_command(command_t *command, char *buffer);
 *
 * Given a buffer containing the text of a command, replaces the
 * whitespace with null terminators and fills the given command
 * structure with pointers to the words in the command.
 *------------------------------------------------------------------------*/
void parse_command(command_t *command, char *buffer)
{
    /* reset the count */
    command->count = 0;

    /* skip past initial whitespace */
    while (isspace(*buffer) && *buffer)
	++buffer;

    /* while we have command text left and not too many words */
    while ((command->count < MAX_COMMAND_WORDS) && *buffer) {

	/* save the start of the word */
	command->text[command->count++] = buffer;

	/* advance to the next whitespace (or the end) */
	while (*buffer && !isspace(*buffer))
	    ++buffer;

	/* convert the whitespace to terminators */
	while (*buffer && isspace(*buffer))
	    *(buffer++) = '\0';
    }
}


/*========================================================================
 * $Log: main.c,v $
 * Revision 1.10  2008/05/25 15:39:32  jwagnerhki
 * file client merge
 *
 * Revision 1.9  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.8  2007/05/31 09:32:07  jwagnerhki
 * removed some signedness warnings, added Mark5 server devel start code
 *
 * Revision 1.7  2006/12/05 15:24:50  jwagnerhki
 * now noretransmit code in client only, merged rt client code
 *
 * Revision 1.6  2006/12/04 14:45:34  jwagnerhki
 * added more proper TSUNAMI_CVS_BUILDNR, added exit and bye commands to client
 *
 * Revision 1.5  2006/10/19 07:44:26  jwagnerhki
 * show VSIB config at program start
 *
 * Revision 1.4  2006/10/19 07:26:51  jwagnerhki
 * clients now show proto version and build nr
 *
 * Revision 1.3  2006/10/16 09:08:20  jwagnerhki
 * added command get
 *
 * Revision 1.2  2006/09/08 11:59:36  jwagnerhki
 * quick hack to allow commands as arguments already from shell side
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:35:11  jwagnerhki
 * added to trunk
 *
 */
