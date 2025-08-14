/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
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

	lw_list (lw_stream, roots);
	lw_list (lw_stream, roots_expanded);

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


