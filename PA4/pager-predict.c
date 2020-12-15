/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <time.h>

#include "simulator.h"

int genRand(int low, int high) {
    int num = (rand() % (high - low + 1)) + low;
    return num;
}

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int progGuesses[MAXPROCESSES]; // track program type by process
    static int last[MAXPROCESSES]; // holds the last pc used, useful for classification
    
    /* Local vars */
    int proctmp;
    int proc;
    int pc;
    int page;
    int newPage;
    int newPage2;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    progGuesses[proctmp] = -1;
	    last[proctmp] = -1;
	}
	initialized = 1;
    }
    
    for(proc=0; proc<MAXPROCESSES; proc++) { 
	/* IS PROCESS ACTIVE */
	if(q[proc].active) {
	    pc = q[proc].pc;
	    /*Classify process by type by tracking pc*/
	    if (last[proc] == 1129 && pc == 0) {
		progGuesses[proc] = 2;
	    }
	    if (pc == 0 && (last[proc] == 501 || last[proc] == 503)) {
		progGuesses[proc] = 5;
	    }
	    if (pc == 1166 && last[proc] == 1682) {
		progGuesses[proc] = 3;
	    }
	    if ((pc == 0 && last[proc] == 1533) || (pc == 1402 && last[proc] == 500)) {
		progGuesses[proc] = 1;
	    }
	    if (pc != last[proc]) {
		last[proc] = pc;
	    }

	    /*Default behavior: try to page in next page needed and page out last page*/
	    page = pc/PAGESIZE;
	    newPage = (pc + 128)/PAGESIZE;
	    newPage2 = (pc - 128)/PAGESIZE;

	    /*Select page to page in and page to page out according to the program type*/
	    switch(progGuesses[proc]) {
		case 1:
		    if (pc == 0) {
			newPage2 = 1533/PAGESIZE;
			}
		    if (pc == 399) {
			newPage = 1402/PAGESIZE;
		    }
		    if (pc == 1200) {
			newPage = 0;
		    }
		    if (pc == 1300) {
			newPage = 1533/PAGESIZE;
		    }
		    if (pc == 1432) {
			newPage = 0;
		    }
		    if (pc == 1534) {
			progGuesses[proc] = -1;
		    }
		    break;
		case 2:
		    if (pc == 0) {
			newPage2 = 1129/PAGESIZE;
		    }
		    if (pc == 1028) {
			newPage = 0/PAGESIZE;
		    }
		    if (pc == 1130) {
			progGuesses[proc] = -1;
		    }
		    break;
		case 3:
		    if (pc == 1166) {
			newPage2 = 0/PAGESIZE;
		    }
		    if (pc == 1481) {
			newPage = 0/PAGESIZE;
		    }
		    if (pc == 1581) {
			newPage = 1166/PAGESIZE;
		    }
		    break;
		case 5:
		    if (pc == 0) {
			newPage2 = 502/PAGESIZE;
		    }
		    if (pc == 400) {
			newPage = 0/PAGESIZE;
		    }
		    if (pc == 504) {
			progGuesses[proc] = -1;
		    }
		    break;
		default:
		    if (pc == 400) {
			newPage = 0/PAGESIZE;
		    }
		    if (pc == 1581) {
			newPage = 1166/PAGESIZE;
		    }
		    if (pc == 1301) {
			newPage = 0/PAGESIZE;
		    }
		    if (pc == 1402) {
			newPage = 1408/PAGESIZE;
		    }

	    }
	
	    /*Swap out pages when necessary*/
	    if (q[proc].pages[page]) {
		page = newPage;
		if (pc > 128) {
		    pageout(proc, newPage2);
	 	}
		else if (pc == 0 && (progGuesses[proc] == 2 || progGuesses[proc] == 5)) {
		    pageout(proc, newPage2);
		}
	    }

	    /* IS PAGE SWAPPED OUT */ 	
	    if(!q[proc].pages[page]) {
		/* SWAP IN FAILED? */
		if(!pagein(proc,page)) {
		    pageout(proc, newPage2);
		}
		
	    }

	}
    }

    /* advance time for next pageit iteration */
    tick++;
} 


