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

public class CoreListJoinA {

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

	public static class ListData {
		public short data16;
		public short idx;
	}

	public static class ListHead {
		ListHead next;
		ListData info;
	}

	// This is not very efficient, but it's only used during initialisation
	private static class ShortWrapper {
		private short val;
		public ShortWrapper(short val) {
			this.val = val;
		}
		public short GetValue() {
			return val;
		}
		public void SetValue(short val) {
			this.val = val;
		}
	}

	private static abstract class AbstractListDataCompare {
		abstract int compare(ListData a, ListData b, CoreResults res);
	}

	/* Function: cmp_complex
		Compare the data item in a list cell.

		Can be used by mergesort.
	*/
	private static class CmpComplex extends AbstractListDataCompare {
		short calc_func(ListData pdata, CoreResults res) {
			short data=pdata.data16;
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
				pdata.data16=(short)((data & 0xff00) | 0x0080 | retval); /* cache the result */
				return retval;
			}
		}

		public int compare(ListData a, ListData b, CoreResults res) {
			short val1=calc_func(a,res);
			short val2=calc_func(b,res);
			return val1 - val2;
		}
	}
	private static CmpComplex cmp_complex = new CmpComplex();

	/* Function: cmp_idx
		Compare the idx item in a list cell, and regen the data.

		Can be used by mergesort.
	*/
	private static class CmpIdx extends AbstractListDataCompare {
		public int compare(ListData a, ListData b, CoreResults res) {
			if (res==null) {
				a.data16=(short)((a.data16 & 0xff00) | (0x00ff & (a.data16>>8)));
				b.data16=(short)((b.data16 & 0xff00) | (0x00ff & (b.data16>>8)));
			}
			return a.idx - b.idx;
		}
	}
	private static CmpIdx cmp_idx = new CmpIdx();


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
		ListHead list=res.list_CoreListJoinA;
		short find_num=res.seed3;
		ListHead this_find;
		ListHead finder, remover;
		ListData info=new ListData();
		short i;

		info.idx=finder_idx;
		/* find <find_num> values in the list, and change the list each time (reverse and cache if value found) */
		for (i=0; i<find_num; i++) {
			info.data16=(short)(i & 0xff);
			this_find=core_list_find(list,info);
			list=core_list_reverse(list);
			if (this_find==null) {
				missed++;
				retval+=(list.next.info.data16 >> 8) & 1;
			}
			else {
				found++;
				if ((this_find.info.data16 & 0x1) != 0) /* use found value */
					retval+=(this_find.info.data16 >> 9) & 1;
				/* and cache next item at the head of the list (if any) */
				if (this_find.next != null) {
					finder = this_find.next;
					this_find.next=finder.next;
					finder.next=list.next;
					list.next=finder;
				}
			}
			if (info.idx>=0)
				info.idx++;
		}
		retval+=found*4-missed;
		/* sort the list by data content and remove one item*/
		if (finder_idx>0)
			list=core_list_mergesort(list,cmp_complex,res);
		remover=core_list_remove(list.next);
		/* CRC data content of list from location of index N forward, and then undo remove */
		finder=core_list_find(list,info);
		if (finder==null)
			finder=list.next;
		while (finder!=null) {
			retval=CoreUtil.crc16(list.info.data16,retval);
			finder=finder.next;
		}
		remover=core_list_undo_remove(remover, list.next);
		/* sort the list by index, in effect returning the list to original state */
		list=core_list_mergesort(list,cmp_idx,null);
		/* CRC data content of list */
		finder=list.next;
		while (finder!=null) {
			retval=CoreUtil.crc16(list.info.data16,retval);
			finder=finder.next;
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
	static ListHead core_list_init(int blksize, short seed) {
		/* calculated pointers for the list */
		int per_item=16+4; //16+sizeof(struct list_data_s);
		int size=(blksize/per_item)-2; /* to accomodate systems with 64b pointers, and make sure same code is executed, set max list elements */
 
 		ShortWrapper memblock = new ShortWrapper((short)0);
		short memblock_end=(short)(size*2); // *2 because in the C version we count in pointers to list_head structs, which are 4 bytes, but in Java we count 2 byte shorts. So we need to reserve *2 as much memory.
		ShortWrapper datablock = new ShortWrapper(memblock_end);
		short datablock_end=(short)(datablock.GetValue()+(size*2)); // *2 because in the C version we count in pointers to list_data structs, which are 4 bytes, but in Java we count 2 byte shorts. So we need to reserve *2 as much memory.

		/* some useful variables */
		int i;
		ListHead finder, list=new ListHead();
		ListData info=new ListData();
 
 		/* create a fake items for the list head and tail */
		list.next=null;
		list.info=new ListData();
		list.info.idx=(short)0x0000;
		list.info.data16=(short)0x8080;
		memblock.SetValue((short)(memblock.GetValue()+2)); // +2 because we're counting shorts instead of structs
		datablock.SetValue((short)(datablock.GetValue()+2)); // +2 because we're counting shorts instead of structs
		info.idx=(short)0x7fff;
		info.data16=(short)0xffff;
		core_list_insert_new(list,info,memblock,datablock,memblock_end,datablock_end);
 
 		/* then insert size items */
		for (i=0; i<size; i++) {
			short datpat=(short)((seed^i) & 0xf);
			short dat=(short)((datpat<<3) | (i&0x7)); /* alternate between algorithms */
			info.data16=(short)((dat<<8) | dat);		/* fill the data with actual data and upper bits with rebuild value */
			core_list_insert_new(list,info,memblock,datablock,memblock_end,datablock_end);
		}
		/* and now index the list so we know initial seed order of the list */
		finder=list.next;
		i=1;
		while (finder.next!=null) {
			if (i<size/5) /* first 20% of the list in order */
				finder.info.idx=(short)(i++);
			else { 
				short pat=(short)(i++ ^ seed); /* get a pseudo random number */
				finder.info.idx=(short)(0x3fff & (((i & 0x07) << 8) | pat)); /* make sure the mixed items end up after the ones in sequence */
			}
			finder=finder.next;
		}
		list = core_list_mergesort(list,cmp_idx,null);
 
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
	static ListHead core_list_insert_new(ListHead insert_point, ListData info, ShortWrapper memblock, ShortWrapper datablock
		, short memblock_end, short datablock_end) {
		ListHead newitem;
		
		short memblock_val = memblock.GetValue();
		short datablock_val = datablock.GetValue();

		// These values are copied from CoreListJoinB since they should still work the same, even if the pointers aren't actually used anymore.
		// if ((*memblock+1) >= memblock_end)
		if ((memblock_val+2) >= memblock_end) // +2 because it's not a list_head pointer anymore, so we need to count 2 shorts instead of 1 struct
			return null;
		// if ((*datablock+1) >= datablock_end)
		if ((datablock_val+2) >= datablock_end) // +2 because it's not a list_head pointer anymore, so we need to count 2 shorts instead of 1 struct
			return null;
			
		newitem=new ListHead();
		// (*memblock)++;
		memblock.SetValue((short)(memblock_val+2)); // +2, see above
		newitem.next=insert_point.next;
		insert_point.next=newitem;
		
		newitem.info=new ListData();
		// (*datablock)++;
		datablock.SetValue((short)(datablock_val+2)); // +2, see above
		// copy_info(newitem->info,info);
		newitem.info.idx=info.idx;
		newitem.info.data16=info.data16;

		return newitem;
	}

	/* Function: core_list_remove
		Remove an item from the list.

		Operation:
		For a singly linked list, remove by copying the data from the next item 
		over to the current cell, and unlinking the next item.

		Note: 
		since there is always a fake item at the end of the list, no need to check for null.

		Returns:
		Removed item.
	*/
	// list_head *core_list_remove(list_head *item) {
	static ListHead core_list_remove(ListHead item) {
		ListData tmp;
		ListHead ret=item.next;
		/* swap data pointers */
		tmp=item.info;
		item.info=ret.info;
		ret.info=tmp;
		/* and eliminate item */
		item.next=item.next.next;
		ret.next=null;
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
	static ListHead core_list_undo_remove(ListHead item_removed, ListHead item_modified) {
		ListData tmp;
		/* swap data pointers */
		tmp=item_removed.info;
		item_removed.info=item_modified.info;
		item_modified.info=tmp;
		/* and insert item */
		item_removed.next=item_modified.next;
		item_modified.next=item_removed;
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
		Found item, or null if not found.
	*/
	// list_head *core_list_find(list_head *list,list_data *info);
	static ListHead core_list_find(ListHead list, ListData info) {
		if (info.idx>=0) {
			while (list != null && (list.info.idx != info.idx))
				list=list.next;
			return list;
		} else {
			while (list != null && ((list.info.data16 & 0xff) != info.data16))
				list=list.next;
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
		Found item, or null if not found.
	*/
	// list_head *core_list_reverse(list_head *list);
	static ListHead core_list_reverse(ListHead list) {
		ListHead next=null, tmp;
		while (list!=null) {
			tmp=list.next;
			list.next=next;
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
	static ListHead core_list_mergesort(ListHead list, AbstractListDataCompare cmp, CoreResults res) {
	    ListHead p, q, e, tail;
	    int insize, nmerges, psize, qsize, i;

	    insize = 1;

	    while (true) {
	        p = list;
	        list = null;
	        tail = null;

	        nmerges = 0;  /* count number of merges we do in this pass */

	        while (p!=null) {
	            nmerges++;  /* there exists a merge to be done */
	            /* step `insize' places along from p */
	            q = p;
	            psize = 0;
	            for (i = 0; i < insize; i++) {
	                psize++;
				    q = q.next;
	                if (q==null) break;
	            }

	            /* if q hasn't fallen off end, we have two lists to merge */
	            qsize = insize;

	            /* now we have two lists; merge them */
	            while (psize > 0 || (qsize > 0 && q!=null)) {

					/* decide whether next element of merge comes from p or q */
					if (psize == 0) {
					    /* p is empty; e must come from q. */
					    e = q; q = q.next; qsize--;
					} else if (qsize == 0 || q==null) {
					    /* q is empty; e must come from p. */
					    e = p; p = p.next; psize--;
					} else if (cmp.compare(p.info,q.info,res) <= 0) {
					    /* First element of p is lower (or same); e must come from p. */
					    e = p; p = p.next; psize--;
					} else {
					    /* First element of q is lower; e must come from q. */
					    e = q; q = q.next; qsize--;
					}

			        /* add the next element to the merged list */
					if (tail!=null) {
					    tail.next = e;
					} else {
					    list = e;
					}
					tail = e;
		        }

				/* now p has stepped `insize' places along, and q has too */
				p = q;
	        }
			
		    tail.next = null;

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







