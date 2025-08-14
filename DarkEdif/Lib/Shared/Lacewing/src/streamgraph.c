/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"
#include "streamgraph.h"
#include "stream.h"

static void graph_dealloc (lwp_streamgraph graph)
{
	lwp_trace ("graph at %p was dealloc'd\n", graph);
	lwp_streamgraph_clear_expanded (graph);

	list_clear (graph->roots);

	free (graph);
}

lwp_streamgraph lwp_streamgraph_new ()
{
	lwp_streamgraph graph = (lwp_streamgraph) calloc (sizeof (*graph), 1);

	if (!graph)
		return 0;

	lwp_set_dealloc_proc (graph, graph_dealloc);
	lwp_enable_refcount_logging (graph, "streamgraph");

	lwp_retain (graph, "streamgraph_new");

	return graph;
}

void lwp_streamgraph_delete (lwp_streamgraph graph)
{
	if (!graph)
		return;

	graph->dead = lw_true;
	lwp_release (graph, "streamgraph_new");
}

static void swallow (lwp_streamgraph graph, lw_stream stream)
{
	assert (graph);
	assert (stream->graph);

	if (stream->graph == graph)
		return;

	stream->graph = graph;
	stream->last_expand = graph->last_expand;

	list_each (lwp_stream_filterspec, stream->filters_upstream, spec)
	{
		swallow (graph, spec->filter);
	}

	list_each (lwp_stream_filterspec, stream->filters_downstream, spec)
	{
		swallow (graph, spec->filter);
	}

	list_each (lwp_stream_filterspec, stream->filtering, spec)
	{
		swallow (graph, spec->stream);
	}

	list_each (lwp_streamgraph_link, stream->next, link)
	{
		swallow (graph, link->to);
	}
}

void lwp_streamgraph_swallow (lwp_streamgraph graph, lwp_streamgraph old_graph)
{
	assert (old_graph != graph);

	lwp_streamgraph_clear_expanded (old_graph);

	list_each (lw_stream, old_graph->roots, root)
	{
		list_push (lw_stream, graph->roots, root);
		swallow (graph, root);
	}

	lwp_streamgraph_delete (old_graph);
}

static void expand_stream (lwp_streamgraph graph, lw_stream stream,
							lw_stream * first, lw_stream * last)
{
	assert (stream); // this is mostly to prevent an analysis complaint

	lw_stream expanded, next;

	*first = *last = 0;

	/* Upstream filters come first */

	list_each (lwp_stream_filterspec, stream->filters_upstream, spec)
	{
		lw_stream filter_stream = spec->filter, expanded, next;

		stream->last_expand = graph->last_expand;

		expand_stream (graph, filter_stream, &expanded, &next);

		if (!*first)
			*first = expanded;
		else
		{
			spec->link.from_exp = *last;
			spec->link.to_exp = expanded;

			list_push (lwp_streamgraph_link, (*last)->next_expanded, &spec->link);
			list_push (lwp_streamgraph_link, expanded->prev_expanded, &spec->link);
		}

		*last = next;
	}

	stream->head_upstream = *first;


	/* Now the stream itself */

	stream->last_expand = graph->last_expand;

	if (*last)
	{
		lwp_streamgraph_link link =
			(lwp_streamgraph_link) lw_calloc_or_exit (sizeof (*link), 1);
		link->from_exp = *last;
		link->to_exp = stream;
		link->bytes_left = SIZE_MAX;

		list_push (lwp_streamgraph_link, (*last)->next_expanded, link);
		list_push (lwp_streamgraph_link, stream->prev_expanded, link);

		*last = stream;
	}
	else
	{
		*first = *last = stream;
	}


	/* And downstream filters afterwards */

	list_each (lwp_stream_filterspec, stream->filters_downstream, spec)
	{
		lw_stream filter_stream = spec->filter;

		filter_stream->last_expand = graph->last_expand;

		expand_stream (graph, filter_stream, &expanded, &next);

		if (*last)
		{
			spec->link.from_exp = *last;
			spec->link.to_exp = expanded;

			list_push (lwp_streamgraph_link, (*last)->next_expanded, &spec->link);
			list_push (lwp_streamgraph_link, expanded->prev_expanded, &spec->link);
		}

		*last = next;
	}


	/* The last downstream filter is in charge of calling any data hooks */

	list_each (struct _lwp_stream_data_hook, stream->data_hooks, hook)
	{
		list_push (struct _lwp_stream_data_hook, (*last)->exp_data_hooks, hook);
	}
}

static void expand (lwp_streamgraph graph, lw_stream last,
					lwp_streamgraph_link last_link,
					lwp_streamgraph_link link, lw_stream stream)
{
	lw_stream expanded_first, expanded_last;

	expand_stream (graph, stream, &expanded_first, &expanded_last);

	if (last)
	{
		link->to_exp = expanded_first;

		list_push (lwp_streamgraph_link, expanded_first->prev_expanded, link);

		link->from_exp = last;

		#ifndef NDEBUG
			list_find(lwp_streamgraph_link, last->next_expanded, link);
			assert(elem == NULL);
		#endif

		list_push (lwp_streamgraph_link, last->next_expanded, link);
	}
	else
	{
		list_push (lw_stream, graph->roots_expanded, expanded_first);
	}

	last = expanded_last;
	last_link = link;

	list_each (lwp_streamgraph_link, stream->next, link)
	{
		expand (graph, last, last_link, link, link->to);
	}
}

void lwp_streamgraph_expand (lwp_streamgraph graph)
{
	lwp_streamgraph_clear_expanded (graph);

	list_each (lw_stream, graph->roots, stream)
	{
		if (stream->last_expand == graph->last_expand)
			continue;

		expand (graph, 0, 0, 0, stream);
	}

	list_each_elem (lw_stream, graph->roots_expanded, elem)
	{
		lw_stream stream = *elem;

		/* A root only needs to remain a root if it doesn't appear
		* elsewhere in the graph.
		*/

		if (list_length (stream->prev_expanded) > 0)
			list_elem_remove (elem);
	}

	#ifdef _lacewing_debug
		lwp_streamgraph_print (graph);
	#endif
}

static lw_bool find_next_direct (lw_stream stream, lw_stream * next_direct,
								 size_t * bytes)
{
	list_each (lwp_streamgraph_link, stream->next_expanded, link)
	{
		if (link->bytes_left != SIZE_MAX && link->bytes_left < *bytes)
			*bytes = link->bytes_left;

		lw_stream next = link->to_exp;

		if (lwp_stream_is_transparent (next))
		{
			if (!find_next_direct (next, next_direct, bytes))
				return lw_false;
		}

		if (next)
		{
			if (*next_direct)
				return lw_false;

			*next_direct = next;
		}
	}

	return lw_true;
}

static void graph_read (lwp_streamgraph graph, int this_expand,
						lw_stream stream, size_t bytes)
{
	/* TODO : Currently, the presence of a filter forces a read */

	if (!list_length (stream->next_expanded))
		return;

	if (bytes == SIZE_MAX)
	{
		/* Use the biggest bytes_left from all the links */

		list_each (lwp_streamgraph_link, stream->next, link)
		{
			if (link->bytes_left == SIZE_MAX)
			{
				bytes = SIZE_MAX;
				break;
			}

			if (link->bytes_left > bytes)
				bytes = link->bytes_left;
		}
	}

	lwp_trace ("Reading " lwp_fmt_size " from %p", bytes, stream);

	lw_bool wrote_direct = lw_false;

	do
	{
		if (list_length (stream->exp_data_hooks) > 0)
			break;

		lw_stream next = 0;

		size_t direct_bytes = bytes;

		/* TODO : Skip to next direct instead of trying to read direct from
		* intermediate streams?
		*/

		if (!find_next_direct (stream, &next, &direct_bytes))
			break;

		lwp_trace ("Next direct from %p -> %p", stream, next);

		if ( (!next) || next->prev_direct)
			break;

		/* Only one non-transparent stream follows.  It may be possible to
		* shift the data directly (without having to read it first).
		*
		* Note that we don't have to check next's queue is empty, because
		* we're already being written (the queue is behind us).
		*/

		next->prev_direct = stream;
		next->direct_bytes_left = direct_bytes;

		lwp_trace ("Attempting to read direct for %p -> %p", stream, next);

		if (lwp_stream_write_direct (next))
		{
			lwp_trace ("write_direct succeeded for %p -> %p", stream, next);

			wrote_direct = lw_true;
			break;
		}
		else
		{
			lwp_trace ("write_direct failed for %p -> %p", stream, next);

			next->prev_direct = 0;
		}

	} while (0);

	if (!wrote_direct)
		lw_stream_read (stream, bytes);

	/* Calling Push or Read may well run user code, or expire a link.  If this
	* causes the graph to re-expand, Read() will be called again and this one
	* must abort.
	*/

	if (this_expand != graph->last_expand || graph->dead)
	{
		lwp_trace ("last_expand %d changed to %d after read; aborting",
			this_expand, graph->last_expand);

		return;
	}

	list_each (lwp_streamgraph_link, stream->next_expanded, link)
	{
		graph_read (graph, this_expand, link->to_exp, link->bytes_left);

		if (this_expand != graph->last_expand || graph->dead)
			return;
	}
}

void lwp_streamgraph_read (lwp_streamgraph graph)
{
	lwp_retain (graph, "streamgraph_read");

	int this_expand = graph->last_expand;

	lwp_trace ("streamgraph_read with last_expand %d", this_expand);

	list_each (lw_stream, graph->roots_expanded, root)
	{
		graph_read (graph, this_expand, root, SIZE_MAX);

		if (graph->last_expand != this_expand || graph->dead)
		{
			lwp_trace ("Abort streamgraph_read with last_expand %d", this_expand);
			break;
		}
	}

	lwp_release (graph, "streamgraph_read");
}

static void clear_expanded (lw_stream stream)
{
	list_each (lwp_streamgraph_link, stream->next_expanded, link)
	{
		assert (link->from_exp == stream);

		if (link->to_exp)
			clear_expanded (link->to_exp);

		link->to_exp = link->from_exp = 0;
	}

	list_clear (stream->prev_expanded);
	list_clear (stream->next_expanded);

	stream->prev_direct = 0;

	list_clear (stream->exp_data_hooks);
}

void lwp_streamgraph_clear_expanded (lwp_streamgraph graph)
{
	++ graph->last_expand;

	list_each (lw_stream, graph->roots_expanded, root)
	{
		clear_expanded (root);
	}

	list_clear (graph->roots_expanded);
}


/* debugging stuff */

static void print (lwp_streamgraph graph, lw_stream stream, int depth)
{
	assert (stream->graph == graph);

	fprintf (stderr, "stream @ %p (" lwp_fmt_size " bytes, %d hooks, filtered %d up/%d down, filters %d)\n",
		stream, lw_stream_bytes_left (stream),
		(int) list_length (stream->data_hooks),
		(int) list_length (stream->filters_upstream),
		(int) list_length (stream->filters_downstream),
		(int) list_length (stream->filtering));

	list_each (lwp_streamgraph_link, stream->next, link)
	{
		for (int i = 0; i < depth; ++ i)
			fprintf (stderr, "  ");

		fprintf (stderr, "link (" lwp_fmt_size ") %p ==> ",
			link->bytes_left, link);

		print (graph, link->to, depth + 1);
	}
}

static void print_expanded (lwp_streamgraph graph, lw_stream stream, int depth)
{
	assert (stream->graph == graph);

	fprintf (stderr, "stream @ %p (" lwp_fmt_size " bytes, %d hooks)\n",
		stream, lw_stream_bytes_left (stream),
		(int) list_length (stream->exp_data_hooks));

	list_each (lwp_streamgraph_link, stream->next_expanded, link)
	{
		for (int i = 0; i < depth; ++ i)
			fprintf (stderr, "  ");

		fprintf (stderr, "link (" lwp_fmt_size ") %p ==> ",
			link->bytes_left, link);

		assert (link->from_exp == stream);

		print_expanded (graph, link->to_exp, depth + 1);
	}
}

void lwp_streamgraph_print (lwp_streamgraph graph)
{
	fprintf (stderr, "\n--- Graph %p (%d) ---\n\n",
		graph, (int) list_length (graph->roots));

	if (list_length (graph->roots) > 0)
	{
		assert (list_front (lw_stream, graph->roots));
		assert (list_back (lw_stream, graph->roots));
	}

	list_each (lw_stream, graph->roots, root)
	{
		print (graph, root, 1);
	}

	fprintf (stderr, "\n--- Graph %p expanded (%d) ---\n\n",
		graph, (int) list_length (graph->roots_expanded));

	if (list_length (graph->roots_expanded) > 0)
	{
		assert (list_front (lw_stream, graph->roots_expanded));
		assert (list_back (lw_stream, graph->roots_expanded));
	}

	list_each (lw_stream, graph->roots_expanded, root)
	{
		print_expanded (graph, root, 1);
	}

	fprintf (stderr, "\n");
}
