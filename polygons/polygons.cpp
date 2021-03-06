#include <stdio.h>
#include <iostream>
#include <fstream>
#include <limits>
#include <tuple>
#include <math.h>
#include <stdlib.h>

#include "polygons.h"
#include "intersection.h"
#include "distance.h"

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)

void polygons_context::check_that_context_is_initialized() const
{
    if (not is_initialized)
    {
        fprintf(stderr, "ERROR: context is not initialized\n");
        exit(-1);
    }
}

POLYGONS_API
polygons_context *polygons_new_context()
{
    return AS_TYPE(polygons_context, new polygons_context());
}
polygons_context::polygons_context() { is_initialized = true; }

POLYGONS_API
void polygons_free_context(polygons_context *context)
{
    if (!context)
        return;
    delete AS_TYPE(polygons_context, context);
}
polygons_context::~polygons_context()
{
    nodes.clear();

    for (std::size_t i = 0; i < polygons.size(); i++)
    {
        polygons[i].clear();
    }
    polygons.clear();

    is_initialized = false;
}

std::vector<node> build_nodes(const int n, const std::vector<node> children)
{
    int num_new_nodes = children.size() / n;
    if (children.size() % n > 0)
        num_new_nodes++;

    std::vector<node> new_nodes;

    std::size_t i = 0;
    for (int k = 0; k < num_new_nodes; k++)
    {
        node new_node;
        for (int l = 0; l < n; l++)
        {
            if (i < children.size())
            {
                new_node.add_child_node(children[i]);
                i++;
            }
        }
        new_nodes.push_back(new_node);
    }

    return new_nodes;
}

POLYGONS_API
void polygons_add_polygon(polygons_context *context,
                          const int num_points,
                          const double x[],
                          const double y[],
                          const int indices[])
{
    AS_TYPE(polygons_context, context)->add_polygon(num_points, x, y, indices);
}
void polygons_context::add_polygon(const int num_points,
                                   const double x[],
                                   const double y[],
                                   const int indices[])
{
    check_that_context_is_initialized();

    std::vector<edge> temp;
    for (int i = 0; i < num_points - 1; i++)
    {
        point p1 = {indices[i], x[i], y[i]};
        point p2 = {indices[i + 1], x[i + 1], y[i + 1]};
        edge e = {p1, p2};

        temp.push_back(e);
    }
    polygons.push_back(temp);
    temp.clear();

    // we clear and rebuild tree every time we add a polygon
    nodes.clear();

    // FIXME move this outside and make it configurable
    const int NUM_EDGES_PER_NODE = 4;
    const int NUM_NODE_CHILDREN = 4;

    for (std::size_t ip = 0; ip < polygons.size(); ip++)
    {
        int num_edges = polygons[ip].size();
        int num_nodes = num_edges / NUM_EDGES_PER_NODE;
        if (num_edges % NUM_EDGES_PER_NODE > 0)
            num_nodes++;

        // collect NUM_EDGES_PER_NODE edges into each node
        int i = 0;
        for (int k = 0; k < num_nodes; k++)
        {
            node new_node;
            for (int l = 0; l < NUM_EDGES_PER_NODE; l++)
            {
                if (i < num_edges)
                {
                    new_node.add_child_edge(polygons[ip][i]);
                    i++;
                }
            }
            nodes.push_back(new_node);
        }
    }

    // build the tree of nodes
    while (nodes.size() > 1)
    {
        nodes = build_nodes(NUM_NODE_CHILDREN, nodes);
    }
}

POLYGONS_API
void polygons_get_distances_edge(const polygons_context *context,
                                 const int num_points,
                                 const double x[],
                                 const double y[],
                                 double distances[])
{
    AS_CTYPE(polygons_context, context)
        ->get_distances_edge(num_points, x, y, distances);
}
void polygons_context::get_distances_edge(const int num_points,
                                          const double x[],
                                          const double y[],
                                          double distances[]) const
{
    double large_number = std::numeric_limits<double>::max();

    for (int i = 0; i < num_points; i++)
    {
        point p = {-1, x[i], y[i]};
        distances[i] = sqrt(nodes[0].get_distance_edge(large_number, p));
    }
}

POLYGONS_API
void polygons_get_distances_vertex(const polygons_context *context,
                                   const int num_points,
                                   const double x[],
                                   const double y[],
                                   double distances[])
{
    AS_CTYPE(polygons_context, context)
        ->get_distances_vertex(num_points, x, y, distances);
}
void polygons_context::get_distances_vertex(const int num_points,
                                            const double x[],
                                            const double y[],
                                            double distances[]) const
{
    double large_number = std::numeric_limits<double>::max();

#pragma omp parallel for
    for (int i = 0; i < num_points; i++)
    {
        point p = {-1, x[i], y[i]};
        auto t = nodes[0].get_distance_vertex(0, large_number, p);
        distances[i] = sqrt(std::get<1>(t));
    }
}

POLYGONS_API
void polygons_get_closest_vertices(const polygons_context *context,
                                   const int num_points,
                                   const double x[],
                                   const double y[],
                                   int incides[])
{
    AS_CTYPE(polygons_context, context)
        ->get_closest_vertices(num_points, x, y, incides);
}
void polygons_context::get_closest_vertices(const int num_points,
                                            const double x[],
                                            const double y[],
                                            int indices[]) const
{
    double large_number = std::numeric_limits<double>::max();

#pragma omp parallel for
    for (int i = 0; i < num_points; i++)
    {
        point p = {-1, x[i], y[i]};
        auto t = nodes[0].get_distance_vertex(0, large_number, p);
        indices[i] = std::get<0>(t);
    }
}

POLYGONS_API
void polygons_contains_points(const polygons_context *context,
                              const int num_points,
                              const double x[],
                              const double y[],
                              bool contains_points[])
{
    AS_CTYPE(polygons_context, context)
        ->contains_points(num_points, x, y, contains_points);
}
void polygons_context::contains_points(const int num_points,
                                       const double x[],
                                       const double y[],
                                       bool contains_points[]) const
{
    check_that_context_is_initialized();

    std::fill(&contains_points[0], &contains_points[num_points], false);
#pragma omp parallel for
    for (int i = 0; i < num_points; i++)
    {
        point p = {-1, x[i], y[i]};
        int n = 0;
        contains_points[i] = nodes[0].num_intersections(n, p) % 2 != 0;
    }
}
