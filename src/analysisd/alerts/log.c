/*   $OSSEC, log.c, v0.4, 2005/09/10, Daniel B. Cid$   */

/* Copyright (C) 2004,2005 Daniel B. Cid <dcid@ossec.net>
 * All right reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software 
 * Foundation
 */

/* v0.4 (2005/09/10): Added logging for multiple events
 * v0.3 (2005/02/10)
 */
 
/* Basic logging operations */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "log.h"
#include "alerts.h"
#include "getloglocation.h"

#include "error_messages/error_messages.h"

#include "headers/defs.h"
#include "headers/os_err.h"
#include "headers/debug_op.h"
#include "headers/file_op.h"

/* analysisd headers */
#include "rules.h"
#include "eventinfo.h"
#include "config.h"


/* OS_Store: v0.2, 2005/02/10 */
/* Will store the events in a file 
 * The string must be null terminated and contain
 * any necessary new lines, tabs, etc.
 *
 */
void OS_Store(Eventinfo *lf)
{
    if(_eflog) 
    {
        fprintf(_eflog,
            "%d %s %02d %s %s\n",
            lf->year,
            lf->mon,
            lf->day,
            lf->hour,
            lf->log);
   
        fflush(_eflog); 
    }
    else
    {
        merror("%s: File descriptor closed.",ARGV0);
    }
    
    return;	
}


/* OS_Log: v0.3, 2006/03/04 */
/* _writefile: v0.2, 2005/02/09 */
void OS_Log(Eventinfo *lf)
{
    /* Writting to the alert log file */
    if(_aflog) 
    {
        fprintf(_aflog,
                "** Alert: %s\n"
                "%d %s %02d %s %s\nRule: %d (level %d) -> '%s'\n"
                "Src IP: %s\nUser: %s\n%s\n",
                lf->mail_flag?"mail":"",
                lf->year,
                lf->mon,
                lf->day,
                lf->hour,
                lf->location,
                lf->sigid,
                lf->level,
                lf->comment,
                lf->srcip == NULL?"(none)":lf->srcip,
                lf->user == NULL?"(none)":lf->user,
                lf->sigid == STATS_PLUGIN?
                "No Log Available (HOURLY_STATS)":lf->log);

        
        /* Printing the last events if present */
        if(lf->lasts_lf)
        {
            char **lasts = lf->lasts_lf;
            while(*lasts)
            {
                fprintf(_aflog,"%s\n",*lasts);
                lasts++;
            }
        }
        
        fprintf(_aflog,"\n");
        
        fflush(_aflog);
    }
    else
    {
        merror("%s: File descriptor (aflog) closed. Error.",ARGV0);
    }
    
    return;	
}


/* FW_Log: v0.1, 2005/12/30 */
int FW_Log(Eventinfo *lf)
{
    /* If we don't have the srcip or the
     * action, there is no point in going
     * forward over here
     */
    if(!lf->action || !lf->srcip)
    {
        return(0);
    }


    /* Setting the actions */
    switch(*lf->action)
    {
        /* discard, drop, deny, */
        case 'd':
        case 'D':
        /* reject, */
        case 'r':
        case 'R':
        /* block */
        case 'b':
        case 'B':
            os_free(lf->action);
            os_strdup("DROP", lf->action);
            break;
        /* allow, accept, */    
        case 'a':
        case 'A':
        /* pass */
        case 'p':
        case 'P':
            os_free(lf->action);
            os_strdup("ALLOW", lf->action);        
            break;
        default:
            os_free(lf->action);
            os_strdup("UNKNOWN", lf->action);
            break;    
    }


    /* log to file */
    if(_fflog)
    {
        fprintf(_fflog,
                "%d %s %02d %s %s %s %s:%s->%s:%s\n",
                    lf->year,
                    lf->mon,
                    lf->day,
                    lf->hour,
                    lf->action,
                    lf->protocol,
                    lf->srcip,
                    lf->srcport,
                    lf->dstip,
                    lf->dstport);
        fflush(_fflog);
    }
    else
    {
        merror("%s: File descriptor closed.",ARGV0);
    }

    return(1);
}

/* EOF */
