/*========================================================================
 * ring.c  --  Ring buffer control routines for Tsunami client.
 *
 * This contains routines for managing the ring buffer that contacts
 * the filesystem thread and the network thread during a Tsunami
 * transfer.
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
 * OR OTHER PROPRIETARY RIGHTS. INDIANA UNIVERSITY MAKES NO
 * WARRANTIES THAT SOFTWARE IS FREE FROM "BUGS", "VIRUSES", "TROJAN
 * HORSES", "TRAP DOORS", "WORMS", OR OTHER HARMFUL CODE.  LICENSEE
 * ASSUMES THE ENTIRE RISK AS TO THE PERFORMANCE OF SOFTWARE AND/OR
 * ASSOCIATED MATERIALS, AND TO THE PERFORMANCE AND VALIDITY OF
 * INFORMATION GENERATED USING SOFTWARE.
 *========================================================================*/

#include <pthread.h>  /* for the pthreads library     */
#include <stdlib.h>   /* for malloc(), free(), etc.   */
#include <string.h>   /* for string-handling routines */

#include <tsunami-client.h>


/*------------------------------------------------------------------------
 * Module-scope constants.
 *------------------------------------------------------------------------*/

const int EMPTY = -1;


/*------------------------------------------------------------------------
 * int ring_full(ring_buffer *ring);
 *
 * Returns non-zero if ring is full.
 *------------------------------------------------------------------------*/
int ring_full(ring_buffer_t *ring)
{
    int status, full;

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0)
        error("Could not get access to ring buffer mutex");

    full = !ring->space_ready;

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0)
        error("Could not relinquish access to ring buffer mutex");

    /* we succeeded */
    return full;
}

/*------------------------------------------------------------------------
 * int ring_cancel(ring_buffer *ring);
 *
 * Cancels the reservation for the slot that was most recently reserved.
 * Returns 0 on success and nonzero on error.
 *------------------------------------------------------------------------*/
int ring_cancel(ring_buffer_t *ring)
{
    int status;

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0)
	error("Could not get access to ring buffer mutex");

    /* convert the reserved slot into space */
    if (--(ring->count_reserved) < 0)
	error("Attempt made to cancel unreserved slot in ring buffer");

    /* signal that space is available */
    ring->space_ready = 1;
    status = pthread_cond_signal(&ring->space_ready_cond);
    if (status != 0)
	error("Could not signal space-ready condition");

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0)
	error("Could not relinquish access to ring buffer mutex");

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int ring_confirm(ring_buffer *ring);
 *
 * Confirms that data is now available in the slot that was most
 * recently reserved.  This data will be handled by the disk thread.
 * Returns 0 on success and nonzero on error.
 *------------------------------------------------------------------------*/
int ring_confirm(ring_buffer_t *ring)
{
    int status;

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0)
	error("Could not get access to ring buffer mutex");

    /* convert the reserved slot into data */
    ++(ring->count_data);
    if (--(ring->count_reserved) < 0)
	error("Attempt made to confirm unreserved slot in ring buffer");

    /* signal that data is available */
    ring->data_ready = 1;
    status = pthread_cond_signal(&ring->data_ready_cond);
    if (status != 0)
	error("Could not signal data-ready condition");

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0)
	error("Could not relinquish access to ring buffer mutex");

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * ring_buffer_t *ring_create(ttp_session_t *session);
 *
 * Creates the ring buffer data structure for a Tsunami transfer and
 * returns a pointer to the new data structure.  Returns NULL if
 * allocation and initialization failed.  The new ring buffer will hold
 * ([6 + block_size] * MAX_BLOCKS_QUEUED datagrams.
 *------------------------------------------------------------------------*/
ring_buffer_t *ring_create(ttp_session_t *session)
{
    ring_buffer_t *ring;
    int            status;

    /* try to allocate the structure */
    ring = (ring_buffer_t *) calloc(1, sizeof(*ring));
    if (ring == NULL)
	error("Could not allocate ring buffer object");

    /* try to allocate the buffer */
    ring->datagram_size = 6 + session->parameter->block_size;
    ring->datagrams = (u_char *) malloc(ring->datagram_size * MAX_BLOCKS_QUEUED);
    if (ring->datagrams == NULL)
	error("Could not allocate buffer for ring buffer");

    /* create the mutex */
    status = pthread_mutex_init(&ring->mutex, NULL);
    if (status != 0)
	error("Could not create mutex for ring buffer");

    /* create the data-ready condition variable */
    status = pthread_cond_init(&ring->data_ready_cond, NULL);
    if (status != 0)
	error("Could not create data-ready condition variable");
    ring->data_ready = 0;

    /* create the space-ready condition variable */
    status = pthread_cond_init(&ring->space_ready_cond, NULL);
    if (status != 0)
	error("Could not create space-ready condition variable");
    ring->space_ready = 1;

    /* initialize the indices */
    ring->count_data     = 0;
    ring->count_reserved = 0;
    ring->base_data      = 0;

    /* and return the ring structure */
    return ring;
}


/*------------------------------------------------------------------------
 * int ring_destroy(ring_buffer_t *ring);
 *
 * Destroys the ring buffer data structure for a Tsunami transfer,
 * including the mutex and condition variables.  Returns 0 on success
 * and nonzero on failure.
 *------------------------------------------------------------------------*/
int ring_destroy(ring_buffer_t *ring)
{
    int status;

    /* destroy the mutex */
    status = pthread_mutex_destroy(&ring->mutex);
    if (status != 0)
	return warn("Could not destroy mutex for ring buffer");

    /* destroy the condition variables */
    status = pthread_cond_destroy(&ring->data_ready_cond);
    if (status != 0)
	return warn("Could not destroy data-ready condition variable");
    status = pthread_cond_destroy(&ring->space_ready_cond);
    if (status != 0)
	return warn("Could not destroy space-ready condition variable");

    /* free the memory used */
    free(ring->datagrams);
    free(ring);

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int ring_dump(ring_buffer_t *ring, FILE *out);
 *
 * Dumps the current contents of the ring buffer to the given output
 * stream.  Returns zero on success and non-zero on error.
 *------------------------------------------------------------------------*/
int ring_dump(ring_buffer_t *ring, FILE *out)
{
    int     status;
    int     index;
    u_char *datagram;

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0)
	return warn("Could not get access to ring buffer mutex");

    /* print out the top-level fields */
    fprintf(out, "datagram_size  = %d\n", ring->datagram_size);
    fprintf(out, "base_data      = %d\n", ring->base_data);
    fprintf(out, "count_data     = %d\n", ring->count_data);
    fprintf(out, "count_reserved = %d\n", ring->count_reserved);
    fprintf(out, "data_ready     = %d\n", ring->data_ready);
    fprintf(out, "space_ready    = %d\n", ring->space_ready);

    /* print out the block list */
    fprintf(out, "block list     = [");
    for (index = ring->base_data; index < ring->base_data + ring->count_data; ++index) {
	datagram = ring->datagrams + ((index % MAX_BLOCKS_QUEUED) * ring->datagram_size);
	fprintf(out, "%d ", ntohl(*((u_int32_t *) datagram)));
    }
    fprintf(out, "]\n");

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0)
	return warn("Could not relinquish access to ring buffer mutex");

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * u_char *ring_peek(ring_buffer_t *ring);
 *
 * Attempts to return a pointer to the datagram at the head of the ring.
 * This will block if the ring is currently empty.  Returns NULL on error.
 *------------------------------------------------------------------------*/
u_char *ring_peek(ring_buffer_t *ring)
{
    int     status;
    u_char *address;

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0) {
	warn("Could not get access to ring buffer mutex");
	return NULL;
    }

    /* wait for the data-ready variable to make us happy */
    while (ring->data_ready == 0) {
	status = pthread_cond_wait(&ring->data_ready_cond, &ring->mutex);
	if (status != 0) {
	    warn("Could not wait for ring buffer to accumulate data");
	    return NULL;
	}
    }

    /* find the address we want */
    address = ring->datagrams + (ring->datagram_size * ring->base_data);

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0) {
	warn("Could not relinquish access to ring buffer mutex");
	return NULL;
    }

    /* return the datagram */
    return address;
}


/*------------------------------------------------------------------------
 * int ring_pop(ring_buffer_t *ring);
 *
 * Attempts to remove a datagram from the head of the ring.  This will
 * block if the ring is currently empty.  Returns 0 on success and
 * nonzero on error.
 *------------------------------------------------------------------------*/
int ring_pop(ring_buffer_t *ring)
{
    int status;

    //printf("Popping slot %d\n", ring->base_data);

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0)
	error("Could not get access to ring buffer mutex");

    /* wait for the data-ready variable to make us happy */
    while (ring->data_ready == 0) {
	status = pthread_cond_wait(&ring->data_ready_cond, &ring->mutex);
	if (status != 0)
    	    error("Could not wait for ring buffer to accumulate data");
    }

    /* perform the pop operation */
    ring->base_data = (ring->base_data + 1) % MAX_BLOCKS_QUEUED;
    if (--(ring->count_data) == 0)
	ring->data_ready = 0;

    /* signal that space is available */
    ring->space_ready = 1;
    status = pthread_cond_signal(&ring->space_ready_cond);
    if (status != 0)
	error("Could not signal space-ready condition");

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0)
	error("Could not relinquish access to ring buffer mutex");

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * u_char *ring_reserve(ring_buffer_t *ring);
 *
 * Reserves a slot in the ring buffer for the next datagram.  A pointer
 * to the memory that should be used to store the datagram is returned.
 * This will block if no space is available in the ring buffer.  Returns
 * NULL on error.
 *------------------------------------------------------------------------*/
u_char *ring_reserve(ring_buffer_t *ring)
{
    int     status;
    int     next;
    u_char *address;

    /* get a lock on the ring buffer */
    status = pthread_mutex_lock(&ring->mutex);
    if (status != 0)
	error("Could not get access to ring buffer mutex");

    /* figure out which slot comes next */
    next = (ring->base_data + ring->count_data + ring->count_reserved) % MAX_BLOCKS_QUEUED;

    /* wait for the space-ready variable to make us happy */
    while (ring->space_ready == 0) {
	printf("FULL! -- ring_reserve() blocking.\n");
	printf("space_ready = %d, data_ready = %d\n", ring->space_ready, ring->data_ready);
	status = pthread_cond_wait(&ring->space_ready_cond, &ring->mutex);
	if (status != 0)
	    error("Could not wait for ring buffer to clear space");
    }

    /* perform the reservation */
    if (++(ring->count_reserved) > 1)
	error("Attempt made to reserve two slots in ring buffer");
    if (((next + 1) % MAX_BLOCKS_QUEUED) == ring->base_data)
	ring->space_ready = 0;

    /* find the address we want */
    address = ring->datagrams + (next * ring->datagram_size);

    /* release the mutex */
    status = pthread_mutex_unlock(&ring->mutex);
    if (status != 0)
	error("Could not relinquish access to ring buffer mutex");

    /* return the address */
    return address;
}


/*========================================================================
 * $Log: ring.c,v $
 * Revision 1.3  2009/12/21 17:46:33  jwagnerhki
 * added mutexed ring_full
 *
 * Revision 1.2  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:26:51  jwagnerhki
 * deleted unnecessary files
 *
 */
