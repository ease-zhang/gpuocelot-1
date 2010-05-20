/*! \file ControlFlowGraph.cpp
	\author Andrew Kerr <arkerr@gatech.edu>
	\brief implementation for ControlFlowGraph
	\date 28 September 2008; 21 Jan 2009
*/

#include <ocelot/ir/interface/ControlFlowGraph.h>
#include <ocelot/ir/interface/Instruction.h>

#include <hydrazine/implementation/debug.h>

#include <iomanip>
#include <unordered_set>
#include <stack>
#include <queue>
#include <algorithm>

#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0

namespace ir {


/////////////////////////////////////////////////////////////////////////////////////////////////

ControlFlowGraph::BasicBlock::DotFormatter::DotFormatter() { }
ControlFlowGraph::BasicBlock::DotFormatter::~DotFormatter() { }

std::string ControlFlowGraph::make_label_dot_friendly( 
	const std::string& string ) {
	
	std::string result;
	for(std::string::const_iterator fi = string.begin(); 
		fi != string.end(); ++fi) {
		
		if( *fi == '{' ) {
			result.push_back('[');
		}
		else if( *fi == '}' ) {
			result.push_back(']');
		}
		else {
			result.push_back(*fi);
		}	
	}
	return result;
}

std::string ControlFlowGraph::BasicBlock::DotFormatter::dotFriendly(const std::string &str) {
	return ControlFlowGraph::make_label_dot_friendly(str);
}

std::string ControlFlowGraph::BasicBlock::DotFormatter::toString(const BasicBlock *block) {
	std::stringstream out;

	out << "[shape=record,";
	out << "label=";
	out << "\"{" << make_label_dot_friendly(block->label);

	BasicBlock::InstructionList::const_iterator instrs 
		= block->instructions.begin();	
	for (; instrs != block->instructions.end(); ++instrs) {
		out << " | " << make_label_dot_friendly((*instrs)->toString());
	}
	out << "}\"]";

	return out.str();
}

std::string ControlFlowGraph::BasicBlock::DotFormatter::toString(const Edge *edge) {
	std::stringstream out;

	if (edge->type == Edge::Dummy) {
		out << "[style=dotted]";
	}
	else if (edge->type == Edge::Branch) {
		out << "[color=blue]";
	}

	return out.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

ControlFlowGraph::Edge::Edge(BlockList::iterator h, 
	BlockList::iterator t, Type y) : head(h), tail(t), type(y) {

}

ControlFlowGraph::BasicBlock::BasicBlock(const std::string& l, Id i, 
	const InstructionList& is) : label(l), id(i), visited(false) {
	for (InstructionList::const_iterator instruction = is.begin();
		instruction != is.end(); ++instruction ) {
		instructions.push_back((*instruction)->clone(true));
	}
}

ControlFlowGraph::BasicBlock::~BasicBlock() {

}

ControlFlowGraph::BasicBlock::EdgeList::iterator 
	ControlFlowGraph::BasicBlock::get_fallthrough_edge() {
	for (EdgePointerVector::iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->type == Edge::FallThrough) return *edge;
	}
	assertM(false, "No fallthrough edge in block " << label);
}

ControlFlowGraph::BasicBlock::EdgeList::const_iterator 
	ControlFlowGraph::BasicBlock::get_fallthrough_edge() const {
	for (EdgePointerVector::const_iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->type == Edge::FallThrough) return *edge;
	}
	assertM(false, "No fallthrough edge in block " << label);
}

ControlFlowGraph::BasicBlock::EdgeList::iterator 
	ControlFlowGraph::BasicBlock::get_branch_edge() {
	for (EdgePointerVector::iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->type == Edge::Branch) return *edge;
	}
	assertM(false, "No branch edge in block " << label);
}

ControlFlowGraph::BasicBlock::EdgeList::const_iterator 
	ControlFlowGraph::BasicBlock::get_branch_edge() const {
	for (EdgePointerVector::const_iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->type == Edge::Branch) return *edge;
	}
	assertM(false, "No branch edge in block " << label);
}

ControlFlowGraph::BasicBlock::EdgeList::iterator 
	ControlFlowGraph::BasicBlock::get_edge(BlockList::iterator b) {
	for (EdgePointerVector::iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->tail == b) return *edge;
	}
	assertM(false, "No edge from " << label << " to " << b->label);
}

ControlFlowGraph::BasicBlock::EdgeList::const_iterator 
	ControlFlowGraph::BasicBlock::get_edge(BlockList::const_iterator b) const {
	for (EdgePointerVector::const_iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->tail == b) return *edge;
	}
	assertM(false, "No edge from " << label << " to " << b->label);
}

ControlFlowGraph::EdgePointerVector::iterator 
	ControlFlowGraph::BasicBlock::find_in_edge(
	BlockList::const_iterator head) {
	for (EdgePointerVector::iterator edge = in_edges.begin(); 
		edge != in_edges.end(); ++edge) {
		if ((*edge)->head == head) return edge;
	}
	return in_edges.end();
}

ControlFlowGraph::EdgePointerVector::iterator 
	ControlFlowGraph::BasicBlock::find_out_edge(
	BlockList::const_iterator tail) {
	for (EdgePointerVector::iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge) {
		if ((*edge)->tail == tail) return edge;
	}
	return out_edges.end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

ControlFlowGraph::ControlFlowGraph(): 
	_entry(_blocks.insert(end(), BasicBlock("entry"))),
	_exit(_blocks.insert(end(), BasicBlock("exit"))) {
}

ControlFlowGraph::~ControlFlowGraph() {
}

size_t ControlFlowGraph::size() const {
	return _blocks.size();
}

bool ControlFlowGraph::empty() const {
	return _blocks.empty();
}

ControlFlowGraph::iterator ControlFlowGraph::insert_block(
	const BasicBlock& block) {
	return _blocks.insert(end(), block);
}

void ControlFlowGraph::remove_block(iterator block) {
	EdgePointerVector in_edges = block->in_edges;
	for (edge_pointer_iterator eit = in_edges.begin(); 
		eit != in_edges.end(); ++eit) {
		remove_edge(*eit);
	}

	EdgePointerVector out_edges = block->out_edges;
	for (edge_pointer_iterator eit = out_edges.begin();
		eit != out_edges.end(); ++eit) {
		remove_edge(*eit);
	}
	
	_blocks.erase(block);
}

ControlFlowGraph::edge_iterator 
	ControlFlowGraph::insert_edge(const Edge& edge) {
	report( "Created " << toString(edge.type) << " edge from " 
		<< edge.head->label << " -> " << edge.tail->label );
	#ifndef NDEBUG
	if (edge.type == Edge::FallThrough) {
		// verify that tail is the tail of NO OTHER FallThrough edges
		edge_pointer_iterator eit = edge.tail->in_edges.begin();
		for (; eit != edge.tail->in_edges.end(); ++eit) {
			assertM((*eit)->type != Edge::FallThrough, 
				"Duplicate fallthrough edge added for block " 
				<< edge.tail->label );
		}
	}
	#endif
	edge_iterator e = _edges.insert(edges_end(), edge);
	edge.head->out_edges.push_back(e);
	edge.tail->in_edges.push_back(e);
	edge.head->successors.push_back(edge.tail);
	edge.tail->predecessors.push_back(edge.head);
	
	return e;
}

void ControlFlowGraph::remove_edge(edge_iterator edge) {
	report( "Removed edge from " << edge->head->label 
		<< " -> " << edge->tail->label );
	edge_pointer_iterator out = std::find(edge->head->out_edges.begin(), 
		edge->head->out_edges.end(), edge);
	assert(out != edge->head->out_edges.end());
	edge->head->out_edges.erase(out);

	edge_pointer_iterator in = std::find(edge->tail->in_edges.begin(), 
		edge->tail->in_edges.end(), edge);
	assert(in != edge->tail->in_edges.end());
	edge->tail->in_edges.erase(in);

	pointer_iterator head = std::find(edge->head->successors.begin(), 
		edge->head->successors.end(), edge->tail);
	assert(head != edge->head->successors.end());
	edge->head->successors.erase(head);

	pointer_iterator tail = std::find(edge->tail->predecessors.begin(), 
		edge->tail->predecessors.end(), edge->head);
	assert(tail != edge->tail->predecessors.end());
	edge->tail->predecessors.erase(tail);

	_edges.erase(edge);
}

ControlFlowGraph::edge_iterator ControlFlowGraph::split_edge(edge_iterator edge,
	const BasicBlock& newBlock) {
	iterator block = insert_block(newBlock);
	edge_iterator newEdge = insert_edge(Edge(block, edge->tail, edge->type));
	edge->tail = block;
	return newEdge;
}

ControlFlowGraph::iterator ControlFlowGraph::split_block(iterator block, 
	unsigned int instruction, Edge::Type type) {
	assert( instruction <= block->instructions.size() );
	report("Splitting block " << block->label 
		<< " at instruction " << instruction);
	iterator newBlock = insert_block(BasicBlock(block->label + "_split"));
	BasicBlock::InstructionList::iterator 
		begin = block->instructions.begin();
	std::advance(begin, instruction);
	BasicBlock::InstructionList::iterator end = block->instructions.end();
	
	newBlock->instructions.insert(newBlock->instructions.begin(), begin, end);
	block->instructions.erase(begin, end);

	EdgePointerVector out_edges = block->out_edges;

	for (EdgePointerVector::iterator edge = out_edges.begin(); 
		edge != out_edges.end(); ++edge ) {
		Edge e(newBlock, (*edge)->tail, (*edge)->type);
		edge_iterator erase = *edge;
		remove_edge( erase );
		insert_edge( e );
	}

	insert_edge(Edge(block, newBlock, type));

	return newBlock;
}

ControlFlowGraph::iterator ControlFlowGraph::get_entry_block() {
	return _entry;
}

ControlFlowGraph::iterator ControlFlowGraph::get_exit_block() {
	return _exit;
}

ControlFlowGraph::const_iterator ControlFlowGraph::get_entry_block() const {
	return _entry;
}

ControlFlowGraph::const_iterator ControlFlowGraph::get_exit_block() const {
	return _exit;
}

std::ostream& ControlFlowGraph::write(std::ostream &out) const { 
//	ControlFlowGraph::BasicBlockColorMap blockColors;
	BasicBlock::DotFormatter defaultFormatter;
	
	return write(out, defaultFormatter);
}

std::ostream& ControlFlowGraph::write(std::ostream &out, 
	ControlFlowGraph::BasicBlock::DotFormatter & blockFormatter) const {

	using namespace std;

	BlockMap blockIndices;

	out << "digraph {\n";

	// emit nodes

	out << "  // basic blocks\n\n";

	out << "  bb_0 [shape=Mdiamond,label=\"" << _entry->label << "\"];\n";
	out << "  bb_1 [shape=Msquare,label=\"" << _exit->label << "\"];\n";

	blockIndices[_entry] = 0;
	blockIndices[_exit] = 1;

	int n = 0;
	for (const_iterator block = begin(); block != end(); ++block, ++n) {
		if (block == _entry || block == _exit) continue;

		blockIndices[block] = n;

		const BasicBlock *blockPtr = &*block;
		out << "  bb_" << n << " " << blockFormatter.toString(blockPtr) << ";\n";

	}

	out << "\n\n  // edges\n\n";

	// emit edges
	for (const_edge_iterator edge = edges_begin(); 
		edge != edges_end(); ++edge) {
		const Edge *edgePtr = &*edge;

		out << "  " << "bb_" << blockIndices[edge->head] << " -> "
			<< "bb_" << blockIndices[edge->tail];
		out << " " << blockFormatter.toString(edgePtr);
	
		out << ";\n";
	}

	out << "}\n";

	return out;
}

std::string ControlFlowGraph::toString( Edge::Type t ) {
	switch( t )
	{
		case Edge::FallThrough: return "fallthrough"; break;
		case Edge::Branch: return "branch"; break;
		default: break;
	}
	return "Invalid";
}


void ControlFlowGraph::clear() {
	_blocks.clear();
	_edges.clear();
	_entry = insert_block(BasicBlock("entry"));
	_exit = insert_block(BasicBlock("exit"));
}


ControlFlowGraph::BlockPointerVector ControlFlowGraph::post_order_sequence() {
	typedef std::unordered_set<iterator> BlockSet;
	typedef std::stack<iterator> Stack;
	
	report("Creating post order traversal");
	BlockSet visited;
	BlockPointerVector sequence;
	Stack stack;
	
	if (!empty()) {
		report(" Adding block " << get_entry_block()->label);
		visited.insert(get_entry_block());
		for (pointer_iterator 
			block = get_entry_block()->successors.begin(); 
			block != get_entry_block()->successors.end(); ++block) {
			if (visited.insert(*block).second) {
				stack.push(*block);
			}
		}
		sequence.push_back(get_entry_block());
	}
	
	while (!stack.empty()) {
		iterator current = stack.top();

		bool one = false;
		for (pointer_iterator block = current->successors.begin(); 
			block != current->successors.end(); ++block) {
			if (visited.insert(*block).second) {
				stack.push(*block);
				one = true;
			}
		}
		
		if(!one) {
			stack.pop();
			sequence.push_back(current);
			report(" Adding block " << current->label);
		}
	}
	
	return std::move(sequence);
}

ControlFlowGraph::BlockPointerVector ControlFlowGraph::pre_order_sequence() {
	typedef std::unordered_set<iterator> BlockSet;
	typedef std::stack<iterator> Stack;
	
	BlockSet visited;
	BlockPointerVector sequence;
	Stack stack;
	
	if (!empty()) {
		stack.push(get_entry_block());
		visited.insert(get_entry_block());
	}
	
	while (!stack.empty()) {
		iterator current = stack.top();
		stack.pop();
		
		sequence.push_back(current);
		for (pointer_iterator block = current->successors.begin(); 
			block != current->successors.end(); ++block) {
			if (visited.insert(*block).second) {
				stack.push(*block);
			}
		}
	}
	
	return std::move(sequence);
}

ControlFlowGraph::BlockPointerVector ControlFlowGraph::executable_sequence() {
	typedef std::list<iterator> BlockPointerList;
	BlockPointerVector _blocks = post_order_sequence();
	BlockPointerList blocks(_blocks.begin(), _blocks.end());
	BlockPointerVector kill;
	BlockPointerVector sequence;

	report("Getting executable sequence.");

	while (blocks.size()) {
		iterator target = blocks.front();
		kill.clear();
		
		for (BlockPointerList::iterator block = blocks.begin(); 
			block != blocks.end() && target != end(); ++block) {
			if ((*block)==target) {
				kill.push_back(*block);
				sequence.push_back(*block);
				report(" " << (*block)->label);
				target = end();
			
				for (edge_pointer_iterator edge = (*block)->out_edges.begin(); 
					edge != (*block)->out_edges.end(); ++edge) {
					if ((*edge)->type == Edge::FallThrough) {
						target = (*edge)->tail;
						block = blocks.begin();
						break;
					}
				}
			}
		}
		
		for (pointer_iterator killed = kill.begin(); 
			killed != kill.end(); ++killed) {
			blocks.remove(*killed);
		}
	}

	return std::move(sequence);
}

ControlFlowGraph & ControlFlowGraph::operator=(const 
	ControlFlowGraph &cfg) {
	report( "Copying cfg" );
	typedef std::unordered_map<const_iterator, iterator> BlockMap;
	BlockMap block_map;
	
	clear();
	
	_entry->id = cfg._entry->id;
	_exit->id = cfg._exit->id;
	
	for (const_iterator bl_it = cfg.begin(); bl_it != cfg.end(); ++bl_it) {
		if (cfg._entry == bl_it) {
			block_map[bl_it] = _entry;
		}
		else if(cfg._exit == bl_it) {
			block_map[bl_it] = _exit;
		}
		else {
			iterator newBlock = insert_block(
				BasicBlock(bl_it->label, bl_it->id, bl_it->instructions));
			block_map[bl_it] = newBlock;
		}
	}

	// duplicate edges using the block_map
	for (const_edge_iterator e_it = cfg.edges_begin(); 
		e_it != cfg.edges_end(); ++e_it) {
		assert( block_map.count( e_it->head ) );
		assert( block_map.count( e_it->tail ) );
		insert_edge(Edge(block_map[e_it->head], 
			block_map[e_it->tail], e_it->type));
	}
	
	return *this;
}

ControlFlowGraph::iterator ControlFlowGraph::begin() {
	return _blocks.begin();
}

ControlFlowGraph::iterator ControlFlowGraph::end() {
	return _blocks.end();
}

ControlFlowGraph::const_iterator ControlFlowGraph::begin() const {
	return _blocks.begin();
}

ControlFlowGraph::const_iterator ControlFlowGraph::end() const {
	return _blocks.end();
}

ControlFlowGraph::edge_iterator ControlFlowGraph::edges_begin() {
	return _edges.begin();
}

ControlFlowGraph::edge_iterator ControlFlowGraph::edges_end() {
	return _edges.end();
}

ControlFlowGraph::const_edge_iterator ControlFlowGraph::edges_begin() const {
	return _edges.begin();
}

ControlFlowGraph::const_edge_iterator ControlFlowGraph::edges_end() const {
	return _edges.end();
}

}

