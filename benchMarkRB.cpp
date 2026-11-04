// rb_bench.cpp : benchmark de l'arbre Red-Black
//
// Compilation (IMPORTANT : toujours avec -O2, sinon les mesures sont faussees) :
//     g++ -O2 -std=c++17 -o rb_bench rb_bench.cpp
//
// Utilisation :
//     ./rb_bench                      -> tailles 10, 100, ... jusqu'a 1 000 000 000 (selon la RAM)
//     ./rb_bench 10000000             -> s'arrete a 10^7
//     ./rb_bench 10000000 1000000     -> idem, affichage chronometre seulement jusqu'a n <= 10^6
//
// Pour chaque taille n :
//   1. INSERT       : n insertions (cles pairs, ordre aleatoire)
//   2. RECHERCHE+   : n recherches de cles presentes
//   3. RECHERCHE-   : n recherches de cles absentes (cles impaires, melangees avec les pairs)
//   4. AFFICHAGE    : rendu complet de l'arbre (colore, comme ton display) vers un compteur
//                     d'octets, pour mesurer le cout du rendu sans la lenteur du terminal
//   5. SUPPRESSION  : n suppressions dans un autre ordre aleatoire (l'arbre finit vide)
// Les phases sont chronometrees avec std::chrono::steady_clock.
// Pour les petites tailles, tout est repete plusieurs fois et on prend la moyenne.

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <streambuf>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <sys/types.h>
  #include <sys/sysctl.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

// ---------------------------------------------------------------- styles ANSI
const string RESET   = "\033[0m";
const string RED_BG  = "\033[1;97;41m";
const string BLK_BG  = "\033[1;97;100m";
const string DIM     = "\033[2m";
const string CYAN    = "\033[1;36m";
const string GREEN   = "\033[1;32m";
const string YELLOW  = "\033[1;33m";
const string RED     = "\033[1;31m";
const string BOLD    = "\033[1m";

// ---------------------------------------------------------------- arbre Red-Black
struct Node {
    int data;
    bool color; // true = RED, false = BLACK
    Node *left, *right, *parent;
};

class RedBlackTree {
private:
    Node* root;
    Node* TNULL;

    void initializeNULLNode(Node* node, Node* parent) {
        node->data = 0;
        node->color = false;
        node->left = nullptr;
        node->right = nullptr;
        node->parent = parent;
    }

    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != TNULL) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rightRotate(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != TNULL) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    void fixInsert(Node* k) {
        Node* u;
        while (k->parent && k->parent->color == true) {
            if (k->parent == k->parent->parent->right) {
                u = k->parent->parent->left;
                if (u->color == true) {
                    u->color = false;
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    k = k->parent->parent;
                } else {
                    if (k == k->parent->left) {
                        k = k->parent;
                        rightRotate(k);
                    }
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    leftRotate(k->parent->parent);
                }
            } else {
                u = k->parent->parent->right;
                if (u->color == true) {
                    u->color = false;
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    k = k->parent->parent;
                } else {
                    if (k == k->parent->right) {
                        k = k->parent;
                        leftRotate(k);
                    }
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    rightRotate(k->parent->parent);
                }
            }
            if (k == root) break;
        }
        root->color = false;
    }

    void transplant(Node* u, Node* v) {
        if (u->parent == nullptr) root = v;
        else if (u == u->parent->left) u->parent->left = v;
        else u->parent->right = v;
        v->parent = u->parent;
    }

    Node* minimum(Node* node) {
        while (node->left != TNULL) node = node->left;
        return node;
    }

    void fixDelete(Node* x) {
        Node* s;
        while (x != root && x->color == false) {
            if (x == x->parent->left) {
                s = x->parent->right;
                if (s->color == true) {
                    s->color = false;
                    x->parent->color = true;
                    leftRotate(x->parent);
                    s = x->parent->right;
                }
                if (s->left->color == false && s->right->color == false) {
                    s->color = true;
                    x = x->parent;
                } else {
                    if (s->right->color == false) {
                        s->left->color = false;
                        s->color = true;
                        rightRotate(s);
                        s = x->parent->right;
                    }
                    s->color = x->parent->color;
                    x->parent->color = false;
                    s->right->color = false;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                s = x->parent->left;
                if (s->color == true) {
                    s->color = false;
                    x->parent->color = true;
                    rightRotate(x->parent);
                    s = x->parent->left;
                }
                if (s->right->color == false && s->left->color == false) {
                    s->color = true;
                    x = x->parent;
                } else {
                    if (s->left->color == false) {
                        s->right->color = false;
                        s->color = true;
                        leftRotate(s);
                        s = x->parent->left;
                    }
                    s->color = x->parent->color;
                    x->parent->color = false;
                    s->left->color = false;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = false;
    }

    string badge(Node* n) {
        return (n->color ? RED_BG : BLK_BG) + " " + to_string(n->data) + " " + RESET;
    }

    // Meme rendu que ton affichage, mais ecrit dans un flux quelconque.
    void printNode(ostream& os, Node* node, const string& prefix, bool isLast, bool isRoot, const string& side) {
        os << prefix;
        if (!isRoot) os << DIM << (isLast ? "└── " : "├── ") << RESET;
        if (!side.empty()) os << DIM << side << RESET << " ";
        os << badge(node) << "\n";

        string childPrefix = prefix;
        if (!isRoot) childPrefix += DIM + string(isLast ? "    " : "│   ") + RESET;

        vector<pair<Node*, string>> kids;
        if (node->right != TNULL) kids.push_back({node->right, "R:"});
        if (node->left  != TNULL) kids.push_back({node->left,  "L:"});

        for (size_t i = 0; i < kids.size(); i++)
            printNode(os, kids[i].first, childPrefix, i == kids.size() - 1, false, kids[i].second);
    }

    int countNodes(Node* n) {
        if (n == TNULL) return 0;
        return 1 + countNodes(n->left) + countNodes(n->right);
    }

    int heightRec(Node* n) {
        if (n == TNULL) return 0;
        int l = heightRec(n->left), r = heightRec(n->right);
        return 1 + (l > r ? l : r);
    }

    // Verifie ordre BST, pas de rouge-rouge, meme hauteur noire. Retourne la hauteur noire ou -1.
    int validateRec(Node* n, long long lo, long long hi, size_t& cnt) {
        if (n == TNULL) return 1;
        cnt++;
        if (n->data <= lo || n->data >= hi) return -1;
        if (n->color && (n->left->color || n->right->color)) return -1;
        int l = validateRec(n->left, lo, n->data, cnt);
        if (l < 0) return -1;
        int r = validateRec(n->right, n->data, hi, cnt);
        if (r < 0 || l != r) return -1;
        return l + (n->color ? 0 : 1);
    }

    void destroy(Node* n) {
        if (n == TNULL || n == nullptr) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

public:
    RedBlackTree() {
        TNULL = new Node;
        initializeNULLNode(TNULL, nullptr);
        root = TNULL;
    }

    ~RedBlackTree() {
        destroy(root);
        delete TNULL;
    }

    void insert(int key) {
        Node* node = new Node;
        node->parent = nullptr;
        node->data = key;
        node->left = TNULL;
        node->right = TNULL;
        node->color = true;

        Node* y = nullptr;
        Node* x = root;
        while (x != TNULL) {
            y = x;
            if (node->data < x->data) x = x->left;
            else x = x->right;
        }

        node->parent = y;
        if (y == nullptr) root = node;
        else if (node->data < y->data) y->left = node;
        else y->right = node;

        if (node->parent == nullptr) {
            node->color = false;
            return;
        }
        if (node->parent->parent == nullptr) return;
        fixInsert(node);
    }

    bool deleteNode(int key) {
        Node* z = root;
        Node* x;
        Node* y;

        while (z != TNULL) {
            if (z->data == key) break;
            if (key < z->data) z = z->left;
            else z = z->right;
        }
        if (z == TNULL) return false;

        y = z;
        bool y_original_color = y->color;

        if (z->left == TNULL) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == TNULL) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) x->parent = y;
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        delete z;
        if (y_original_color == false) fixDelete(x);
        return true;
    }

    bool search(int key) {
        Node* temp = root;
        while (temp != TNULL) {
            if (key == temp->data) return true;
            temp = key < temp->data ? temp->left : temp->right;
        }
        return false;
    }

    void display(ostream& os) {
        os << "\n" << CYAN << "┌─ Red-Black Tree ";
        os << "─────────────────────────────" << RESET << "\n";
        if (root == TNULL) {
            os << DIM << "  (empty)" << RESET << "\n";
        } else {
            printNode(os, root, "  ", true, true, "");
            os << "\n  " << RED_BG << " red " << RESET << " "
               << BLK_BG << " black " << RESET
               << DIM << "   nodes: " << countNodes(root)
               << "  height: " << heightRec(root) << RESET << "\n";
        }
        os << CYAN << "└──────────────────────────────────────────────" << RESET << "\n";
    }

    bool empty() const { return root == TNULL; }
    int height() { return heightRec(root); }
    bool validate(size_t expected) {
        if (root == TNULL) return expected == 0;
        if (root->color) return false;
        size_t cnt = 0;
        return validateRec(root, -1, LLONG_MAX, cnt) >= 0 && cnt == expected;
    }
};

// ---------------------------------------------------------------- utilitaires
struct CountingBuf : streambuf {
    unsigned long long count = 0;
    int overflow(int c) override { if (c != EOF) count++; return c; }
    streamsize xsputn(const char*, streamsize n) override { count += (unsigned long long)n; return n; }
};

static double secs(Clock::time_point a, Clock::time_point b) {
    return chrono::duration<double>(b - a).count();
}

static string fmtTime(double s) {
    char b[64];
    if (s >= 1.0)       snprintf(b, sizeof b, "%.3f s", s);
    else if (s >= 1e-3) snprintf(b, sizeof b, "%.3f ms", s * 1e3);
    else if (s >= 1e-6) snprintf(b, sizeof b, "%.3f us", s * 1e6);
    else                snprintf(b, sizeof b, "%.1f ns", s * 1e9);
    return b;
}

static string fmtInt(unsigned long long v) {
    string s = to_string(v), out;
    int c = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        out.push_back(s[i]);
        if (++c % 3 == 0 && i > 0) out.push_back(' ');
    }
    reverse(out.begin(), out.end());
    return out;
}

static string fmtRate(double ops) {
    char b[64];
    if (ops >= 1e6)      snprintf(b, sizeof b, "%.2f Mop/s", ops / 1e6);
    else if (ops >= 1e3) snprintf(b, sizeof b, "%.1f kop/s", ops / 1e3);
    else                 snprintf(b, sizeof b, "%.0f op/s", ops);
    return b;
}

static string fmtBytes(double v) {
    char b[64];
    if (v >= 1073741824.0)   snprintf(b, sizeof b, "%.2f GB", v / 1073741824.0);
    else if (v >= 1048576.0) snprintf(b, sizeof b, "%.2f MB", v / 1048576.0);
    else if (v >= 1024.0)    snprintf(b, sizeof b, "%.2f KB", v / 1024.0);
    else                     snprintf(b, sizeof b, "%.0f B", v);
    return b;
}

struct MemInfo { double total_gb = 0, avail_gb = 0; };

static MemInfo memInfo() {
    MemInfo m;
#if defined(__linux__)
    ifstream f("/proc/meminfo");
    string line;
    while (getline(f, line)) {
        long long v;
        if (sscanf(line.c_str(), "MemTotal: %lld", &v) == 1) m.total_gb = v / 1048576.0;
        else if (sscanf(line.c_str(), "MemAvailable: %lld", &v) == 1) m.avail_gb = v / 1048576.0;
    }
#elif defined(__APPLE__)
    uint64_t mem = 0; size_t len = sizeof(mem);
    if (sysctlbyname("hw.memsize", &mem, &len, nullptr, 0) == 0) {
        m.total_gb = mem / 1073741824.0;
        m.avail_gb = m.total_gb * 0.7; // estimation
    }
#elif defined(_WIN32)
    MEMORYSTATUSEX s; s.dwLength = sizeof(s);
    if (GlobalMemoryStatusEx(&s)) {
        m.total_gb = s.ullTotalPhys / 1073741824.0;
        m.avail_gb = s.ullAvailPhys / 1073741824.0;
    }
#endif
    return m;
}

static string cpuModel() {
#if defined(__linux__)
    ifstream f("/proc/cpuinfo");
    string line;
    while (getline(f, line))
        if (line.rfind("model name", 0) == 0) {
            auto p = line.find(':');
            if (p != string::npos && p + 2 <= line.size()) return line.substr(p + 2);
        }
#endif
    return "inconnu";
}

// ---------------------------------------------------------------- une taille de benchmark
struct Row {
    size_t n = 0;
    int reps = 1;
    bool skipped = false;
    string reason;
    double gen = 0, ins = 0, sea = 0, mis = 0, dis = 0, del = 0; // secondes (moyenne par repetition)
    bool dispDone = false;
    unsigned long long dispBytes = 0;
    int h = 0;
    bool valid = true, correct = true;
};

static Row runSize(size_t n, size_t dispMax, mt19937& rng) {
    Row r;
    r.n = n;
    r.reps = (int)min<size_t>(1000, max<size_t>(1, 1000000 / n));
    const bool live = n >= 1000000;

    auto timed = [&](const char* name, double& acc, auto&& fn) {
        if (live) { cout << "  > " << name << " en cours...\r" << flush; }
        auto a = Clock::now();
        fn();
        auto b = Clock::now();
        acc += secs(a, b);
        if (live) cout << string(50, ' ') << "\r" << flush;
    };

    // generation des cles : permutation aleatoire de 0..n-1
    auto g0 = Clock::now();
    vector<int> keys(n);
    iota(keys.begin(), keys.end(), 0);
    shuffle(keys.begin(), keys.end(), rng);
    r.gen = secs(g0, Clock::now());

    RedBlackTree tree;
    double tIns = 0, tSea = 0, tMis = 0, tDis = 0, tDel = 0;

    for (int rep = 0; rep < r.reps; rep++) {
        // 1. insertion (cles paires)
        timed("insertion", tIns, [&] {
            for (size_t i = 0; i < n; i++) tree.insert(2 * keys[i]);
        });
        if (rep == 0) {
            r.h = tree.height();
            r.valid = tree.validate(n);
        }

        // 2. recherche de cles presentes
        size_t found = 0;
        timed("recherche (presents)", tSea, [&] {
            for (size_t i = 0; i < n; i++) found += tree.search(2 * keys[i]);
        });

        // 3. recherche de cles absentes (impaires, entremelees avec les paires)
        size_t foundMiss = 0;
        timed("recherche (absents)", tMis, [&] {
            for (size_t i = 0; i < n; i++) foundMiss += tree.search(2 * keys[i] + 1);
        });
        if (found != n || foundMiss != 0) r.correct = false;

        // 4. affichage (rendu complet vers un compteur d'octets)
        if (n <= dispMax) {
            CountingBuf cb;
            ostream os(&cb);
            timed("affichage", tDis, [&] { tree.display(os); });
            r.dispBytes = cb.count;
            r.dispDone = true;
        }
        if (rep == 0 && n <= 10) {
            cout << "\n  Apercu reel de l'arbre (n = " << n << ") :";
            tree.display(cout);
        }

        // 5. suppression dans un autre ordre aleatoire
        shuffle(keys.begin(), keys.end(), rng);
        size_t deleted = 0;
        timed("suppression", tDel, [&] {
            for (size_t i = 0; i < n; i++) deleted += tree.deleteNode(2 * keys[i]);
        });
        if (!tree.empty() || deleted != n) r.correct = false;
    }

    r.ins = tIns / r.reps;
    r.sea = tSea / r.reps;
    r.mis = tMis / r.reps;
    r.dis = tDis / r.reps;
    r.del = tDel / r.reps;
    return r;
}

static void printRow(const string& label, double sec, size_t ops, double log2n, bool showLog) {
    double per = sec / (double)ops;
    cout << "  " << left << setw(14) << label << right
         << setw(13) << fmtTime(sec)
         << setw(14) << fmtTime(per)
         << setw(15) << fmtRate(1.0 / per);
    if (showLog) {
        char b[32];
        snprintf(b, sizeof b, "%.1f", per * 1e9 / log2n);
        cout << setw(16) << b;
    } else {
        cout << setw(16) << "-";
    }
    cout << "\n";
}

static void printBlock(const Row& r) {
    double log2n = log2((double)r.n);
    cout << "\n  " << left << setw(14) << "Phase" << right
         << setw(13) << "Temps total" << setw(14) << "Par operation"
         << setw(15) << "Debit" << setw(16) << "ns/op / log2(n)" << "\n";
    cout << "  " << string(72, '-') << "\n";
    printRow("INSERT", r.ins, r.n, log2n, true);
    printRow("RECHERCHE +", r.sea, r.n, log2n, true);
    printRow("RECHERCHE -", r.mis, r.n, log2n, true);
    if (r.dispDone) printRow("AFFICHAGE", r.dis, r.n, log2n, false);
    else cout << "  " << left << setw(14) << "AFFICHAGE" << right << "  ignore (n > limite d'affichage)\n";
    printRow("SUPPRESSION", r.del, r.n, log2n, true);
    cout << "  " << string(72, '-') << "\n";

    cout << "  Hauteur de l'arbre : " << r.h
         << "  (log2(n) = " << fixed << setprecision(1) << log2n
         << ", borne theorique 2*log2(n+1) = " << 2.0 * log2((double)r.n + 1) << ")" << defaultfloat << "\n";
    if (r.dispDone) cout << "  Taille du rendu    : " << fmtBytes((double)r.dispBytes) << "\n";
    cout << "  Generation des cles: " << fmtTime(r.gen) << " (non comptee)\n";
    cout << "  Verification       : "
         << (r.valid ? GREEN + "proprietes Red-Black OK" + RESET : RED + "ARBRE INVALIDE" + RESET) << " | "
         << (r.correct ? GREEN + "recherches/suppressions OK" + RESET : RED + "RESULTATS INCORRECTS" + RESET) << "\n";
}

// pente de la regression lineaire ln(y) = a + p*ln(x)
static double slopeLogLog(const vector<double>& xs, const vector<double>& ys) {
    size_t m = xs.size();
    if (m < 3) return NAN;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (size_t i = 0; i < m; i++) {
        double x = log(xs[i]), y = log(ys[i]);
        sx += x; sy += y; sxx += x * x; sxy += x * y;
    }
    double den = m * sxx - sx * sx;
    return den == 0 ? NAN : (m * sxy - sx * sy) / den;
}

static string verdict(double s, bool nlogn) {
    if (std::isnan(s)) return "pas assez de points";
    if (nlogn) {
        if (s < 0.95) return "sous-lineaire (bruit / surcout fixe)";
        if (s <= 1.30) return "coherent avec O(n log n)";
        return "plus lent que la theorie (effets cache/memoire probables)";
    }
    if (s < 0.90) return "sous-lineaire (bruit / surcout fixe)";
    if (s <= 1.20) return "coherent avec O(n)";
    return "plus lent que la theorie (effets cache/memoire probables)";
}

int main(int argc, char** argv) {
    size_t maxN = 1000000000ULL;
    size_t dispMax = 10000000ULL;
    if (argc > 1) maxN = strtoull(argv[1], nullptr, 10);
    if (argc > 2) dispMax = strtoull(argv[2], nullptr, 10);
    if (maxN < 10) maxN = 10;
    if (maxN > 1000000000ULL) maxN = 1000000000ULL;

    MemInfo mem = memInfo();
    double availGB = mem.avail_gb > 0 ? mem.avail_gb : 8.0;

    // ---- en-tete : infos machine
    cout << CYAN << "╔══════════════════════════════════════════════════════════════╗\n"
         << "║              BENCHMARK  RED-BLACK TREE                       ║\n"
         << "╚══════════════════════════════════════════════════════════════╝" << RESET << "\n";
    cout << "  CPU            : " << cpuModel() << "\n";
    cout << "  Coeurs logiques: " << thread::hardware_concurrency() << "\n";
    if (mem.total_gb > 0)
        cout << "  RAM            : " << fixed << setprecision(1) << mem.total_gb << " GB au total, "
             << mem.avail_gb << " GB disponibles" << defaultfloat << "\n";
    else
        cout << "  RAM            : inconnue (on suppose 8 GB disponibles)\n";
    cout << "  sizeof(Node)   : " << sizeof(Node) << " octets (~48 octets reels avec malloc)\n";
#if defined(__VERSION__)
    cout << "  Compilateur    : " << __VERSION__ << "\n";
#endif
#ifndef __OPTIMIZE__
    cout << YELLOW << "  ATTENTION : compile sans optimisation ! Utilise : g++ -O2 -std=c++17 ..." << RESET << "\n";
#else
    cout << "  Optimisation   : activee (OK)\n";
#endif
    cout << "  Horloge        : steady_clock, resolution "
         << (double)Clock::period::num / Clock::period::den * 1e9 << " ns\n";
    cout << "  Tailles        : 10^1 .. 10^" << (int)round(log10((double)maxN))
         << "  |  affichage chronometre jusqu'a n <= " << fmtInt(dispMax) << "\n";
    if (maxN >= 100000000ULL)
        cout << YELLOW << "  Les tailles >= 10^8 peuvent prendre plusieurs minutes." << RESET << "\n";

    mt19937 rng(12345); // graine fixe : resultats reproductibles
    vector<Row> rows;

    for (size_t n = 10; n <= maxN; n *= 10) {
        cout << "\n" << CYAN << "━━━━━━━━━━ n = " << fmtInt(n) << " ━━━━━━━━━━" << RESET;

        // estimation memoire : ~48 o/noeud (malloc) + 4 o/cle
        double needGB = 52.0 * (double)n / 1073741824.0;
        if (needGB > availGB * 0.85) {
            Row r; r.n = n; r.skipped = true;
            char b[160];
            snprintf(b, sizeof b, "memoire estimee %.1f GB > 85%% des %.1f GB disponibles", needGB, availGB);
            r.reason = b;
            cout << "\n  " << YELLOW << "IGNORE : " << r.reason << RESET << "\n";
            rows.push_back(r);
            continue;
        }

        cout << DIM << "   (memoire estimee : " << fixed << setprecision(2) << needGB << " GB)" << defaultfloat << RESET;
        try {
            Row r = runSize(n, dispMax, rng);
            cout << "\n  " << DIM << "repetitions : " << r.reps << " (moyenne par repetition)" << RESET;
            printBlock(r);
            rows.push_back(r);
        } catch (const bad_alloc&) {
            Row r; r.n = n; r.skipped = true; r.reason = "bad_alloc : memoire insuffisante";
            cout << "\n  " << RED << "ECHEC : " << r.reason << RESET << "\n";
            rows.push_back(r);
        }
    }

    // ---- recapitulatif
    vector<Row> ok;
    for (auto& r : rows) if (!r.skipped) ok.push_back(r);

    cout << "\n\n" << CYAN << "╔══════════════════════════════════════════════════════════════╗\n"
         << "║                        RECAPITULATIF                         ║\n"
         << "╚══════════════════════════════════════════════════════════════╝" << RESET << "\n";

    for (auto& r : rows)
        if (r.skipped) cout << YELLOW << "  n = " << fmtInt(r.n) << " ignore : " << r.reason << RESET << "\n";

    if (ok.empty()) { cout << "Aucune taille executee.\n"; return 0; }

    cout << "\n" << BOLD << " Temps total par phase" << RESET << "\n";
    cout << "  " << right << setw(16) << "n" << setw(13) << "INSERT" << setw(13) << "RECH +"
         << setw(13) << "RECH -" << setw(13) << "AFFICH" << setw(13) << "SUPPR" << "\n";
    cout << "  " << string(81, '-') << "\n";
    for (auto& r : ok)
        cout << "  " << setw(16) << fmtInt(r.n) << setw(13) << fmtTime(r.ins) << setw(13) << fmtTime(r.sea)
             << setw(13) << fmtTime(r.mis) << setw(13) << (r.dispDone ? fmtTime(r.dis) : string("-"))
             << setw(13) << fmtTime(r.del) << "\n";

    cout << "\n" << BOLD << " Cout par operation (ns/op) : la theorie dit que ca croit comme log2(n)" << RESET << "\n";
    cout << "  " << right << setw(16) << "n" << setw(9) << "log2(n)" << setw(11) << "INSERT" << setw(11) << "RECH +"
         << setw(11) << "RECH -" << setw(11) << "AFFICH" << setw(11) << "SUPPR" << "\n";
    cout << "  " << string(80, '-') << "\n";
    for (auto& r : ok) {
        auto ns = [&](double s) { char b[32]; snprintf(b, sizeof b, "%.0f", s / r.n * 1e9); return string(b); };
        char l[16]; snprintf(l, sizeof l, "%.1f", log2((double)r.n));
        cout << "  " << setw(16) << fmtInt(r.n) << setw(9) << l << setw(11) << ns(r.ins) << setw(11) << ns(r.sea)
             << setw(11) << ns(r.mis) << setw(11) << (r.dispDone ? ns(r.dis) : string("-")) << setw(11) << ns(r.del) << "\n";
    }

    if (ok.size() >= 2) {
        cout << "\n" << BOLD << " Croissance du temps total quand n est multiplie par 10" << RESET << "\n";
        cout << DIM << "  (theorie : n log n -> x10 x log(n2)/log(n1) ; affichage O(n) -> x10 exactement)" << RESET << "\n";
        cout << "  " << right << setw(20) << "passage" << setw(11) << "theo nlogn" << setw(10) << "INSERT"
             << setw(10) << "RECH +" << setw(10) << "RECH -" << setw(10) << "SUPPR"
             << setw(10) << "theo n" << setw(10) << "AFFICH" << "\n";
        cout << "  " << string(91, '-') << "\n";
        for (size_t i = 1; i < ok.size(); i++) {
            const Row& a = ok[i - 1]; const Row& b = ok[i];
            double th = ((double)b.n * log2((double)b.n)) / ((double)a.n * log2((double)a.n));
            double thn = (double)b.n / (double)a.n;
            auto f = [](double x) { char s[32]; snprintf(s, sizeof s, "x%.1f", x); return string(s); };
            string pass = fmtInt(a.n) + "->" + fmtInt(b.n);
            cout << "  " << setw(20) << pass << setw(11) << f(th) << setw(10) << f(b.ins / a.ins)
                 << setw(10) << f(b.sea / a.sea) << setw(10) << f(b.mis / a.mis) << setw(10) << f(b.del / a.del)
                 << setw(10) << f(thn)
                 << setw(10) << ((a.dispDone && b.dispDone) ? f(b.dis / a.dis) : string("-")) << "\n";
        }
    }

    cout << "\n" << BOLD << " Hauteur de l'arbre" << RESET << "\n";
    cout << "  " << right << setw(16) << "n" << setw(12) << "mesuree" << setw(12) << "log2(n)" << setw(18) << "borne 2log2(n+1)" << "\n";
    cout << "  " << string(58, '-') << "\n";
    for (auto& r : ok) {
        char l[16], bnd[16];
        snprintf(l, sizeof l, "%.1f", log2((double)r.n));
        snprintf(bnd, sizeof bnd, "%.1f", 2.0 * log2((double)r.n + 1));
        cout << "  " << setw(16) << fmtInt(r.n) << setw(12) << r.h << setw(12) << l << setw(18) << bnd << "\n";
    }

    // ---- complexite pratique : pente log-log (n >= 1000)
    cout << "\n" << BOLD << " Complexite pratique mesuree sur ta machine" << RESET << "\n";
    cout << DIM << "  Pente p de la droite ln(temps) = a + p*ln(n), sur les tailles n >= 1000.\n"
         << "  p = 1 -> lineaire ; O(n log n) donne p legerement > 1 (typiquement 1.05 a 1.20).\n"
         << "  Au-dela, ce sont surtout les defauts de cache qui ralentissent les grands arbres." << RESET << "\n\n";
    struct Ph { const char* name; double Row::*f; bool nlogn; };
    Ph phases[] = {
        {"INSERT", &Row::ins, true}, {"RECHERCHE +", &Row::sea, true}, {"RECHERCHE -", &Row::mis, true},
        {"SUPPRESSION", &Row::del, true}, {"AFFICHAGE", &Row::dis, false}
    };
    for (auto& ph : phases) {
        vector<double> xs, ys;
        for (auto& r : ok) {
            if (r.n < 1000) continue;
            if (!strcmp(ph.name, "AFFICHAGE") && !r.dispDone) continue;
            double t = r.*(ph.f);
            if (t > 0) { xs.push_back((double)r.n); ys.push_back(t); }
        }
        double p = slopeLogLog(xs, ys);
        char b[32];
        if (std::isnan(p)) snprintf(b, sizeof b, "n/a"); else snprintf(b, sizeof b, "%.3f", p);
        cout << "  " << left << setw(14) << ph.name << " pente = " << setw(8) << b
             << " -> " << verdict(p, ph.nlogn) << "\n";
    }

    // ---- complexite theorique
    cout << "\n" << BOLD << " Complexite theorique (arbre Red-Black)" << RESET << "\n";
    cout << "  Operation        1 operation      n operations     Remarque\n";
    cout << "  " << string(76, '-') << "\n";
    cout << "  Insertion        O(log n)         O(n log n)       <= 2 rotations, recoloration O(log n) pire cas\n";
    cout << "  Recherche        O(log n)         O(n log n)       hauteur <= 2*log2(n+1)\n";
    cout << "  Suppression      O(log n)         O(n log n)       <= 3 rotations\n";
    cout << "  Affichage        O(n)             -                chaque noeud affiche une fois\n";
    cout << "  Memoire          O(n)             -                ~48 octets/noeud en pratique\n";

    bool allOk = true;
    for (auto& r : ok) if (!r.valid || !r.correct) allOk = false;
    cout << "\n  " << (allOk ? GREEN + "Toutes les verifications ont reussi." + RESET
                             : RED + "Des verifications ont echoue : voir les blocs ci-dessus." + RESET) << "\n";
    return 0;
}

/*
resultat terminal:
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/REdBlack$ ./benchMarkRB
╔══════════════════════════════════════════════════════════════╗
║              BENCHMARK  RED-BLACK TREE                       ║
╚══════════════════════════════════════════════════════════════╝
  CPU            : 11th Gen Intel(R) Core(TM) i5-1145G7 @ 2.60GHz
  Coeurs logiques: 8
  RAM            : 15.3 GB au total, 2.9 GB disponibles
  sizeof(Node)   : 32 octets (~48 octets reels avec malloc)
  Compilateur    : 15.2.0
  ATTENTION : compile sans optimisation ! Utilise : g++ -O2 -std=c++17 ...
  Horloge        : steady_clock, resolution 1 ns
  Tailles        : 10^1 .. 10^9  |  affichage chronometre jusqu'a n <= 10 000 000
  Les tailles >= 10^8 peuvent prendre plusieurs minutes.

━━━━━━━━━━ n = 10 ━━━━━━━━━━   (memoire estimee : 0.00 GB)
  Apercu reel de l'arbre (n = 10) :
┌─ Red-Black Tree ─────────────────────────────
   12 
  ├── R:  16 
  │   ├── R:  18 
  │   └── L:  14 
  └── L:  6 
      ├── R:  10 
      │   └── L:  8 
      └── L:  2 
          ├── R:  4 
          └── L:  0 

   red   black    nodes: 10  height: 4
└──────────────────────────────────────────────

  repetitions : 1000 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT             847.5 ns       84.8 ns    11.80 Mop/s            25.5
  RECHERCHE +        449.1 ns       44.9 ns    22.27 Mop/s            13.5
  RECHERCHE -        523.3 ns       52.3 ns    19.11 Mop/s            15.8
  AFFICHAGE         25.514 us      2.551 us    391.9 kop/s               -
  SUPPRESSION        868.9 ns       86.9 ns    11.51 Mop/s            26.2
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 4  (log2(n) = 3.3, borne theorique 2*log2(n+1) = 6.9)
  Taille du rendu    : 950 B
  Generation des cles: 8.651 us (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

━━━━━━━━━━ n = 100 ━━━━━━━━━━   (memoire estimee : 0.00 GB)
  repetitions : 1000 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT            13.002 us      130.0 ns     7.69 Mop/s            19.6
  RECHERCHE +        7.486 us       74.9 ns    13.36 Mop/s            11.3
  RECHERCHE -        8.958 us       89.6 ns    11.16 Mop/s            13.5
  AFFICHAGE        267.187 us      2.672 us    374.3 kop/s               -
  SUPPRESSION       12.587 us      125.9 ns     7.94 Mop/s            18.9
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 8  (log2(n) = 6.6, borne theorique 2*log2(n+1) = 13.3)
  Taille du rendu    : 10.20 KB
  Generation des cles: 11.505 us (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

━━━━━━━━━━ n = 1 000 ━━━━━━━━━━   (memoire estimee : 0.00 GB)
  repetitions : 1000 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT           147.513 us      147.5 ns     6.78 Mop/s            14.8
  RECHERCHE +       94.812 us       94.8 ns    10.55 Mop/s             9.5
  RECHERCHE -      110.541 us      110.5 ns     9.05 Mop/s            11.1
  AFFICHAGE          2.345 ms      2.345 us    426.4 kop/s               -
  SUPPRESSION      145.318 us      145.3 ns     6.88 Mop/s            14.6
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 12  (log2(n) = 10.0, borne theorique 2*log2(n+1) = 19.9)
  Taille du rendu    : 143.33 KB
  Generation des cles: 45.054 us (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

━━━━━━━━━━ n = 10 000 ━━━━━━━━━━   (memoire estimee : 0.00 GB)
  repetitions : 100 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT             2.359 ms      235.9 ns     4.24 Mop/s            17.8
  RECHERCHE +        1.706 ms      170.6 ns     5.86 Mop/s            12.8
  RECHERCHE -        1.914 ms      191.4 ns     5.23 Mop/s            14.4
  AFFICHAGE         27.093 ms      2.709 us    369.1 kop/s               -
  SUPPRESSION        2.179 ms      217.9 ns     4.59 Mop/s            16.4
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 16  (log2(n) = 13.3, borne theorique 2*log2(n+1) = 26.6)
  Taille du rendu    : 1.82 MB
  Generation des cles: 718.503 us (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

━━━━━━━━━━ n = 100 000 ━━━━━━━━━━   (memoire estimee : 0.00 GB)
  repetitions : 10 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT            40.312 ms      403.1 ns     2.48 Mop/s            24.3
  RECHERCHE +       31.080 ms      310.8 ns     3.22 Mop/s            18.7
  RECHERCHE -       34.273 ms      342.7 ns     2.92 Mop/s            20.6
  AFFICHAGE        279.024 ms      2.790 us    358.4 kop/s               -
  SUPPRESSION       47.705 ms      477.0 ns     2.10 Mop/s            28.7
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 21  (log2(n) = 16.6, borne theorique 2*log2(n+1) = 33.2)
  Taille du rendu    : 22.58 MB
  Generation des cles: 4.984 ms (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

                                                  imee : 0.05 GB)  > insertion en cours...
  repetitions : 1 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT              1.319 s      1.319 us    758.0 kop/s            66.2
  RECHERCHE +      895.187 ms      895.2 ns     1.12 Mop/s            44.9
  RECHERCHE -         1.000 s      1.000 us    999.9 kop/s            50.2
  AFFICHAGE           3.149 s      3.149 us    317.6 kop/s               -
  SUPPRESSION         1.080 s      1.080 us    925.9 kop/s            54.2
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 24  (log2(n) = 19.9, borne theorique 2*log2(n+1) = 39.9)
  Taille du rendu    : 268.04 MB
  Generation des cles: 67.883 ms (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

                                                  timee : 0.48 GB)  > insertion en cours...
  repetitions : 1 (moyenne par repetition)
  Phase           Temps total Par operation          Debit ns/op / log2(n)
  ------------------------------------------------------------------------
  INSERT             18.404 s      1.840 us    543.4 kop/s            79.1
  RECHERCHE +        12.514 s      1.251 us    799.1 kop/s            53.8
  RECHERCHE -        15.494 s      1.549 us    645.4 kop/s            66.6
  AFFICHAGE          27.209 s      2.721 us    367.5 kop/s               -
  SUPPRESSION        17.619 s      1.762 us    567.6 kop/s            75.8
  ------------------------------------------------------------------------
  Hauteur de l'arbre : 28  (log2(n) = 23.3, borne theorique 2*log2(n+1) = 46.5)
  Taille du rendu    : 3.04 GB
  Generation des cles: 1.321 s (non comptee)
  Verification       : proprietes Red-Black OK | recherches/suppressions OK

━━━━━━━━━━ n = 100 000 000 ━━━━━━━━━━
  IGNORE : memoire estimee 4.8 GB > 85% des 2.9 GB disponibles

━━━━━━━━━━ n = 1 000 000 000 ━━━━━━━━━━
  IGNORE : memoire estimee 48.4 GB > 85% des 2.9 GB disponibles


╔══════════════════════════════════════════════════════════════╗
║                        RECAPITULATIF                         ║
╚══════════════════════════════════════════════════════════════╝
  n = 100 000 000 ignore : memoire estimee 4.8 GB > 85% des 2.9 GB disponibles
  n = 1 000 000 000 ignore : memoire estimee 48.4 GB > 85% des 2.9 GB disponibles

 Temps total par phase
                 n       INSERT       RECH +       RECH -       AFFICH        SUPPR
  ---------------------------------------------------------------------------------
                10     847.5 ns     449.1 ns     523.3 ns    25.514 us     868.9 ns
               100    13.002 us     7.486 us     8.958 us   267.187 us    12.587 us
             1 000   147.513 us    94.812 us   110.541 us     2.345 ms   145.318 us
            10 000     2.359 ms     1.706 ms     1.914 ms    27.093 ms     2.179 ms
           100 000    40.312 ms    31.080 ms    34.273 ms   279.024 ms    47.705 ms
         1 000 000      1.319 s   895.187 ms      1.000 s      3.149 s      1.080 s
        10 000 000     18.404 s     12.514 s     15.494 s     27.209 s     17.619 s

 Cout par operation (ns/op) : la theorie dit que ca croit comme log2(n)
                 n  log2(n)     INSERT     RECH +     RECH -     AFFICH      SUPPR
  --------------------------------------------------------------------------------
                10      3.3         85         45         52       2551         87
               100      6.6        130         75         90       2672        126
             1 000     10.0        148         95        111       2345        145
            10 000     13.3        236        171        191       2709        218
           100 000     16.6        403        311        343       2790        477
         1 000 000     19.9       1319        895       1000       3149       1080
        10 000 000     23.3       1840       1251       1549       2721       1762

 Croissance du temps total quand n est multiplie par 10
  (theorie : n log n -> x10 x log(n2)/log(n1) ; affichage O(n) -> x10 exactement)
               passage theo nlogn    INSERT    RECH +    RECH -     SUPPR    theo n    AFFICH
  -------------------------------------------------------------------------------------------
               10->100      x20.0     x15.3     x16.7     x17.1     x14.5     x10.0     x10.5
            100->1 000      x15.0     x11.3     x12.7     x12.3     x11.5     x10.0      x8.8
         1 000->10 000      x13.3     x16.0     x18.0     x17.3     x15.0     x10.0     x11.6
       10 000->100 000      x12.5     x17.1     x18.2     x17.9     x21.9     x10.0     x10.3
    100 000->1 000 000      x12.0     x32.7     x28.8     x29.2     x22.6     x10.0     x11.3
  1 000 000->10 000 000      x11.7     x14.0     x14.0     x15.5     x16.3     x10.0      x8.6

 Hauteur de l'arbre
                 n     mesuree     log2(n)  borne 2log2(n+1)
  ----------------------------------------------------------
                10           4         3.3               6.9
               100           8         6.6              13.3
             1 000          12        10.0              19.9
            10 000          16        13.3              26.6
           100 000          21        16.6              33.2
         1 000 000          24        19.9              39.9
        10 000 000          28        23.3              46.5

 Complexite pratique mesuree sur ta machine
  Pente p de la droite ln(temps) = a + p*ln(n), sur les tailles n >= 1000.
  p = 1 -> lineaire ; O(n log n) donne p legerement > 1 (typiquement 1.05 a 1.20).
  Au-dela, ce sont surtout les defauts de cache qui ralentissent les grands arbres.

  INSERT         pente = 1.294    -> coherent avec O(n log n)
  RECHERCHE +    pente = 1.296    -> coherent avec O(n log n)
  RECHERCHE -    pente = 1.301    -> plus lent que la theorie (effets cache/memoire probables)
  SUPPRESSION    pente = 1.286    -> coherent avec O(n log n)
  AFFICHAGE      pente = 1.019    -> coherent avec O(n)

 Complexite theorique (arbre Red-Black)
  Operation        1 operation      n operations     Remarque
  ----------------------------------------------------------------------------
  Insertion        O(log n)         O(n log n)       <= 2 rotations, recoloration O(log n) pire cas
  Recherche        O(log n)         O(n log n)       hauteur <= 2*log2(n+1)
  Suppression      O(log n)         O(n log n)       <= 3 rotations
  Affichage        O(n)             -                chaque noeud affiche une fois
  Memoire          O(n)             -                ~48 octets/noeud en pratique

  Toutes les verifications ont reussi.
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/REdBlack$ 
*/