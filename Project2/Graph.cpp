#include "Graph.h"
#include <fstream>
#include <sstream>
#include <stack>
#include <queue>
#include <unordered_set>
#include <utility>  // load pair class
#include <tuple>    // load tuple class
#include <iostream>

Graph::Graph(const char* const & edgelist_csv_fn) {
    // open and read file
    ifstream my_file(edgelist_csv_fn);      // open the file
    string line;                    
    while(getline(my_file, line)) {  
        istringstream ss(line);      
        // grab vertices and weight
        string node1, node2, distLabel; 
        getline(ss, node1, ',');     
        getline(ss, node2, ',');    
        getline(ss, distLabel, '\n');    

        double dist = std::stod(distLabel);
        nodesMap[node1][node2] = dist;
        nodesMap[node2][node1] = dist;  // do both since this is an undirected edge
    }
    my_file.close();                 // close file when done
}


unsigned int Graph::num_nodes() {
    return nodesMap.size();
}


vector<string> Graph::nodes() {
    vector<string> results;

    // iterate through map, and add the key labels to results
    for (auto keyVal : nodesMap) {
        results.push_back(keyVal.first);
    }

    return results;
}


unsigned int Graph::num_edges() {
    int count = 0;

    // iterate through keyVal pairs, increment count by the size of the hashmap
    for (auto keyVal : nodesMap) {
        count += (keyVal.second).size();
    }

    count /= 2; // we double count th undirected edges

    return count;
}

unsigned int Graph::num_neighbors(string const & node_label) {
    return nodesMap[node_label].size();
}

double Graph::edge_weight(string const & u_label, string const & v_label) {
    // if the edge does not exist
    if (nodesMap[u_label].find(v_label) == nodesMap[u_label].end()) {
        return -1;
    } else {
        return nodesMap[u_label][v_label];
    }
}

vector<string> Graph::neighbors(string const & node_label) {
    vector<string> results;
    for (auto nodeDist : nodesMap[node_label]) {
        results.push_back(nodeDist.first);
    }

    return results;
}

// Use BFS algorithm, with queue
vector<string> Graph::shortest_path_unweighted(string const & start_label, string const & end_label) {
    // maybe use 3 tuples and keep track of the distance as well to prevent disorder!!!!!****
    unordered_map<string, string> previous; // store previous pointer
    queue<pair<string, string>> toVisit;
    stack<string> reversePath;
    unordered_set<string> visited;
    vector<string> results;

    toVisit.push(make_pair(start_label, ""));

    while (!toVisit.empty()) {
        pair<string, string> currPrev = toVisit.front();
        toVisit.pop();

        string curr = currPrev.first;
        string prev = currPrev.second;

        if (curr == end_label) {
            while (curr != "") {
                reversePath.push(curr);
                curr = previous[curr];
            }

            while (!reversePath.empty()) {
                results.push_back(reversePath.top());
                reversePath.pop();
            }

            return results;
        }

        if (visited.count(curr) == 0) {
            visited.insert(curr);

            for (auto nodeDist : nodesMap[curr]) {
                if (visited.count(nodeDist.first) == 0) { // if neighbor has not been visited yet, then add
                    pair<string, string> nextCurr = make_pair(nodeDist.first, curr);
                    toVisit.push(nextCurr);
                    if (previous.count(nodeDist.first) == 0) {
                        previous[nodeDist.first] = curr; // to prevent out of order
                    }
                    //previous[nodeDist.first] = curr;
                }
            }
        }
    }

    return results;

}


class Comp {
    public:
        bool operator()(const tuple<string, string, double>& a, const tuple<string, string, double>& b) const {
            return (get<2>(a) > get<2>(b));
        }
};

vector<tuple<string,string,double>> Graph::shortest_path_weighted(string const & start_label, string const & end_label) {
    // TODO
    vector<tuple<string, string, double>> results;
    priority_queue<tuple<string, string, double>, vector<tuple<string, string, double>>, Comp> toVisit;
    unordered_set<string> visited;
    unordered_map<string, string> previous;
    
    toVisit.push(make_tuple(start_label, start_label, -1));

    // special case: start == end
    if (start_label == end_label) {
        results.push_back(toVisit.top());  // just add (start, start, 0) to results
        return results;
    }

    while (!toVisit.empty()) {
        tuple<string, string, double> curr = toVisit.top();  // (from, to, dist)
        toVisit.pop();
        string to = get<1>(curr);
        string from = get<0>(curr);
        double distance = get<2>(curr);

        // we encounter the goal
        if (to == end_label) {
            // use stack to get path
            previous[to] = from;
            stack<tuple<string, string, double>> reversePath;
            while (to != start_label) {
                string prev = previous[to];
                double weight = nodesMap[prev][to];
                reversePath.push(make_tuple(prev, to, weight));
                to = prev;
            }

            while (!reversePath.empty()) {
                results.push_back(reversePath.top());
                reversePath.pop();
            }

            return results;
        }

        // "to" not visited
        if (visited.count(to) == 0) {
            visited.insert(to);
            previous[to] = from; // we got the path finally

            for (auto nbr : nodesMap[to]) {
                string name = nbr.first;
                double weight = nbr.second;

                if (visited.count(name) == 0) {  // for all nbrs not visited
                    toVisit.push(make_tuple(to, name, distance + weight));
                }
            }
        }
    }

    return results; // no path exists
}

/*
DS:
    unordered_set unvisited <labels of the nodes>
    queue for BFS
    unordered_set c for connected_components <labels of nodes>
    // might want to make a vector since we're making a vector of vectors
connected_components
    Initialize all nodes in the graph to "unvisited"
    While there are still unvisited nodes:
    Arbitrarily choose one of the remaining unvisited nodes (call it u)
    Perform BFS starting at u, and store all nodes visited in the BFS (including u) in a set c
    // check for threshold in the BFS
    Once BFS is complete, output c as a component of the graph
*/
vector<vector<string>> Graph::connected_components(double const & threshold) {
    // TODO
    vector<vector<string>> results;

    unordered_set<string> unvisited;

    for (auto it = nodesMap.begin(); it != nodesMap.end(); it++) { // get nodes into unvisited
        unvisited.insert(it->first);
    }

    while (!unvisited.empty()) {
        // get a node
        auto front = unvisited.begin(); // iterator to first element
        string label = *front; // does that get the label?
        //cout << "label: " << label;
        unordered_set<string> visited;
        queue<string> toVisit;

        string start = label;
        toVisit.push(start);

        while (!toVisit.empty()) {
            string curr = toVisit.front();
            toVisit.pop();
            if (visited.count(curr) == 0) {
                visited.insert(curr);
                unvisited.erase(curr);

                for (auto nbrDist : nodesMap[curr]) {
                    double weight = nbrDist.second;
                    string nbr = nbrDist.first;

                    if (weight <= threshold) {
                        toVisit.push(nbr);
                    }
                }
            }
        }

        // we got visited, so iterate through into a vector

        vector<string> subresult;

        for (string s : visited) {
            subresult.push_back(s);
        }

        results.push_back(subresult);

        // bfs, check for threshold as well, remove from unvisited and add to visited
        // store bfs results into a vector, put this vector into the results vector
    }

    return results;
}

double Graph::smallest_connecting_threshold(string const & start_label, string const & end_label) {
    // TODO
    return 0.0;
}



