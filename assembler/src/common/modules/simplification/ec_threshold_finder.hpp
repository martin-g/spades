//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#ifndef OMNI_TOOLS_HPP_
#define OMNI_TOOLS_HPP_

#include "assembly_graph/graph_support/basic_edge_conditions.hpp"
#include "assembly_graph/graph_support/basic_vertex_conditions.hpp"
#include "assembly_graph/core/basic_graph_stats.hpp"

#include "utils/stl_utils.hpp"


#ifdef USE_GLIBCXX_PARALLEL
#include "parallel/algorithm"
#endif

namespace omnigraph {

template<class Graph>
class ErroneousConnectionThresholdFinder {
private:
    typedef typename Graph::VertexId VertexId;
    typedef typename Graph::EdgeId EdgeId;
    const Graph &graph_;
    size_t backet_width_;
    typedef std::map<size_t, size_t> Histogram;

    bool IsInteresting(EdgeId e) const {
        if (graph_.length(e) > graph_.k() + 1)
            return false;

        if (graph_.OutgoingEdgeCount(graph_.EdgeStart(e)) < 2 ||
            graph_.IncomingEdgeCount(graph_.EdgeEnd(e)) < 2)
            return false;

        std::vector<EdgeId> v1;
        utils::push_back_all(v1, graph_.OutgoingEdges(graph_.EdgeStart(e)));
        std::vector<EdgeId> v2;
        utils::push_back_all(v2, graph_.IncomingEdges(graph_.EdgeEnd(e)));
        bool eq = (v1.size() == 2 && v2.size() == 2) &&
                  ((v1[0] == v2[0] && v1[1] == v2[1]) || (v1[0] == v2[1] && v1[0] == v2[1]));
        return !eq;
    }

    double weight(size_t val, const Histogram &histogram, size_t backet_width) const {
        double result = 0;
        for (size_t i = 0; i < backet_width && val + i < histogram.size(); i++) {
            result += (double) (value(val + i, histogram) * std::min(i + 1, backet_width - i));
        }
        return result;
    }

    double Median(double thr = 500.0) const {
        std::vector<double> coverages;
        for (EdgeId e : graph_.edges()) {
            if (graph_.length(e) <= thr)
                continue;
            coverages.push_back(graph_.coverage(e));
        }

        auto middle_it = coverages.begin() + coverages.size() / 2;
#ifdef USE_GLIBCXX_PARALLEL
        __gnu_parallel::nth_element(coverages.begin(), middle_it, coverages.end());
#else
        std::nth_element(coverages.begin(), middle_it, coverages.end());
#endif
        return coverages[coverages.size() / 2];
    }

    size_t value(size_t arg, const Histogram &ssmap) const {
        auto it = ssmap.find(arg);
        if (it == ssmap.end())
            return 0;
        else
            return it->second;
    }

public:
    ErroneousConnectionThresholdFinder(const Graph &graph, size_t backet_width = 0) :
            graph_(graph), backet_width_(backet_width) {
    }

    double AvgCoverage() const {
        double cov = 0;
        double length = 0;
        for (EdgeId e : graph_.edges()) {
            double l = double(graph_.length(e));
            cov += graph_.coverage(e) * l;
            length += l;
        }
        return cov / length;
    }

    Histogram ConstructHistogram() const {
        Histogram result;
        for (EdgeId e : graph_.edges()) {
            if (!IsInteresting(e))
                continue;

            result[size_t(graph_.coverage(e))]++;
        }
        return result;
    }

    double FindThreshold(const Histogram &histogram) const {
        size_t backet_width = backet_width_;
        double avg_cov = AvgCoverageCounter<Graph>(graph_).Count();
        if (backet_width == 0)
            backet_width = (size_t)(0.3 * avg_cov+ 5);

        size_t size = 0;
        if (histogram.size() != 0)
            size = histogram.rbegin()->first + 1;
        INFO("Bucket size: " << backet_width);
        size_t cnt = 0;
        for (size_t i = 1; i + backet_width < size; i++) {
            if (weight(i, histogram, backet_width) > weight(i - 1, histogram, backet_width))
                cnt++;

            if (i > backet_width &&
                weight(i - backet_width,     histogram, backet_width) >
                weight(i - backet_width - 1, histogram, backet_width)) {
                cnt--;
            }
            if (2 * cnt >= backet_width)
                return (double) i;

        }
        INFO("Proper threshold was not found. Threshold set to 0.1 of average coverage");
        return 0.1 * avg_cov;
    }

    double FindThreshold() const {
        INFO("Finding threshold started");
        auto histogram = ConstructHistogram(/*weights*/);
        for (size_t i = 0; i < histogram.size(); i++) {
            TRACE(i << " " << histogram[i]);
        }
        double result = FindThreshold(histogram);
        INFO("Average edge coverage: " << AvgCoverage());
        INFO("Graph threshold: " << result);
        result = std::max(AvgCoverage(), result);
        INFO("Threshold finding finished. Threshold is set to " << result);
        return result;
    }
private:
    DECL_LOGGER("ThresholdFinder");
};

}

#endif /* OMNI_TOOLS_HPP_ */
