    #include <iostream>
#include <vector>
using namespace std;
vector<vector<int>> dep;
vector<vector<int>> conf;
void dfs(vector<bool> & ans, int index) {
    ans[index] = true;
    for (int i = 0; i < dep[index].size(); i++) {
        if (!ans[dep[index][i]]) {
            dfs(ans, dep[index][i]);
        }
    }
}

int main() {
    int n, m;
    cin >> n >> m;
    dep = vector<vector<int>> (n, vector<int> ());
    conf = vector<vector<int>> (n, vector<int> ());
    vector<bool> req(n, 0);
    vector<bool> ans(n, 0);
    for (int i = 0; i < m; i++) {
        string s;
        cin >> s;
        if (s == "DEP") {
            int first, second;
            cin >> first >> second;
            dep[first-1].push_back(second-1);
        }
        else if (s == "CONFLICT") {
            int first, second;
            cin >> first >> second;
            conf[first-1].push_back(second-1);
            conf[second-1].push_back(first-1);
        }
        else if (s == "REQUIRE") {
            int elem;
            cin >> elem;
            req[elem-1] = true;
        }
    }
    for (int i = 0; i < n; i++) {
        if (req[i]) {
            dfs(ans, i);
        }
    }
    bool flag = 1;
    for (int i = 0; i < n; i++) {
        if (ans[i]) {
            for (int j = 0; j < conf[i].size(); j++) {
                if (ans[conf[i][j]] == 1) {
                    flag = 0;
                    break;
                }
            }
        }
        if (flag == 0) break;
    }
    if (!flag) {
        cout << "IMPOSSIBLE" << '\n';
        return 0;
    }
    else {
        for (int i = 0; i < n; i++) {
            if (ans[i]) cout << i+1 << ": ON"<< '\n';
            else cout << i+1 << ": OFF" << '\n';
        }
    }
}