/*
 * one-many_contigs_enlarger.hpp
 *
 *  Created on: Aug 1, 2011
 *      Author: undead
 */

#ifndef ONE_MANY_CONTIGS_ENLARGER_HPP_
#define ONE_MANY_CONTIGS_ENLARGER_HPP_
using namespace omnigraph;

template <class Graph>
class one_many_contigs_enlarger {
	typedef typename Graph::EdgeId EdgeId;
	typedef typename Graph::VertexId VertexId;

	Graph &g_;
public:
	one_many_contigs_enlarger(Graph &g): g_(g){
	}
	void one_many_resolve(){
		INFO("one_many_resolve");
		int inc_count;
		for (auto iter = g_.SmartVertexBegin(); ! iter.IsEnd(); ++iter) {
			DEBUG(*iter);
			if ((g_.OutgoingEdgeCount(*iter) == 1) && ((inc_count = g_.IncomingEdgeCount(*iter)) > 1)){
				EdgeId unique = g_.GetUniqueOutgoingEdge(*iter);
				vector<EdgeId> incEdges = g_.IncomingEdges(*iter);
				for(int j = 0; j < inc_count; j++) {
					VertexId tmp_v = g_.AddVertex();

					EdgeId edge2 = g_.AddEdge(tmp_v, g_.EdgeEnd(unique), g_.EdgeNucls(unique));
					EdgeId edge1 = g_.AddEdge(g_.EdgeStart(incEdges[j]), tmp_v, g_.EdgeNucls(incEdges[j]));
					vector<EdgeId> toMerge;
					toMerge.push_back(edge1);
					toMerge.push_back(edge2);
					g_.MergePath(toMerge);
				}
				g_.ForceDeleteVertex(*iter);
			}
		}


		for (auto iter = g_.SmartVertexBegin(); ! iter.IsEnd(); ++iter){
			if ((g_.OutgoingEdgeCount(*iter) > 1) && ((inc_count = g_.IncomingEdgeCount(*iter)) == 1)){
				EdgeId unique = g_.GetUniqueIncomingEdge(*iter);
				vector<EdgeId> incEdges = g_.OutgoingEdges(*iter);
				for(int j = 0; j < inc_count; j++) {
					VertexId tmp_v = g_.AddVertex();

					EdgeId edge2 = g_.AddEdge(g_.EdgeStart(unique), tmp_v, g_.EdgeNucls(unique));
					;
					EdgeId edge1 = g_.AddEdge(tmp_v, g_.EdgeEnd(incEdges[j]), g_.EdgeNucls(incEdges[j]));
					vector<EdgeId> toMerge ;
					toMerge.push_back(edge2);
					toMerge.push_back(edge1);

					g_.MergePath(toMerge);
				}
				g_.ForceDeleteVertex(*iter);
			}
		}



	}
};

#endif /* ONE_MANY_CONTIGS_ENLARGER_HPP_ */
