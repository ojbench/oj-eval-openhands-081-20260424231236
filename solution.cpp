
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
};

class GomoryHuTree {
private:
    int n;
    vector<vector<int>> tree;
    vector<vector<int>> tree_weight;
    vector<int> parent, parent_weight;
    vector<int> depth;
    vector<vector<int>> up;
    vector<vector<int>> min_edge;
    int LOG;
    
    void dfs_lca(int v, int p, int d) {
        depth[v] = d;
        up[v][0] = p;
        min_edge[v][0] = (p == -1) ? 1e9 : tree_weight[v][0];
        
        for (int i = 1; i < LOG; i++) {
            if (up[v][i-1] != -1) {
                up[v][i] = up[up[v][i-1]][i-1];
                min_edge[v][i] = min(min_edge[v][i-1], min_edge[up[v][i-1]][i-1]);
            }
        }
        
        for (int i = 0; i < tree[v].size(); i++) {
            int to = tree[v][i];
            if (to != p) {
                // Find the weight of edge v-to
                int weight = tree_weight[v][i];
                // Swap tree_weight[to] to have the correct weight
                for (int j = 0; j < tree[to].size(); j++) {
                    if (tree[to][j] == v) {
                        tree_weight[to][j] = weight;
                        break;
                    }
                }
                dfs_lca(to, v, d + 1);
            }
        }
    }
    
    int get_min_on_path(int u, int v) {
        if (depth[u] < depth[v]) swap(u, v);
        
        int result = 1e9;
        
        // Bring u up to depth of v
        for (int i = LOG-1; i >= 0; i--) {
            if (depth[u] - (1 << i) >= depth[v]) {
                result = min(result, min_edge[u][i]);
                u = up[u][i];
            }
        }
        
        if (u == v) return result;
        
        for (int i = LOG-1; i >= 0; i--) {
            if (up[u][i] != -1 && up[u][i] != up[v][i]) {
                result = min(result, min_edge[u][i]);
                result = min(result, min_edge[v][i]);
                u = up[u][i];
                v = up[v][i];
            }
        }
        
        result = min(result, min_edge[u][0]);
        result = min(result, min_edge[v][0]);
        
        return result;
    }
    
public:
    GomoryHuTree(int n) : n(n), tree(n), tree_weight(n), parent(n), parent_weight(n) {
        LOG = 1;
        while ((1 << LOG) <= n) LOG++;
        depth.resize(n);
        up.assign(n, vector<int>(LOG, -1));
        min_edge.assign(n, vector<int>(LOG, 1e9));
    }
    
    void build(const vector<pair<int, int>>& edges) {
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
        
        // Preprocess for LCA queries
        dfs_lca(0, -1, 0);
    }
    
    int get_max_flow(int u, int v) {
        if (u == v) return 0;
        return get_min_on_path(u, v);
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
