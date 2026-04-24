
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>
using namespace std;

class MaxFlow {
private:
    struct Edge {
        int to, cap, rev;
        Edge(int t, int c, int r) : to(t), cap(c), rev(r) {}
    };
    
    vector<vector<Edge>> graph;
    vector<int> level, iter;
    int n;
    
    void bfs(int s) {
        level.assign(n, -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);
        
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (const Edge& e : graph[v]) {
                if (e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[v] + 1;
                    q.push(e.to);
                }
            }
        }
    }
    
    int dfs(int v, int t, int f) {
        if (v == t) return f;
        for (int& i = iter[v]; i < graph[v].size(); i++) {
            Edge& e = graph[v][i];
            if (e.cap > 0 && level[v] < level[e.to]) {
                int d = dfs(e.to, t, min(f, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    graph[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }
    
public:
    MaxFlow(int n) : n(n), graph(n) {}
    
    void add_edge(int from, int to, int cap) {
        graph[from].push_back(Edge(to, cap, graph[to].size()));
        graph[to].push_back(Edge(from, 0, graph[from].size() - 1));
    }
    
    int max_flow(int s, int t) {
        int flow = 0;
        while (true) {
            bfs(s);
            if (level[t] < 0) return flow;
            iter.assign(n, 0);
            while (true) {
                int f = dfs(s, t, 1e9);
                if (f == 0) break;
                flow += f;
            }
        }
    }
    
    void reset() {
        for (int i = 0; i < n; i++) {
            for (Edge& e : graph[i]) {
                if (e.to < i) {
                    e.cap = 0;
                } else {
                    e.cap = 1;
                }
            }
        }
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n, m;
    cin >> n >> m;
    
    vector<pair<int, int>> edges;
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        a--; b--; // Convert to 0-based
        edges.push_back({a, b});
    }
    
    long long total = 0;
    
    // For small n, compute max flow directly for each pair
    if (n <= 100) {
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                MaxFlow mf(n);
                for (const auto& edge : edges) {
                    mf.add_edge(edge.first, edge.second, 1);
                    mf.add_edge(edge.second, edge.first, 1);
                }
                int flow = mf.max_flow(i, j);
                total += flow;
            }
        }
    } else {
        // For larger n, use a more efficient approach
        // Since max degree is 3 and all edges have unit capacity,
        // the max flow between any two nodes is at most 3
        
        // Build adjacency list
        vector<vector<int>> adj(n);
        for (const auto& edge : edges) {
            adj[edge.first].push_back(edge.second);
            adj[edge.second].push_back(edge.first);
        }
        
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                // Quick check: if i and j are directly connected, flow is at least 1
                bool direct = false;
                for (int neighbor : adj[i]) {
                    if (neighbor == j) {
                        direct = true;
                        break;
                    }
                }
                
                if (!direct) {
                    // Check if there's a path
                    vector<bool> visited(n, false);
                    queue<int> q;
                    q.push(i);
                    visited[i] = true;
                    bool found = false;
                    
                    while (!q.empty() && !found) {
                        int v = q.front();
                        q.pop();
                        for (int neighbor : adj[v]) {
                            if (!visited[neighbor]) {
                                if (neighbor == j) {
                                    found = true;
                                    break;
                                }
                                visited[neighbor] = true;
                                q.push(neighbor);
                            }
                        }
                    }
                    
                    if (!found) {
                        // No path, flow is 0
                        continue;
                    }
                }
                
                // There is a path, compute actual max flow
                MaxFlow mf(n);
                for (const auto& edge : edges) {
                    mf.add_edge(edge.first, edge.second, 1);
                    mf.add_edge(edge.second, edge.first, 1);
                }
                int flow = mf.max_flow(i, j);
                total += flow;
            }
        }
    }
    
    cout << total << endl;
    return 0;
}
