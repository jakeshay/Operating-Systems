/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int proc;
    int pc;
    int page;
    int tmpProc, tmpPage = -1;
    int minTick = 2147483647;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		timestamps[proctmp][pagetmp] = 0; 
	    }
	}
	initialized = 1;
    }
    
    /* TODO: Implement LRU Paging */
    
    for(proc=0; proc<MAXPROCESSES; proc++) { 
	/* IS PROCESS ACTIVE */
	if(q[proc].active) {
	    pc = q[proc].pc; 		       
	    page = pc/PAGESIZE;
	    timestamps[proc][page] = tick;
	    /* IS PAGE SWAPPED OUT */ 	
	    if(!q[proc].pages[page]) {
		/* SWAP IN FAILED? */
		if(!pagein(proc,page)) {
		    /* FIND MIN TO SWAP OUT */
		    for (int i = 0; i < MAXPROCESSES; i++) {
			/* ONLY LOOK IN ACTIVE PROCESSES */
			if (q[i].active) {
				for (int j = 0; j < MAXPROCPAGES; j++) {
				    /* ONLY LOOK AT SWAPPED IN PAGES */
				     if (q[i].pages[j]) {
					     if (timestamps[i][j] != 0 && timestamps[i][j] < minTick) {
						if (!(i == proc && j == page)) {
							tmpProc = i;
							tmpPage = j;
						}
					     }
				     }
				}	
			}
		    }

		    pageout(tmpProc, tmpPage);
		    break;

		}
	    }

	}
    } 

    
    tick++;

} 
