package javax.rtcbench;

import javax.rtc.RTC;

// This is the benchmark that most clearly shows the differences between Java and C.
// I see three possible options to port this
//   a) Create many separate objects for each linked-list element and link them with normal Java references
//   b) Use a global static byte array, and pass short indexes instead of pointers,
//      then add some tiny (inlined) methods to access the fields of the list elements
//   c) Instead of global static array, which is ugly, use a small object containing the byte array and
//      index into the array.


/*
Author : Shay Gal-On, EEMBC

This file is part of  EEMBC(R) and CoreMark(TM), which are Copyright (C) 2009 
All rights reserved.                            

EEMBC CoreMark Software is a product of EEMBC and is provided under the terms of the
CoreMark License that is distributed with the official EEMBC COREMARK Software release. 
If you received this EEMBC CoreMark Software without the accompanying CoreMark License, 
you must discontinue use and download the official release from www.coremark.org.  

Also, if you are publicly displaying scores generated from the EEMBC CoreMark software, 
make sure that you are in compliance with Run and Reporting rules specified in the accompanying readme.txt file.

EEMBC 
4354 Town Center Blvd. Suite 114-200
El Dorado Hills, CA, 95762 
*/ 

/*
Topic: Description
	Benchmark using a linked list.

	Linked list is a common data structure used in many applications.
	
	For our purposes, this will excercise the memory units of the processor.
	In particular, usage of the list pointers to find and alter data.
	
	We are not using Malloc since some platforms do not support this library.
	
	Instead, the memory block being passed in is used to create a list,
	and the benchmark takes care not to add more items then can be
	accomodated by the memory block. The porting layer will make sure
	that we have a valid memory block.
	
	All operations are done in place, without using any extra memory.
	
	The list itself contains list pointers and pointers to data items.
	Data items contain the following:
	
	idx - An index that captures the initial order of the list.
	data - Variable data initialized based on the input parameters. The 16b are divided as follows:
	o Upper 8b are backup of original data.
	o Bit 7 indicates if the lower 7 bits are to be used as is or calculated.
	o Bits 0-2 indicate type of operation to perform to get a 7b value.
	o Bits 3-6 provide input for the operation.
	
*/

public class CoreListJoinB {

	// /* list data structures */
	// typedef struct list_data_s {
	// 	ee_s16 data16;
	// 	ee_s16 idx;
	// } list_data;
	//
	// typedef struct list_head_s {
	// 	struct list_head_s *next;
	// 	struct list_data_s *info;
	// } list_head;

	// Since both a pointer and the data and idx fields are 16 bit,
	// we model de data as an array of shorts.
	// Pointers *next and *info will be indexes into this array. The
	// next two shorts at that location will contain the data of the
	// list_head_s and list_data_s structures respectively.
	// So the data array consists of pairs of two shorts which may be
	// either a list_head_s or list_data_s struct.
	public static short[] data;


	private final static short ListNULL = -1;

	// These should be tiny enough to inline by ProGuard.
	// We should also do a manual inline to see if there is a big
	// difference or not (since ProGuard inlining isn't as efficient
	// as manual inlining)
	private static short ListData_GetData16(short ptr) {
		return data[ptr];
	}
	private static short ListData_GetIdx(short ptr) {
		return data[ptr+1];
	}
	private static void ListData_SetData16(short ptr, short val) {
		data[ptr] = val;
	}
	private static void ListData_SetIdx(short ptr, short val) {
		data[ptr+1] = val;
	}

	private static short ListHead_GetNext(short ptr) {
		return data[ptr];
	}
	private static short ListHead_GetInfo(short ptr) {
		return data[ptr+1];
	}
	private static void ListHead_SetNext(short ptr, short val) {
		data[ptr] = val;
	}
	private static void ListHead_SetInfo(short ptr, short val) {
		data[ptr+1] = val;
	}

	/* Function: cmp_complex
		Compare the data item in a list cell.

		Can be used by mergesort.
	*/
	private static class CmpComplex {
		static short calc_func(short pdata, CoreResults res) {
			short data=ListData_GetData16(pdata);
			short retval;
			byte optype=(byte)((data>>7) & 1); /* bit 7 indicates if the function result has been cached */
			if (optype!=0) /* if cached, use cache */
				return (short)(data & 0x007f);
			else { /* otherwise calculate and cache the result */
				short flag=(short)(data & 0x7); /* bits 0-2 is type of function to perform */
				short dtype=(short)((data>>3) & 0xf); /* bits 3-6 is specific data for the operation */
				dtype |= dtype << 4; /* replicate the lower 4 bits to get an 8b value */
				switch (flag) {
					case 0:
						if (dtype<0x22) /* set min period for bit corruption */
							dtype=0x22;
						retval=CoreState.core_bench_state(res.size,res.statememblock3,res.seed1,res.seed2,dtype,res.crc);
						if (res.crcstate==0)
							res.crcstate=retval;
						break;
					case 1:
						retval=CoreMatrix.core_bench_matrix(res.mat,dtype,res.crc);
						if (res.crcmatrix==0)
							res.crcmatrix=retval;
						break;
					default:
						retval=data;
						break;
				}
				res.crc=CoreUtil.crcu16(retval,res.crc);
				retval &= 0x007f; 
				ListData_SetData16(pdata, (short)((data & 0xff00) | 0x0080 | retval)); /* cache the result */
				return retval;
			}
		}

		public static int compare(short a, short b, CoreResults res) {
			short val1=calc_func(a,res);
			short val2=calc_func(b,res);
			return val1 - val2;
		}
	}

	/* Function: cmp_idx
		Compare the idx item in a list cell, and regen the data.

		Can be used by mergesort.
	*/
	private static class CmpIdx {
		public static int compare(short a, short b, CoreResults res) {
			if (res==null) {
				ListData_SetData16(a, (short)((ListData_GetData16(a) & 0xff00) | (0x00ff & (ListData_GetData16(a)>>8))));
				ListData_SetData16(b, (short)((ListData_GetData16(b) & 0xff00) | (0x00ff & (ListData_GetData16(b)>>8))));
			}
			return ListData_GetIdx(a) - ListData_GetIdx(b);
		}
	}


	/* Benchmark for linked list:
		- Try to find multiple data items.
		- List sort
		- Operate on data from list (crc)
		- Single remove/reinsert
		* At the end of this function, the list is back to original state
	*/
	static short core_bench_list(CoreResults res, short finder_idx) {
		short retval=0;
		short found=0,missed=0;
		short list=res.list_CoreListJoinB;
		short find_num=res.seed3;
		short this_find;
		short finder, remover;
		short info_data16=0;
		short info_idx;
		short i;

		info_idx=finder_idx;
		/* find <find_num> values in the list, and change the list each time (reverse and cache if value found) */
		for (i=0; i<find_num; i++) {
			info_data16=(short)(i & 0xff);
			this_find=core_list_find(list,info_data16,info_idx);
			list=core_list_reverse(list);
			if (this_find==ListNULL) {
				missed++;
				retval+=(ListData_GetData16(ListHead_GetInfo(ListHead_GetNext(list))) >> 8) & 1;
			}
			else {
				found++;
				if ((ListData_GetData16(ListHead_GetInfo(this_find)) & 0x1) != 0) /* use found value */
					retval+=(ListData_GetData16(ListHead_GetInfo(this_find)) >> 9) & 1;
				/* and cache next item at the head of the list (if any) */
				if (ListHead_GetNext(this_find) != ListNULL) {
					finder = ListHead_GetNext(this_find);
					ListHead_SetNext(this_find, ListHead_GetNext(finder));
					ListHead_SetNext(finder, ListHead_GetNext(list));
					ListHead_SetNext(list, finder);
				}
			}
			if (info_idx>=0)
				info_idx++;
		}
		retval+=found*4-missed;
		/* sort the list by data content and remove one item*/
		if (finder_idx>0)
			list=core_list_mergesort(list,false,res);
		remover=core_list_remove(ListHead_GetNext(list));
		/* CRC data content of list from location of index N forward, and then undo remove */
		finder=core_list_find(list,info_data16,info_idx);
		if (finder==ListNULL)
			finder=ListHead_GetNext(list);
		while (finder!=ListNULL) {
			retval=CoreUtil.crc16(ListData_GetData16(ListHead_GetInfo(list)),retval);
			finder=ListHead_GetNext(finder);
		}
		remover=core_list_undo_remove(remover, ListHead_GetNext(list));
		/* sort the list by index, in effect returning the list to original state */
		list=core_list_mergesort(list,true,null);
		/* CRC data content of list */
		finder=ListHead_GetNext(list);
		while (finder!=ListNULL) {
			retval=CoreUtil.crc16(ListData_GetData16(ListHead_GetInfo(list)),retval);
			finder=ListHead_GetNext(finder);
		}
		return retval;
	}

	/* Function: core_list_init
		Initialize list with data.

		Parameters:
		blksize - Size of memory to be initialized.
		memblock - Pointer to memory block.
		seed - 	Actual values chosen depend on the seed parameter.
			The seed parameter MUST be supplied from a source that cannot be determined at compile time

		Returns:
		Pointer to the head of the list.

	*/
	// list_head *core_list_init(ee_u32 blksize, list_head *memblock, ee_s16 seed) {
	static short core_list_init(int blksize, short seed) {
		System.out.println("Using CoreListJoinB");

		/* calculated pointers for the list */
		int per_item=16+4; //16+sizeof(struct list_data_s);
		int size=(blksize/per_item)-2; /* to accomodate systems with 64b pointers, and make sure same code is executed, set max list elements */

		ShortWrapper memblock = new ShortWrapper();
		memblock.value = (short)0;
		short memblock_end=(short)(size*2); // *2 because in the C version we count in pointers to list_head structs, which are 4 bytes, but in Java we count 2 byte shorts. So we need to reserve *2 as much memory.
		ShortWrapper datablock = new ShortWrapper();
		datablock.value = memblock_end;
		short datablock_end=(short)(datablock.value+(size*2)); // *2 because in the C version we count in pointers to list_data structs, which are 4 bytes, but in Java we count 2 byte shorts. So we need to reserve *2 as much memory.

		data = new short[datablock_end];

		/* some useful variables */
		int i;
		short finder, list=0;
		short info_data16;
		short info_idx;

		/* create a fake items for the list head and tail */
		ListHead_SetNext(list, ListNULL);
		ListHead_SetInfo(list, datablock.value);
		ListData_SetIdx(ListHead_GetInfo(list), (short)0x0000);
		ListData_SetData16(ListHead_GetInfo(list), (short)0x8080);
		memblock.value=((short)(memblock.value+2)); // +2 because we're counting shorts instead of structs
		datablock.value=((short)(datablock.value+2)); // +2 because we're counting shorts instead of structs
		info_idx=(short)0x7fff;
		info_data16=(short)0xffff;
		core_list_insert_new(list,info_data16,info_idx,memblock,datablock,memblock_end,datablock_end);
		
		/* then insert size items */
		for (i=0; i<size; i++) {
			short datpat=(short)((seed^i) & 0xf);
			short dat=(short)((datpat<<3) | (i&0x7)); /* alternate between algorithms */
			info_data16=(short)((dat<<8) | dat);		/* fill the data with actual data and upper bits with rebuild value */
			core_list_insert_new(list,info_data16,info_idx,memblock,datablock,memblock_end,datablock_end);
		}
		/* and now index the list so we know initial seed order of the list */
		finder=ListHead_GetNext(list);
		i=1;
		while (ListHead_GetNext(finder)!=ListNULL) {
			if (i<size/5) /* first 20% of the list in order */
				ListData_SetIdx(ListHead_GetInfo(finder), (short)(i++));
			else { 
				short pat=(short)(i++ ^ seed); /* get a pseudo random number */
				ListData_SetIdx(ListHead_GetInfo(finder), (short)(0x3fff & (((i & 0x07) << 8) | pat))); /* make sure the mixed items end up after the ones in sequence */
			}
			finder=ListHead_GetNext(finder);
		}
		list = core_list_mergesort(list,true,null);

		return list;
	}

	/* Function: core_list_insert
		Insert an item to the list

		Parameters:
		insert_point - where to insert the item.
		info - data for the cell.
		memblock - pointer for the list header
		datablock - pointer for the list data
		memblock_end - end of region for list headers
		datablock_end - end of region for list data

		Returns:
		Pointer to new item.
	*/
	// list_head *core_list_insert_new(list_head *insert_point, list_data *info, list_head **memblock, list_data **datablock
	// , list_head *memblock_end, list_data *datablock_end) {
	static short core_list_insert_new(short insert_point, short info_data16, short info_idx, ShortWrapper memblock, ShortWrapper datablock
		, short memblock_end, short datablock_end) {
		short newitem;
		
		short memblock_val = memblock.value;
		short datablock_val = datablock.value;

		// if ((*memblock+1) >= memblock_end)
		if ((memblock_val+2) >= memblock_end) // +2 because it's not a list_head pointer anymore, so we need to count 2 shorts instead of 1 struct
			return ListNULL;
		// if ((*datablock+1) >= datablock_end)
		if ((datablock_val+2) >= datablock_end) // +2 because it's not a list_head pointer anymore, so we need to count 2 shorts instead of 1 struct
			return ListNULL;
			
		newitem=memblock_val;
		// (*memblock)++;
		memblock.value=((short)(memblock_val+2)); // +2, see above
		ListHead_SetNext(newitem, ListHead_GetNext(insert_point));
		ListHead_SetNext(insert_point, newitem);
		
		ListHead_SetInfo(newitem, datablock.value);
		// (*datablock)++;
		datablock.value=((short)(datablock_val+2)); // +2, see above
		// copy_info(newitem->info,info);
		// void copy_info(list_data *to,list_data *from) {
		// 	to->data16=from->data16;
		// 	to->idx=from->idx;
		// }
		ListData_SetData16(ListHead_GetInfo(newitem), info_data16);
		ListData_SetIdx(ListHead_GetInfo(newitem), info_idx);

		return newitem;
	}

	/* Function: core_list_remove
		Remove an item from the list.

		Operation:
		For a singly linked list, remove by copying the data from the next item 
		over to the current cell, and unlinking the next item.

		Note: 
		since there is always a fake item at the end of the list, no need to check for NULL.

		Returns:
		Removed item.
	*/
	// list_head *core_list_remove(list_head *item) {
	static short core_list_remove(short item) {
		short tmp;
		short ret=ListHead_GetNext(item);
		/* swap data pointers */
		tmp=ListHead_GetInfo(item);
		ListHead_SetInfo(item, ListHead_GetInfo(ret));
		ListHead_SetInfo(ret, tmp);
		/* and eliminate item */
		ListHead_SetNext(item, ListHead_GetNext(ListHead_GetNext(item)));
		ListHead_SetNext(ret, ListNULL);
		return ret;
	}

	/* Function: core_list_undo_remove
		Undo a remove operation.

		Operation:
		Since we want each iteration of the benchmark to be exactly the same,
		we need to be able to undo a remove. 
		Link the removed item back into the list, and switch the info items.

		Parameters:
		item_removed - Return value from the <core_list_remove>
		item_modified - List item that was modified during <core_list_remove>

		Returns:
		The item that was linked back to the list.
		
	*/
	// list_head *core_list_undo_remove(list_head *item_removed, list_head *item_modified) {
	static short core_list_undo_remove(short item_removed, short item_modified) {
		short tmp;
		/* swap data pointers */
		tmp=ListHead_GetInfo(item_removed);
		ListHead_SetInfo(item_removed, ListHead_GetInfo(item_modified));
		ListHead_SetInfo(item_modified, tmp);
		/* and insert item */
		ListHead_SetNext(item_removed, ListHead_GetNext(item_modified));
		ListHead_SetNext(item_modified, item_removed);
		return item_removed;
	}

	/* Function: core_list_find
		Find an item in the list

		Operation:
		Find an item by idx (if not 0) or specific data value

		Parameters:
		list - list head
		info - idx or data to find

		Returns:
		Found item, or NULL if not found.
	*/
	// list_head *core_list_find(list_head *list,list_data *info);
	static short core_list_find(short list, short info_data16, short info_idx) {
		if (info_idx>=0) {
			while (list != ListNULL && (ListData_GetIdx(ListHead_GetInfo(list)) != info_idx))
				list=ListHead_GetNext(list);
			return list;
		} else {
			while (list != ListNULL && ((ListData_GetData16(ListHead_GetInfo(list)) & 0xff) != info_data16))
				list=ListHead_GetNext(list);
			return list;
		}
	}

	/* Function: core_list_reverse
		Reverse a list

		Operation:
		Rearrange the pointers so the list is reversed.

		Parameters:
		list - list head
		info - idx or data to find

		Returns:
		Found item, or NULL if not found.
	*/
	// list_head *core_list_reverse(list_head *list);
	static short core_list_reverse(short list) {
		short next=ListNULL, tmp;
		while (list!=ListNULL) {
			tmp=ListHead_GetNext(list);
			ListHead_SetNext(list, next);
			next=list;
			list=tmp;
		}
		return next;
	}

	/* Function: core_list_mergesort
		Sort the list in place without recursion.

		Description:
		Use mergesort, as for linked list this is a realistic solution. 
		Also, since this is aimed at embedded, care was taken to use iterative rather then recursive algorithm.
		The sort can either return the list to original order (by idx) ,
		or use the data item to invoke other other algorithms and change the order of the list.

		Parameters:
		list - list to be sorted.
		cmp - cmp function to use

		Returns:
		New head of the list.

		Note: 
		We have a special header for the list that will always be first,
		but the algorithm could theoretically modify where the list starts.

	 */
	// list_head *core_list_mergesort(list_head *list, list_cmp cmp, core_results *res) {
	static short core_list_mergesort(short list, boolean useIdxCompare, CoreResults res) {
	    short p, q, e, tail;
	    int insize, nmerges, psize, qsize, i;

	    insize = 1;

	    while (true) {
	        p = list;
	        list = ListNULL;
	        tail = ListNULL;

	        nmerges = 0;  /* count number of merges we do in this pass */

	        while (p!=ListNULL) {
	            nmerges++;  /* there exists a merge to be done */
	            /* step `insize' places along from p */
	            q = p;
	            psize = 0;
	            for (i = 0; i < insize; i++) {
	                psize++;
				    q = ListHead_GetNext(q);
	                if (q==ListNULL) break;
	            }

	            /* if q hasn't fallen off end, we have two lists to merge */
	            qsize = insize;

	            /* now we have two lists; merge them */
	            while (psize > 0 || (qsize > 0 && q!=ListNULL)) {

					/* decide whether next element of merge comes from p or q */
					if (psize == 0) {
					    /* p is empty; e must come from q. */
					    e = q; q = ListHead_GetNext(q); qsize--;
					} else if (qsize == 0 || q==ListNULL) {
					    /* q is empty; e must come from p. */
					    e = p; p = ListHead_GetNext(p); psize--;
					} else if ((useIdxCompare ? CmpIdx.compare(ListHead_GetInfo(p),ListHead_GetInfo(q),res) : CmpComplex.compare(ListHead_GetInfo(p),ListHead_GetInfo(q),res)) <= 0) {
					    /* First element of p is lower (or same); e must come from p. */
					    e = p; p = ListHead_GetNext(p); psize--;
					} else {
					    /* First element of q is lower; e must come from q. */
					    e = q; q = ListHead_GetNext(q); qsize--;
					}

			        /* add the next element to the merged list */
					if (tail!=ListNULL) {
					    ListHead_SetNext(tail, e);
					} else {
					    list = e;
					}
					tail = e;
		        }

				/* now p has stepped `insize' places along, and q has too */
				p = q;
	        }
			
		    ListHead_SetNext(tail, ListNULL);

	        /* If we have done only one merge, we're finished. */
	        if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
	            return list;

	        /* Otherwise repeat, merging lists twice the size */
	        insize *= 2;
	    }

	    // Unreachable
		// return list;
	}
}







