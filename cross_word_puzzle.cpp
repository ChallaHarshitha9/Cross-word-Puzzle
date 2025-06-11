#include <bits/stdc++.h>
using namespace std;

struct position {
    int x;
    int y;
    int orientation;
};

class Grid {
public:
    int n, m, w;
    vector<vector<char>> grid;
    vector<position> orient;

    Grid(vector<vector<char>> &grid, vector<position> &orient)
    {
        this->n = grid.size();
        this->m = grid[0].size();
        this->w = orient.size() - 1;

        this->grid = grid;
        this->orient = orient;

        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < grid[i].size(); j++) {
                if (grid[i][j] >= '0' && grid[i][j] <= '9') {
                    int num = grid[i][j] - '0';
                    this->orient[num].y = i;
                    this->orient[num].x = j;
                }
            }
        }
    }
};

class Solution {
public:
    Grid *g;
    vector<string> listOfWords;
    vector<unordered_set<int>> choices;
    vector<vector<pair<int, int>>> adj;
    vector<int> wordLen;
    vector<unordered_map<int, int>> wordPosConstraint;
    vector<int> ans;

    Solution(Grid *g, vector<string> &listOfWords)
    {
        this->g = g;
        this->listOfWords = listOfWords;
        choices.resize(g->w + 1);
        wordLen.resize(g->w + 1);
        ans.resize(g->w + 1);
        for (int constraint = 1; constraint <= g->w; constraint++) {
            for (int i = 0; i < listOfWords.size(); i++) {
                choices[constraint].insert(i);
            }
        }
        adj.resize(g->w + 1);
    }

    void createConstraintGraph()
    {
        vector<vector<pair<int, int>>> constraintGrid(g->grid.size(), vector<pair<int, int>>(g->grid[0].size()));
        for (int consNum = 1; consNum < g->w + 1; consNum++) {
            auto pos = g->orient[consNum];
            int x = pos.x, y = pos.y;
            bool down = pos.orientation;
            int cnt = 0;
            for (int i = y, j = x; i < g->grid.size() && j < g->grid[0].size();) {
                if (g->grid[i][j] == '*') break;
                if (constraintGrid[i][j].first != 0) {
                    adj[constraintGrid[i][j].first].push_back(make_pair(consNum, cnt));
                    adj[consNum].push_back(constraintGrid[i][j]);
                }
                constraintGrid[i][j] = make_pair(consNum, cnt++);
                if (down) i++; else j++;
            }
            wordLen[consNum] = cnt;
        }
    }

    void nodeConsistency()
    {
        for (int i = 1; i < choices.size(); i++) {
            int val = wordLen[i];
            for (int j = 0; j < listOfWords.size(); j++) {
                if (listOfWords[j].size() != val) {
                    choices[i].erase(j);
                }
            }
        }
    }

    bool arcConsistency3()
    {
        queue<pair<int, int>> q;
        set<pair<int, int>> pairsInQ;
        int i = 0;
        wordPosConstraint.resize(adj.size());
        for (auto &adjList : adj) {
            for (auto &edge : adjList) {
                q.push({i, edge.first});
                pairsInQ.insert({i, edge.first});
                wordPosConstraint[i][edge.first] = edge.second;
            }
            i++;
        }

        while (!q.empty()) {
            pair<int, int> edge = q.front(); q.pop();
            pairsInQ.erase(edge);
            int xi = edge.first, xj = edge.second;

            if (revise(xi, xj)) {
                if (choices[xi].empty()) return false;
                for (auto &neigh : adj[xi]) {
                    if (neigh.first == xj) continue;
                    if (!pairsInQ.count({neigh.first, xi})) {
                        q.push({neigh.first, xi});
                        pairsInQ.insert({neigh.first, xi});
                    }
                }
            }
        }
        return true;
    }

    bool revise(int xi, int xj)
    {
        bool revised = false;
        stack<int> toRemove;
        for (int wordi : choices[xi]) {
            int indexXi = wordPosConstraint[xi][xj];
            int indexXj = wordPosConstraint[xj][xi];
            bool valid = false;

            for (int wordj : choices[xj]) {
                if (listOfWords[wordi][indexXj] == listOfWords[wordj][indexXi]) {
                    valid = true;
                    break;
                }
            }

            if (!valid) {
                toRemove.push(wordi);
                revised = true;
            }
        }

        while (!toRemove.empty()) {
            choices[xi].erase(toRemove.top());
            toRemove.pop();
        }

        return revised;
    }

    bool backtrack(int ind)
    {
        if (ind > g->w) return true;
        if (ans[ind] != -1) return backtrack(ind + 1);

        for (int choice : choices[ind]) {
            bool valid = true;
            for (auto &[neighbor, pos] : adj[ind]) {
                if (ans[neighbor] == -1) continue;
                if (listOfWords[choice][wordPosConstraint[neighbor][ind]] !=
                    listOfWords[ans[neighbor]][wordPosConstraint[ind][neighbor]]) {
                    valid = false;
                    break;
                }
            }

            if (!valid) continue;

            ans[ind] = choice;
            if (backtrack(ind + 1)) return true;
            ans[ind] = -1;
        }

        return false;
    }

    void solve()
    {
        createConstraintGraph();
        nodeConsistency();
        arcConsistency3();

        fill(ans.begin(), ans.end(), -1);
        if (backtrack(1)) {
            for (int i = 1; i < ans.size(); i++) {
                cout << i << ": " << listOfWords[ans[i]] << "\n";
            }
        } else {
            cout << "No valid assignment\n";
        }
    }
};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n = 4, m = 5, w = 4, tot = 14;
    vector<vector<char>> grid = {
        {'1', '.', '2', '.', '3'},
        {'*', '*', '.', '*', '.'},
        {'*', '4', '.', '.', '.'},
        {'*', '*', '.', '*', '*'}
    };

    vector<position> orient(w + 1);
    orient[1].orientation = 0; // across
    orient[2].orientation = 1; // down
    orient[3].orientation = 1;
    orient[4].orientation = 0;

    vector<string> words = {
        "astar", "happy", "hello", "hoses", "live", "load", "loom",
        "peal", "peel", "save", "talk", "ant", "oak", "old"
    };

    Grid *g = new Grid(grid, orient);
    Solution *s = new Solution(g, words);
    s->solve();

    return 0;
}
