
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _lw_streamgraph_h
#define _lw_streamgraph_h

typedef struct _lwp_streamgraph_link
{
   lw_stream to, to_exp;
   lw_stream from, from_exp;

   size_t bytes_left; /* ctor: = -1 */

   lw_bool delete_stream;

} * lwp_streamgraph_link;

typedef struct _lwp_streamgraph
{
   lwp_refcounted;
   lw_bool dead;

   /* Each StreamGraph actually stores two graphs - one public without the
    * filters, and an internally expanded version with the filters included.
    */

   list (lw_stream, roots);
   list (lw_stream, roots_expanded);

   int last_expand;

} * lwp_streamgraph;

lwp_streamgraph lwp_streamgraph_new ();
void lwp_streamgraph_delete (lwp_streamgraph);


/* Merge another graph into this one, and then delete it.  This is used, for
 * example, when writing one stream to another.
 */
 void lwp_streamgraph_swallow (lwp_streamgraph graph, lwp_streamgraph old_graph);


/* Clear the expanded graph.  This is usually done before modifying the
 * non-expanded version.
 */
 void lwp_streamgraph_clear_expanded (lwp_streamgraph);


/* Generate the expanded graph.  This is usually done after modifying the
 * non-expanded version.
 */
 void lwp_streamgraph_expand (lwp_streamgraph);


/* Scan through the graph and issue a read wherever a link needs one.
 * Depending on how the graph was modified, this may or may not be necessary
 * after expansion.
 */
 void lwp_streamgraph_read (lwp_streamgraph);


 /* Print the graph to stderr (for debugging)
  */
 void lwp_streamgraph_print (lwp_streamgraph);

#endif


