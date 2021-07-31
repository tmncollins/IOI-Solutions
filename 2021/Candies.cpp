#include <iostream>
#include <list>
#include <stdio.h>
#include <sstream>
#include <vector>  
#include <iterator> 
#include <tuple>
#include <istream>
#include <map>
#include <set>
#include <deque>
#include <limits>
#include <algorithm>
#include <queue>
#include <time.h>
#include <cstring>
#include <array>
#include <unordered_map>
#include <fstream>
#include <cmath>

using namespace std;

// NOTE 1 << x is equal to 2^x

const int bits = 19;
const long long  INF = 1e18;
const int SIZE = 1 << 20;

array<long long, SIZE> minTree, maxTree, lazy;  

// segment tree with both min and max storage
// min stores the minimum number of candies that could possibly be in a box
// max stores the maximum number of candies that could possible be in a box

// the nth node in the underlying array of the segment tree stores the state of the boxes 
// after the nth update has been processed. 

// we the updates are done in order of ranges rather than chronilogical order, allowing the 
// final value for each box to be determined one by one from left to right
// this means for the update all the boxes to the right of the one being updated is also updated
// and all the boxes to the left can be ignored as they have already been processed

struct segment_tree {
    long long val = 0;
    long long minV = INF;
    long long maxV = -INF;

    void push(int node, long long v) {
        // update the number of sweets in this infinite capacity box
        val += v;

        // find the base node (suffix)
        node += (1 << bits);

        lazy[node] += v;

        // work way back to top of segment tree, updating values
        while (node > 1) {
            // update lazy propogation info
            if (node % 2 == 0) {
                // if this is the left child node, update the lazy value for the right child node as well
                lazy[node+1] += v;
            }
            // update parent tree data with this node and the parent's other child
            //                           val of this child            value of other child
            minTree[node / 2] = min(minTree[node] + lazy[node], minTree[1 ^ node] + lazy[1 ^ node]);
            maxTree[node / 2] = max(maxTree[node] + lazy[node], maxTree[1 ^ node] + lazy[1 ^ node]);

            // move to the parent node
            node /= 2;
        }
    } 

    int num_sweets(long long cap) {
        int node = 1; // start at head
        minV = INF;
        maxV = -INF;

        long long lazyTot = 0;

        // move down the tree towards the right to find the greatest index of node (most days processed),
        // so that the range of the maximum and minimum values is >= required capacity of box
        while (node < SIZE / 2) {
            // accumulate the total for the lazy propogation
            lazyTot += lazy[node];
            // move to node's left child
            node *= 2;

            // get the range of the max and min possible values
            // if the maximum range is greater than the capacity, we have to move on to the next node (right hand child)
            long long range = max(maxV, maxTree[node + 1] + lazy[node + 1] + lazyTot) - min(minV, minTree[node + 1] + lazy[node + 1] + lazyTot);
            if (range > cap) {
                node += 1;
            }
            else {
                // update range info
                maxV = max(maxV, maxTree[node + 1] + lazy[node + 1] + lazyTot);
                minV = min(minV, minTree[node + 1] + lazy[node + 1] + lazyTot);
            }
        }

        if (minTree[node] + lazy[node] + lazyTot < val) {
            return cap - (maxV - val);
        }
        else {
            return val - minV;
        }
    }
};

array<vector<pair<int, long long>>, SIZE> updates;

vector<int> distribute_candies(vector<int> c, vector<int> l, vector<int> r, vector<int> v) {
    int n = c.size();
    int q = l.size();

    segment_tree tree;

    for (int k = 0; k < q; k++) {
        updates[l[k]].push_back(make_pair(k, v[k])); // everything to the right of the left bound needs to be incr by v
        updates[r[k] + 1].push_back(make_pair(k, -v[k])); // everything to the right of the right bound needs to be decr by v
    }

    // set up array to return
    vector<int> boxes;
    boxes.resize(n);

    // process each update from left to right
    for (int k = 0; k < n; k++) {
        // process all of the updates that concern this box
        for (pair<int, long long> p : updates[k]) {
            // push the update value to the update day order node
            // the values are stored as if the boxes have infinite capacity
            tree.push(p.first + 2, p.second);
        }

        // check if the range of the entire tree is less than the box capacity
        if (maxTree[1] - minTree[1] < c[k]) {
            // this is the simple case 
            // the number of candies is equal to the sweets in the infinite capacity box minus the 
            // mininum possible number of sweets in the whole tree minus the lazy propogation of the whole tree
            boxes[k] = tree.val - (minTree[1] + lazy[1]);
        }
        else {
            // binary search to find the number of sweets in this box, given its capacity
            boxes[k] = tree.num_sweets(c[k]);
        }
    }

    return boxes;
}

int main() {

    clock_t tStart = clock();   

    vector<int> c, l, r, v;
    int n, q;

    ifstream fin("input.txt");
    ofstream fout("output.txt");

    fin >> n;
    c.resize(n);
    for (int k = 0; k < n; k++) fin >> c[k];
    fin >> q;
    l.resize(q);
    r.resize(q);
    v.resize(q);
    for (int k = 0; k < q; k++) fin >> l[k] >> r[k] >> v[k];

    vector<int> x = distribute_candies(c, l, r, v);

    printf("Time taken: %.4fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    for (int i : x) fout << i << " ";
}