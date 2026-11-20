#include <iostream>
#include "oracle.hpp"
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Oracle oracle;
    string cmd;
    while (cin >> cmd) {
        if (cmd == "U") {
            PlayerId id; Score score;
            cin >> id >> score;
            oracle.update(id, score);
        } else if (cmd == "D") {
            PlayerId id;
            cin >> id;
            oracle.removePlayer(id);              // missing id: ignored, no output
        } else if (cmd == "R") {
            PlayerId id; long long r;
            cin >> id;
            if (oracle.rank(id, r)) cout << r << "\n";
            else cout << MSG_NOT_FOUND << "\n";
        } else if (cmd == "S") {
            long long k; PlayerId id;
            cin >> k;
            if (oracle.select(k, id)) cout << id << "\n";
            else cout << MSG_INVALID << "\n";
        } else if (cmd == "L") {
            long long a, b;
            cin >> a >> b;
            vector<PlayerId> ids = oracle.list(a, b);   // clamped; may be empty
            for (size_t i = 0; i < ids.size(); i++) { if (i) cout << ' '; cout << ids[i]; }
            cout << "\n";                               // an empty result is an empty line
        } else cerr << "Unknown command: " << cmd << "\n";
    }
    return 0;
}
