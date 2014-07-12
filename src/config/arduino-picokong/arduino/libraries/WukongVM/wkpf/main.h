/*
 * main.h
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef __MAIN_H__
#define __MAIN_H__
#include "contiki.h"

#include "net/rime.h"
#include "sys/pt.h"

#include "jlib_base.h"
#include "jlib_darjeeling.h"
#include "pointerwidth.h"


#ifdef HAS_USART
#include "dev/rs232.h"
#endif

#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
void init_connections(struct broadcast_conn * broadcast_connection, struct rmh_conn* rmhunicast_connection);
#else
void init_connections(struct broadcast_conn * broadcast_connection, struct unicast_conn* unicast_connection);
#endif
void fill_broadcast_buffer(char* data, int length);
void fill_unicast_buffer(char* data, int length, rimeaddr_t to);

#ifdef WITH_MULTIHOP_RELIABLE_UNICAST
#define NUM_OF_MAX_TRIES 1
#define MAX_HOPS 4
#define NEIGHBOR_TIMEOUT 6000 * CLOCK_SECOND
#define MAX_NEIGHBORS 16

//int putchar(int input);
struct example_neighbor {
  struct example_neighbor *next;
  rimeaddr_t addr;
  struct ctimer ctimer;
};

LIST(neighbor_table);
MEMB(neighbor_mem, struct example_neighbor, MAX_NEIGHBORS);
//TODO:the following two function might be moved back to javax_radio_Radio.c
//the reason not doing that is that we are going to use simple single-hop unicast
//so, I won't spend time on checking this in early future
/*
 * This function is called by the ctimer present in each neighbor
 * table entry. The function removes the neighbor from the table
 * because it has become too old.
 */
static void
remove_neighbor(void *n)
{
  struct example_neighbor *e = n;

  list_remove(neighbor_table, e);
  memb_free(&neighbor_mem, e);
}

/*
 * This function is called when an incoming announcement arrives. The
 * function checks the neighbor table to see if the neighbor is
 * already present in the list. If the neighbor is not present in the
 * list, a new neighbor table entry is allocated and is added to the
 * neighbor table.
 */
static void
received_announcement(struct announcement *a, rimeaddr_t *from,
		      uint16_t id, uint16_t value)
{
  struct example_neighbor *e;

//   We received an announcement from a neighbor so we need to update
//     the neighbor list, or add a new entry to the table.
  for(e = list_head(neighbor_table); e != NULL; e = e->next) {
    if(rimeaddr_cmp(from, &e->addr)) {
//       Our neighbor was found, so we update the timeout.
      ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
      return;
    }
  }

//   The neighbor was not found in the list, so we add a new entry by
//     allocating memory from the neighbor_mem pool, fill in the
//     necessary fields, and add it to the list.
  e = memb_alloc(&neighbor_mem);
  if(e != NULL) {
    rimeaddr_copy(&e->addr, from);
    list_add(neighbor_table, e);
    ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
  }
}
#endif
#endif
