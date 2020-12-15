/*
 * File: pager-basic.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a simple pageit
 *      implmentation at performs very poorly, but
 *      provided a basic idea of how to user the
 *      simulator interface. Your job is to improve
 *      upon this implmentation.
 */

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* Local vars */
    int proc;
    int pc;
    int page;
    int oldpage; 
    static int fleebs = -1;
    static int stackCount[MAXPROCESSES][214748];
    static int initialized = 0;
    static int printed[MAXPROCESSES];
    static int counter = 0;
    static int last = -1;

    if(!initialized){
	for(int proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(int pagetmp=0; pagetmp < 214748; pagetmp++){
		stackCount[proctmp][pagetmp] = 0; 
	    }
	    printed[proctmp] = 0;
	}
	initialized = 1;
    }
    

    /* Trivial paging strategy */
    /* Select first active process */ 
    for(proc=0; proc<MAXPROCESSES; proc++) { 
	/* Is process active? */
	if(q[proc].active && proc == 0) {
	    /* Dedicate all work to first active process*/ 
	    pc = q[proc].pc; 
	    if (counter < 4000) {
		if (pc != last) {
	    		printf("%d\n", pc);
			last = pc;
			counter++;
		}
	    }
	    
	    stackCount[proc][pc]++;
	    page = pc/PAGESIZE; 		// page the program counter needs
	    /* Is page swapped-out? */
	    if(!q[proc].pages[page]) {
		/* Try to swap in */
		if(!pagein(proc,page)) {
		    /* If swapping fails, swap out another page */
		    for(oldpage=0; oldpage < q[proc].npages; oldpage++) {
	 		/* Make sure page isn't one I want */
			if(oldpage != page) {
			    /* Try to swap-out */
			    if(pageout(proc,oldpage)) {
				/* Break loop once swap-out starts*/
				break;
			    } 
			}
		    }
		}
	    }
	    /* Break loop after finding first active process */
	    break;
	}
	//else {
	/*    if (printed[proc] == 0 && proc == 0) {
		    printed[proc] = 1;
		    for(int x =0; x < 214748; x++) {
			if (stackCount[proc][x]) {
				printf("PC: %d, %d times\n", x, stackCount[proc][x]);
			}
		    }
	    }
	}*/
    }

} 
