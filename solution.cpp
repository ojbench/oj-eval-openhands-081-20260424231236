
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
    
    void get_reachable(int s, vector<bool>& reachable) {
        reachable.assign(n, false);
        queue<int> q;
        q.push(s);
        reachable[s] = true;
        
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (const Edge& e : graph[v]) {
                if (e.cap > 0 && !reachable[e.to]) {
                    reachable[e.to] = true;
                    q.push(e.to);
                }
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

class GomoryHuTree {
private:
    int n;
    vector<vector<int>> tree;
    vector<vector<int>> tree_weight;
    
    void dfs(int u, int parent, int min_weight, int target, int& result) {
        if (u == target) {
            result = min_weight;
            return;
        }
        for (int i = 0; i < tree[u].size(); i++) {
            int v = tree[u][i];
            int w = tree_weight[u][i];
            if (v != parent) {
                dfs(v, u, min(min_weight, w), target, result);
                if (result != -1) return;
            }
        }
    }
    
public:
    GomoryHuTree(int n) : n(n), tree(n), tree_weight(n) {}
    
    void build(const vector<pair<int, int>>& edges) {
        vector<int> parent(n);
        vector<int> parent_weight(n);
        
        // Initialize parent array
        for (int i = 1; i < n; i++) {
            parent[i] = 0;
        }
        
        // Build Gomory-Hu tree
        for (int s = 1; s < n; s++) {
            int t = parent[s];
            
            // Create flow network
            MaxFlow mf(n);
            for (const auto& edge : edges) {
                mf.add_edge(edge.first, edge.second, 1);
                mf.add_edge(edge.second, edge.first, 1);
            }
            
            // Compute max flow
            int flow = mf.max_flow(s, t);
            parent_weight[s] = flow;
            
            // Find reachable vertices from s in residual graph
            vector<bool> reachable;
            mf.get_reachable(s, reachable);
            
            // Update parent pointers
            for (int i = s + 1; i < n; i++) {
                if (parent[i] == t && reachable[i]) {
                    parent[i] = s;
                }
            }
        }
        
        // Build tree adjacency lists
        for (int i = 1; i < n; i++) {
            tree[i].push_back(parent[i]);
            tree[parent[i]].push_back(i);
            tree_weight[i].push_back(parent_weight[i]);
            tree_weight[parent[i]].push_back(parent_weight[i]);
        }
    }
    
    int get_max_flow(int u, int v) {
        if (u == v) return 0;
        int result = -1;
        dfs(u, -1, 1e9, v, result);
        return result;
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
    
    GomoryHuTree ght(n);
    ght.build(edges);
    
    long long total = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            total += ght.get_max_flow(i, j);
        }
    }
    
    cout << total << endl;
    return 0;
}
