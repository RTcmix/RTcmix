// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "Frame.h"
#include <stdio.h>

//=============================================================================
PartialNode::PartialNode(void)
	: _id(0), _freq(0.0), _amp(0.0), _phase(0.0)
{
}

PartialNode::~PartialNode(void)
{
}

//=============================================================================
Frame::Frame(const int maxNumPartials)
	: _time(0.0), _maxNumNodes(maxNumPartials), _numNodes(maxNumPartials)
{
	_nodes = new PartialNode * [_maxNumNodes];
	for (int i = 0; i < _maxNumNodes; i++)
		_nodes[i] = new PartialNode();
}

Frame::~Frame(void)
{
	for (int i = 0; i < _maxNumNodes; i++)
		delete _nodes[i];
	delete [] _nodes;
}

PartialNode *Frame::partialFromID(const int id)
{
	for (int i = 0; i < _numNodes; i++) {
		PartialNode *node = _nodes[i];
		if (node->id() == id)
			return node;
	}
	return NULL;
}

void Frame::dump(void)
{
	printf("------------------------------------\n");
	printf("Printing frame at time %f... [id freq amp phase], %d partials\n", time(), numPartials());
	for (int i = 0; i < _numNodes; i++) {
		PartialNode *node = _nodes[i];
		printf("[%d] %f %f %f\n", node->id(), node->freq(), node->amp(), node->phase());
	}
}

