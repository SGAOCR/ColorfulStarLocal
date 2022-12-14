#include <stdlib.h>
#include <cstdio>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <cstdarg>
#include <unordered_map>
#include <omp.h>
#include "../header/heapLLU.h"
#include "../header/Graph.hpp"
#include "../header/tool.hpp"
#include "../header/ColorfulStarCore.hpp"

using namespace std;

int main(int argc, char **argv)
{
    auto t0 = getTime();
    
    // build a graph from the input file
    Graph g;
    int h = atoi(argv[1]);
    cout << "Reading edgelist from file: " << argv[2] << endl;
    g.readedgelist(argv[2]);
    cout << "Reading edgelist finished!" << endl;
    g.mkGraph();
    cout << "mkGraph finished!" << endl;

    omp_set_num_threads(atoi(argv[3]));
    int optimization = atoi(argv[4]);

    int *color = new int[g.n];
    int colorNum = g.color(color);
    printf("Total number of colors: %d\n", colorNum);


    int *newId = new int[g.n]();
    if (optimization >= 2)
    {
        pair<int, __int128> *idBounds = new pair<int, __int128>[g.n];

        for (int i = 0; i < g.n; i++)
        {
            idBounds[i].first = i;
            idBounds[i].second = g.deg[i];
        }

        sort(idBounds, idBounds + g.n, ubCMPRe);

        int *newAdj = new int[2 * g.e];
        int *newDeg = new int[g.n]();
        int *newCD = new int[g.n + 1];

        int *colorBak = new int[g.n];
        memcpy(colorBak, color, sizeof(int) * g.n);
        for (int i = 0; i < g.n; i++)
        {
            newId[idBounds[i].first] = i;
            color[i] = colorBak[idBounds[i].first];
        }
        delete[] colorBak;

        newCD[0] = 0;
        for (int i = 0; i < g.n; i++)
        {
            int id = idBounds[i].first;
            newCD[i + 1] = newCD[i] + g.deg[id];

            for (int j = g.cd[id]; j < g.cd[id] + g.deg[id]; j++)
            {
                newAdj[newCD[i] + j - g.cd[id]] = newId[g.adj[j]];
            }
            newDeg[i] = g.deg[id];
        }

        g.adj = newAdj;
        g.cd = newCD;
        g.deg = newDeg;
    }

    auto t1 = getTime();

    __int128 **dp = new __int128 *[g.n];
    int **CC = new int *[g.n];
    __int128 *ColofulStarCoreNum = new __int128[g.n];

    // initialize the colorful h-star degree for each node
    initColStarDegree(g, dp, h, colorNum, color, CC);
    // compute the colorful h-star core decomposition
    ColorfulStarHIndex(g, dp, h, color, CC, ColofulStarCoreNum, colorNum, optimization);

    auto t2 = getTime();
    printf("- Overall time = %lf\n", ((double)timeGap(t1, t2)) / 1e6);

    if (argc >= 6)
    {
        ofstream outfile(argv[5], ios::out);
        if (optimization >= 2)
        {
            for (int i = 0; i < g.n; i++)
                outfile << i << " " << _int128_to_str(ColofulStarCoreNum[newId[i]]) << endl;
        }
        else
        {
            for (int i = 0; i < g.n; i++)
            {
                outfile << i << " " << _int128_to_str(ColofulStarCoreNum[i]) << endl;
            }
        }

        outfile.close();
    }

    return 0;
}
