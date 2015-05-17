// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#ifndef _FRAME_H_
#define _FRAME_H_

class PartialNode {
public:
	PartialNode(void);
	~PartialNode(void);
	void set(int id, float freq, float amp, float phase) {
		_id = id;
		_freq = freq;
		_amp = amp;
		_phase = phase;
	}
	int id(void) const { return _id; }
	float freq(void) const { return _freq; }
	float amp(void) const { return _amp; }
	float phase(void) const { return _phase; }

private:
	int		_id;	// partial id tag
	float		_freq;
	float		_amp;
	float		_phase;
#ifdef SUPPORT_BWE
	float		_noise;
#endif
};

#include <stdlib.h>

inline int _compareFreq(const void *a, const void *b)
{
	PartialNode **p1 = (PartialNode **) a;
	PartialNode **p2 = (PartialNode **) b;
	const float freq1 = (*p1)->freq();
	const float freq2 = (*p2)->freq();
	if (freq1 > freq2)
		return 1;
	if (freq1 < freq2)
		return -1;
	return 0;
}

class Frame {
public:
	Frame(const int maxNumPartials);
	~Frame(void);
	void time(const double val) { _time = val; }
	double time(void) const { return _time; }

	// This can be less than maxNumPartials, if not all _nodes are
	// currently being used.
	void numPartials(const int numPartials) { _numNodes = numPartials; }
	int numPartials(void) const { return _numNodes; }

	PartialNode **partials(void) { return _nodes; }

	// Return the partial node with matching ID, or NULL.
	PartialNode *partialFromID(const int id);

	// Sort the partial nodes in this frame in ascending order by partial
	// frequency.
	void sortPartials(void)
	{
		qsort(_nodes, _numNodes, sizeof(PartialNode *), _compareFreq);
	}

	void dump(void);

private:
	double		_time;		// actual time of frame in SDIF file
	int			_maxNumNodes, _numNodes;
	PartialNode	**_nodes;	// array of PartialNode objects
};

#endif // _FRAME_H_
