// =============================================================================
//  BRAND-AFFILIATED E-COMMERCE INVENTORY & SEARCH SYSTEM (CS201)
//  Domain  : Hardware & Power Tools
//  Brands  : DeWalt, Makita, Bosch, Stanley   (5 products each = 20 products)
//  Course  : Data Structure and Algorithm Analysis - Final Output
//
//  Lab integration map
//  -------------------------------------------------------------------------
//  Lab 1  Dynamic Allocation   -> class Catalog        (growable new[] array)
//  Lab 2  Doubly Linked List   -> class ShoppingCart   (next/prev navigation)
//  Lab 3  Circular List        -> class BrandRing      (brand banner showcase)
//  Lab 4  Stack (LIFO)         -> class ActionStack    (admin undo / redo)
//  Lab 5  Queue (FIFO)         -> class OrderQueue     (checkout line)
//  Lab 6  BST                  -> class ProductBST     (O(log n) ID lookup)
//  Lab 7  Searching            -> linear keyword search + binary price search
//  Lab 8  Sorting              -> insertionSort() and quickSort()
//
//  Build : g++ -std=c++17 -Wall -Wextra -o inventory main.cpp
//  Run   : ./inventory        (Windows: inventory.exe)
// =============================================================================

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// -----------------------------------------------------------------------------
//  Product model (as required by the specification)
// -----------------------------------------------------------------------------
struct Product {
    int    id            = 0;   // Unique Identifier
    string name;                // Product Title
    string category;            // Category (e.g., Power Tools)
    string brand;               // Official Partner Brand
    double price         = 0.0; // Unit Price (PHP)
    int    stockQuantity = 0;   // Available Inventory Level
};

static const int LOW_STOCK_LEVEL = 5;

// -----------------------------------------------------------------------------
//  Small utilities: string helpers and safe console input
// -----------------------------------------------------------------------------
struct InputClosed {};   // thrown when stdin reaches EOF so we can exit cleanly

static string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

static string toLower(string s) {
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

// 7495.5 -> "7,495.50"
static string formatMoney(double v) {
    long long cents = static_cast<long long>(v * 100.0 + 0.5);
    long long whole = cents / 100;
    int frac = static_cast<int>(cents % 100);
    string digits = to_string(whole), out;
    int count = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        out.insert(out.begin(), digits[i]);
        if (++count % 3 == 0 && i > 0) out.insert(out.begin(), ',');
    }
    char buf[8];
    snprintf(buf, sizeof(buf), ".%02d", frac);
    return out + buf;
}

static string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) throw InputClosed();
    return trim(line);
}

static int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty()) {
            try {
                size_t pos = 0;
                long v = stol(s, &pos);
                if (pos == s.size() && v >= lo && v <= hi) return static_cast<int>(v);
            } catch (...) { /* fall through to the error message */ }
        }
        cout << "  [!] Please enter a whole number from " << lo << " to " << hi << ".\n";
    }
}

static double readDouble(const string& prompt, double lo, double hi) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty()) {
            try {
                size_t pos = 0;
                double v = stod(s, &pos);
                if (pos == s.size() && v >= lo && v <= hi) return v;
            } catch (...) { /* fall through to the error message */ }
        }
        cout << "  [!] Please enter a number from " << formatMoney(lo) << " to "
             << formatMoney(hi) << ".\n";
    }
}

static void pauseScreen() { readLine("\nPress Enter to continue..."); }

// -----------------------------------------------------------------------------
//  Table printing
// -----------------------------------------------------------------------------
static void printRule() {
    const int w[] = {5, 40, 16, 9, 12, 6};
    cout << '+';
    for (int width : w) cout << string(width + 2, '-') << '+';
    cout << '\n';
}

static void printTable(const Product* a, int n) {
    printRule();
    cout << "| " << right << setw(5) << "ID" << " | " << left << setw(40) << "Product"
         << " | " << setw(16) << "Category" << " | " << setw(9) << "Brand"
         << " | " << right << setw(12) << "Price (PHP)" << " | " << setw(6) << "Stock"
         << " |\n";
    printRule();
    long long units = 0;
    double value = 0.0;
    for (int i = 0; i < n; ++i) {
        string name = a[i].name;
        if (name.size() > 40) name = name.substr(0, 37) + "...";
        cout << "| " << right << setw(5) << a[i].id << " | " << left << setw(40) << name
             << " | " << setw(16) << a[i].category << " | " << setw(9) << a[i].brand
             << " | " << right << setw(12) << formatMoney(a[i].price) << " | "
             << setw(6) << a[i].stockQuantity << " |";
        if (a[i].stockQuantity <= LOW_STOCK_LEVEL) cout << "  <- LOW STOCK";
        cout << '\n';
        units += a[i].stockQuantity;
        value += a[i].price * a[i].stockQuantity;
    }
    printRule();
    cout << left << "  " << n << " product(s) | " << units << " unit(s) in stock | stock value: PHP "
         << formatMoney(value) << '\n';
}

// =============================================================================
//  LAB 1 - Dynamic Allocation: growable catalog backed by a raw new[] array
// =============================================================================
class Catalog {
    Product* items_;
    int count_;
    int capacity_;
    int resizes_;

    void grow() {
        int newCapacity = capacity_ * 2;
        Product* bigger = new Product[newCapacity];
        for (int i = 0; i < count_; ++i) bigger[i] = items_[i];
        delete[] items_;                       // release the old block
        items_ = bigger;
        capacity_ = newCapacity;
        ++resizes_;
    }

public:
    explicit Catalog(int initialCapacity = 4)
        : items_(new Product[initialCapacity]), count_(0),
          capacity_(initialCapacity), resizes_(0) {}
    ~Catalog() { delete[] items_; }
    Catalog(const Catalog&) = delete;
    Catalog& operator=(const Catalog&) = delete;

    void add(const Product& p) {
        if (count_ == capacity_) grow();
        items_[count_++] = p;
    }
    int size() const { return count_; }
    int capacity() const { return capacity_; }
    int resizes() const { return resizes_; }
    Product& at(int i) { return items_[i]; }
    const Product& at(int i) const { return items_[i]; }
    const Product* data() const { return items_; }

    // Returns a new[] copy so sorting never disturbs the master catalog.
    // The CALLER must delete[] the returned array.
    Product* cloneArray() const {
        Product* copy = new Product[count_ > 0 ? count_ : 1];
        for (int i = 0; i < count_; ++i) copy[i] = items_[i];
        return copy;
    }
};

// =============================================================================
//  LAB 2 - Doubly Linked List: shopping cart with next / previous navigation
// =============================================================================
class ShoppingCart {
    struct Node {
        int productId;
        int qty;
        Node* prev;
        Node* next;
    };
    Node* head_;
    Node* tail_;
    Node* cursor_;   // the item currently "selected" while navigating
    int size_;

public:
    ShoppingCart() : head_(nullptr), tail_(nullptr), cursor_(nullptr), size_(0) {}
    ~ShoppingCart() { clear(); }
    ShoppingCart(const ShoppingCart&) = delete;
    ShoppingCart& operator=(const ShoppingCart&) = delete;

    void addItem(int productId, int qty) {
        for (Node* p = head_; p; p = p->next) {          // merge duplicates
            if (p->productId == productId) { p->qty += qty; cursor_ = p; return; }
        }
        Node* n = new Node{productId, qty, tail_, nullptr};
        if (tail_) tail_->next = n; else head_ = n;
        tail_ = n;
        cursor_ = n;
        ++size_;
    }

    int qtyOf(int productId) const {
        for (const Node* p = head_; p; p = p->next)
            if (p->productId == productId) return p->qty;
        return 0;
    }

    bool removeCurrent() {
        if (!cursor_) return false;
        Node* dead = cursor_;
        if (dead->prev) dead->prev->next = dead->next; else head_ = dead->next;
        if (dead->next) dead->next->prev = dead->prev; else tail_ = dead->prev;
        cursor_ = dead->next ? dead->next : dead->prev;
        delete dead;
        --size_;
        return true;
    }

    bool moveNext() { if (cursor_ && cursor_->next) { cursor_ = cursor_->next; return true; } return false; }
    bool movePrev() { if (cursor_ && cursor_->prev) { cursor_ = cursor_->prev; return true; } return false; }

    void clear() {
        while (head_) { Node* nx = head_->next; delete head_; head_ = nx; }
        tail_ = cursor_ = nullptr;
        size_ = 0;
    }

    bool isEmpty() const { return size_ == 0; }
    int size() const { return size_; }

    // Visits items head -> tail as f(productId, qty, isCursor)
    template <class F> void forEach(F f) const {
        for (const Node* p = head_; p; p = p->next) f(p->productId, p->qty, p == cursor_);
    }
};

// =============================================================================
//  LAB 3 - Circular Linked List: continuous brand banner showcase
// =============================================================================
class BrandRing {
    struct Node {
        string brand;
        string tagline;
        Node* next;
    };
    Node* tail_;      // tail_->next is always the head, so the list is a ring
    Node* current_;
    int size_;

public:
    BrandRing() : tail_(nullptr), current_(nullptr), size_(0) {}
    ~BrandRing() {
        if (!tail_) return;
        Node* p = tail_->next;
        tail_->next = nullptr;                   // break the ring before deleting
        while (p) { Node* nx = p->next; delete p; p = nx; }
    }
    BrandRing(const BrandRing&) = delete;
    BrandRing& operator=(const BrandRing&) = delete;

    void add(const string& brand, const string& tagline) {
        Node* n = new Node{brand, tagline, nullptr};
        if (!tail_) { n->next = n; tail_ = n; current_ = n; }
        else { n->next = tail_->next; tail_->next = n; tail_ = n; }
        ++size_;
    }
    int size() const { return size_; }
    const string& currentBrand() const { return current_->brand; }
    const string& currentTagline() const { return current_->tagline; }
    void reset() { if (tail_) current_ = tail_->next; }

    // Moves to the next banner; returns true when it wrapped back to the start.
    bool advance() {
        current_ = current_->next;
        return current_ == tail_->next;
    }
    int currentPosition() const {                // 1-based position of current banner
        int pos = 1;
        for (const Node* p = tail_->next; p != current_; p = p->next) ++pos;
        return pos;
    }
    string brandAt(int i) const {                // 0-based
        const Node* p = tail_->next;
        for (; i > 0; --i) p = p->next;
        return p->brand;
    }
};

// =============================================================================
//  LAB 4 - Stack (LIFO): admin undo / redo of inventory changes
// =============================================================================
struct Action {
    enum Type { STOCK, PRICE };
    Type   type = STOCK;
    int    productId = 0;
    double oldValue = 0.0;
    double newValue = 0.0;
    string description;
};

class ActionStack {
    struct Node {
        Action data;
        Node* next;
    };
    Node* top_;
    int size_;

public:
    ActionStack() : top_(nullptr), size_(0) {}
    ~ActionStack() { clear(); }
    ActionStack(const ActionStack&) = delete;
    ActionStack& operator=(const ActionStack&) = delete;

    void push(const Action& a) { top_ = new Node{a, top_}; ++size_; }
    bool pop(Action& out) {
        if (!top_) return false;
        Node* dead = top_;
        out = dead->data;
        top_ = dead->next;
        delete dead;
        --size_;
        return true;
    }
    void clear() { Action tmp; while (pop(tmp)) {} }
    bool isEmpty() const { return top_ == nullptr; }
    int size() const { return size_; }

    template <class F> void forEach(F f) const {  // top -> bottom
        for (const Node* p = top_; p; p = p->next) f(p->data);
    }
};

// =============================================================================
//  LAB 5 - Queue (FIFO): customer checkout and order processing line
// =============================================================================
struct OrderLine {
    int productId;
    int qty;
};

struct Order {
    int    orderId = 0;
    string customer;
    vector<OrderLine> lines;
};

class OrderQueue {
    struct Node {
        Order data;
        Node* next;
    };
    Node* front_;
    Node* rear_;
    int size_;

public:
    OrderQueue() : front_(nullptr), rear_(nullptr), size_(0) {}
    ~OrderQueue() { Order tmp; while (dequeue(tmp)) {} }
    OrderQueue(const OrderQueue&) = delete;
    OrderQueue& operator=(const OrderQueue&) = delete;

    void enqueue(const Order& o) {
        Node* n = new Node{o, nullptr};
        if (rear_) rear_->next = n; else front_ = n;
        rear_ = n;
        ++size_;
    }
    bool dequeue(Order& out) {
        if (!front_) return false;
        Node* dead = front_;
        out = dead->data;
        front_ = dead->next;
        if (!front_) rear_ = nullptr;
        delete dead;
        --size_;
        return true;
    }
    bool isEmpty() const { return size_ == 0; }
    int size() const { return size_; }

    template <class F> void forEach(F f) const {  // front -> rear
        for (const Node* p = front_; p; p = p->next) f(p->data);
    }
};

// =============================================================================
//  LAB 6 - Binary Search Tree: product ID -> catalog index (O(log n) lookup)
// =============================================================================
class ProductBST {
    struct Node {
        int id;
        int index;
        Node* left;
        Node* right;
        Node(int i, int x) : id(i), index(x), left(nullptr), right(nullptr) {}
    };
    Node* root_;
    int size_;

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }
    Node* insertAt(Node* n, int id, int index) {
        if (!n) { ++size_; return new Node(id, index); }
        if (id < n->id) n->left = insertAt(n->left, id, index);
        else if (id > n->id) n->right = insertAt(n->right, id, index);
        return n;                                  // duplicate IDs are ignored
    }
    static int heightOf(const Node* n) {
        return n ? 1 + max(heightOf(n->left), heightOf(n->right)) : 0;
    }

public:
    ProductBST() : root_(nullptr), size_(0) {}
    ~ProductBST() { destroy(root_); }
    ProductBST(const ProductBST&) = delete;
    ProductBST& operator=(const ProductBST&) = delete;

    void insert(int id, int index) { root_ = insertAt(root_, id, index); }

    // Returns the catalog index or -1. `steps` = nodes compared on the way down.
    int find(int id, int& steps) const {
        steps = 0;
        const Node* cur = root_;
        while (cur) {
            ++steps;
            if (id == cur->id) return cur->index;
            cur = (id < cur->id) ? cur->left : cur->right;
        }
        return -1;
    }
    int height() const { return heightOf(root_); }
    int size() const { return size_; }
};

// =============================================================================
//  LAB 8 - Sorting algorithms (operate on a copied Product array)
// =============================================================================
typedef bool (*LessFn)(const Product&, const Product&);
static long gCompareCount = 0;   // comparisons made by the last sort (for analysis)

static bool priceAsc(const Product& a, const Product& b)  { return a.price != b.price ? a.price < b.price : a.id < b.id; }
static bool priceDesc(const Product& a, const Product& b) { return a.price != b.price ? a.price > b.price : a.id < b.id; }
static bool stockAsc(const Product& a, const Product& b)  { return a.stockQuantity != b.stockQuantity ? a.stockQuantity < b.stockQuantity : a.id < b.id; }
static bool stockDesc(const Product& a, const Product& b) { return a.stockQuantity != b.stockQuantity ? a.stockQuantity > b.stockQuantity : a.id < b.id; }
static bool brandAsc(const Product& a, const Product& b)  { return a.brand != b.brand ? a.brand < b.brand : a.name < b.name; }
static bool brandDesc(const Product& a, const Product& b) { return a.brand != b.brand ? a.brand > b.brand : a.name < b.name; }

// Insertion sort: O(n^2) worst case, O(n) when nearly sorted, stable.
static void insertionSort(Product* a, int n, LessFn less) {
    for (int i = 1; i < n; ++i) {
        Product key = a[i];
        int j = i - 1;
        while (j >= 0) {
            ++gCompareCount;
            if (!less(key, a[j])) break;
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}

// Quick sort (Lomuto partition, middle element as pivot): O(n log n) average.
static void quickSort(Product* a, int lo, int hi, LessFn less) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    swap(a[mid], a[hi]);
    const Product pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; ++j) {
        ++gCompareCount;
        if (less(a[j], pivot)) { ++i; swap(a[i], a[j]); }
    }
    swap(a[i + 1], a[hi]);
    quickSort(a, lo, i, less);
    quickSort(a, i + 2, hi, less);
}

// =============================================================================
//  LAB 7 - Binary search helpers (array must be sorted by price ascending)
// =============================================================================
// First index whose price >= target
static int lowerBoundPrice(const Product* a, int n, double target, int& steps) {
    int lo = 0, hi = n;
    while (lo < hi) {
        ++steps;
        int mid = lo + (hi - lo) / 2;
        if (a[mid].price < target) lo = mid + 1; else hi = mid;
    }
    return lo;
}
// First index whose price > target
static int upperBoundPrice(const Product* a, int n, double target, int& steps) {
    int lo = 0, hi = n;
    while (lo < hi) {
        ++steps;
        int mid = lo + (hi - lo) / 2;
        if (a[mid].price <= target) lo = mid + 1; else hi = mid;
    }
    return lo;
}

// =============================================================================
//  The application: owns every data structure and implements the menu
// =============================================================================
class InventorySystem {
    Catalog      catalog;
    ProductBST   index;
    BrandRing    brands;
    ShoppingCart cart;
    OrderQueue   orders;
    ActionStack  undoStack;
    ActionStack  redoStack;
    int          nextOrderId;

    // ---------------------------------------------------------------- setup
    void loadSampleData() {
        const Product seed[] = {
            // ---- DeWalt ----
            {101, "20V Cordless Drill/Driver (DCD777)",     "Power Tools",    "DeWalt",  7495.00, 15},
            {102, "4.5in Angle Grinder (DWE402)",           "Power Tools",    "DeWalt",  3850.00, 22},
            {103, "20V Circular Saw 6-1/2in (DCS391)",      "Power Tools",    "DeWalt",  9200.00,  8},
            {104, "Sliding Compound Miter Saw 12in (DWS780)","Power Tools",   "DeWalt", 32500.00,  4},
            {105, "20V Impact Driver (DCF887)",             "Power Tools",    "DeWalt",  8350.00, 12},
            // ---- Makita ----
            {106, "24mm SDS-Plus Rotary Hammer (HR2470)",   "Power Tools",    "Makita", 10990.00,  9},
            {107, "10mm (3/8in) Corded Drill (M0600B)",     "Power Tools",    "Makita",  2350.00, 30},
            {108, "4in Angle Grinder (9553NB)",             "Power Tools",    "Makita",  3150.00, 25},
            {109, "Orbital Jigsaw (4329)",                  "Power Tools",    "Makita",  5450.00, 11},
            {110, "18V LXT Cordless Impact Driver (DTD153)","Power Tools",    "Makita",  7980.00, 14},
            // ---- Bosch ----
            {111, "Professional Impact Drill (GSB 550)",    "Power Tools",    "Bosch",   2799.00, 20},
            {112, "Professional Angle Grinder (GWS 060)",   "Power Tools",    "Bosch",   2450.00, 26},
            {113, "Laser Distance Measure 40m (GLM 40)",    "Measuring Tools","Bosch",   4490.00, 10},
            {114, "Professional Circular Saw (GKS 190)",    "Power Tools",    "Bosch",   7350.00,  7},
            {115, "Professional Jigsaw (GST 650)",          "Power Tools",    "Bosch",   3990.00, 13},
            // ---- Stanley ----
            {116, "FatMax Tape Measure 25ft",               "Measuring Tools","Stanley",  650.00, 60},
            {117, "16oz Claw Hammer",                       "Hand Tools",     "Stanley",  520.00, 45},
            {118, "Adjustable Wrench 10in",                 "Hand Tools",     "Stanley",  780.00, 38},
            {119, "6-Piece Screwdriver Set",                "Hand Tools",     "Stanley",  950.00,  3},
            {120, "Heavy-Duty Toolbox 19in",                "Hardware",       "Stanley", 1650.00, 16}
        };
        const int n = static_cast<int>(sizeof(seed) / sizeof(seed[0]));
        for (int i = 0; i < n; ++i) catalog.add(seed[i]);    // triggers Lab 1 resizing

        // Lab 6: insert IDs median-first so the BST stays balanced (not a linked list).
        vector<pair<int, int>> byId;
        for (int i = 0; i < catalog.size(); ++i) byId.push_back({catalog.at(i).id, i});
        sort(byId.begin(), byId.end());
        buildBalanced(byId, 0, static_cast<int>(byId.size()) - 1);

        // Lab 3: one banner per partner brand
        brands.add("DeWalt",  "Built for the toughest job sites.");
        brands.add("Makita",  "Cordless freedom, all-day power.");
        brands.add("Bosch",   "Precision engineering you can trust.");
        brands.add("Stanley", "Hand tools made to last a lifetime.");

        cout << "  [Lab 1] Loaded " << catalog.size() << " products | array capacity "
             << catalog.capacity() << " | resized " << catalog.resizes() << " time(s)\n"
             << "  [Lab 6] BST index built: " << index.size() << " nodes, height "
             << index.height() << "\n"
             << "  [Lab 3] " << brands.size() << " partner brands in the banner ring\n";
    }

    void buildBalanced(const vector<pair<int, int>>& v, int lo, int hi) {
        if (lo > hi) return;
        int mid = lo + (hi - lo) / 2;
        index.insert(v[mid].first, v[mid].second);
        buildBalanced(v, lo, mid - 1);
        buildBalanced(v, mid + 1, hi);
    }

    // -------------------------------------------------------------- helpers
    Product* lookup(int id, int& steps) {
        int idx = index.find(id, steps);
        return idx < 0 ? nullptr : &catalog.at(idx);
    }

    // Prompts for a product ID and finds it through the BST (Lab 6).
    Product* askProduct(const string& prompt) {
        int id = readInt(prompt + " (0 = cancel): ", 0, 999999);
        if (id == 0) return nullptr;
        int steps = 0;
        Product* p = lookup(id, steps);
        if (!p) {
            cout << "  [!] No product with ID " << id << " (BST compared " << steps << " node(s)).\n";
            return nullptr;
        }
        cout << "  [Lab 6] BST found ID " << id << " in " << steps << " comparison(s): "
             << p->name << " (" << p->brand << ")\n";
        return p;
    }

    void record(const Action& a) {
        undoStack.push(a);
        redoStack.clear();          // a new action invalidates the redo history
    }

    void applyAction(const Action& a, bool useOld) {
        int steps = 0;
        Product* p = lookup(a.productId, steps);
        if (!p) return;
        double v = useOld ? a.oldValue : a.newValue;
        if (a.type == Action::STOCK) p->stockQuantity = static_cast<int>(v + 0.5);
        else p->price = v;
    }

    // ------------------------------------------------- 1. view whole catalog
    void viewCatalog() {
        cout << "\n--- COMPLETE CATALOG (" << catalog.size() << " products, "
             << brands.size() << " partner brands) ---\n";
        printTable(catalog.data(), catalog.size());
    }

    // --------------------------------------------------- 2. filter by brand
    void filterByBrand() {
        cout << "\n--- FILTER BY OFFICIAL BRAND ---\n";
        for (int i = 0; i < brands.size(); ++i)
            cout << "  " << (i + 1) << ") " << brands.brandAt(i) << '\n';
        int choice = readInt("Select brand (0 = back): ", 0, brands.size());
        if (choice == 0) return;
        string brand = brands.brandAt(choice - 1);

        vector<Product> hits;
        for (int i = 0; i < catalog.size(); ++i)
            if (catalog.at(i).brand == brand) hits.push_back(catalog.at(i));
        cout << "\nProducts by " << brand << ":\n";
        printTable(hits.data(), static_cast<int>(hits.size()));
    }

    // ----------------------------------- 3. keyword search (Lab 7: linear)
    void keywordSearch() {
        cout << "\n--- KEYWORD SEARCH (Linear Search) ---\n";
        string key = toLower(readLine("Keyword (name, category or brand): "));
        if (key.empty()) { cout << "  [!] Keyword cannot be empty.\n"; return; }

        vector<Product> hits;
        int comparisons = 0;
        for (int i = 0; i < catalog.size(); ++i) {           // check every product
            ++comparisons;
            const Product& p = catalog.at(i);
            if (toLower(p.name).find(key) != string::npos ||
                toLower(p.category).find(key) != string::npos ||
                toLower(p.brand).find(key) != string::npos)
                hits.push_back(p);
        }
        if (hits.empty()) cout << "  No products match \"" << key << "\".\n";
        else printTable(hits.data(), static_cast<int>(hits.size()));
        cout << "  [Lab 7] Linear search examined " << comparisons << " item(s) - O(n).\n";
    }

    // --------------------------------- 4. price range (Lab 7: binary search)
    void priceRangeFilter() {
        cout << "\n--- FILTER BY PRICE RANGE (Binary Search) ---\n";
        double lo = readDouble("Minimum price (PHP): ", 0.0, 10000000.0);
        double hi = readDouble("Maximum price (PHP): ", 0.0, 10000000.0);
        if (lo > hi) { swap(lo, hi); cout << "  (min and max were swapped)\n"; }

        int n = catalog.size();
        Product* sorted = catalog.cloneArray();               // new[]
        gCompareCount = 0;
        quickSort(sorted, 0, n - 1, priceAsc);                // binary search needs sorted data

        int steps = 0;
        int first = lowerBoundPrice(sorted, n, lo, steps);
        int last = upperBoundPrice(sorted, n, hi, steps);     // one past the final match
        int found = last - first;

        if (found <= 0) cout << "  No products between PHP " << formatMoney(lo)
                             << " and PHP " << formatMoney(hi) << ".\n";
        else printTable(sorted + first, found);
        cout << "  [Lab 7] Binary search took " << steps << " step(s) - O(log n) "
             << "(after a one-time quick sort of " << n << " items).\n";
        delete[] sorted;                                      // free the temporary copy
    }

    // --------------------------------------------- 5. cart (Lab 2: DLL)
    void printCart() {
        if (cart.isEmpty()) { cout << "  Your cart is empty.\n"; return; }
        double total = 0.0;
        int pos = 0, count = cart.size();
        cout << "\n  Shopping cart (" << count << " item type(s)):\n";
        cart.forEach([&](int id, int qty, bool isCursor) {
            int steps = 0;
            const Product* p = lookup(id, steps);
            if (!p) return;
            double sub = p->price * qty;
            total += sub;
            ++pos;
            cout << (isCursor ? "  > " : "    ") << "[" << pos << "/" << count << "] "
                 << p->name << " (" << p->brand << ")  " << qty << " x "
                 << formatMoney(p->price) << " = " << formatMoney(sub) << '\n';
        });
        cout << "    TOTAL: PHP " << formatMoney(total) << '\n';
    }

    void navigateCart() {
        if (cart.isEmpty()) { cout << "  Your cart is empty.\n"; return; }
        while (!cart.isEmpty()) {
            printCart();
            string cmd = toLower(readLine("  [N]ext  [P]revious  [R]emove current  [Q]uit view > "));
            char c = cmd.empty() ? ' ' : cmd[0];
            if (c == 'n') { if (!cart.moveNext()) cout << "  (already at the last item)\n"; }
            else if (c == 'p') { if (!cart.movePrev()) cout << "  (already at the first item)\n"; }
            else if (c == 'r') { cart.removeCurrent(); cout << "  Item removed from cart.\n"; }
            else if (c == 'q') return;
            else cout << "  [!] Unknown command.\n";
        }
        cout << "  Your cart is now empty.\n";
    }

    void addToCart() {
        printTable(catalog.data(), catalog.size());
        Product* p = askProduct("Enter Product ID to add");
        if (!p) return;
        int inCart = cart.qtyOf(p->id);
        int available = p->stockQuantity - inCart;
        if (available <= 0) {
            cout << "  [!] No more units available (stock " << p->stockQuantity
                 << ", already in cart " << inCart << ").\n";
            return;
        }
        int q = readInt("Quantity (1-" + to_string(available) + ", 0 = cancel): ", 0, available);
        if (q == 0) return;
        cart.addItem(p->id, q);
        cout << "  Added " << q << " x " << p->name << " to your cart.\n";
    }

    void placeOrder() {
        if (cart.isEmpty()) { cout << "  [!] Your cart is empty - add items first.\n"; return; }
        string name = readLine("Customer name: ");
        if (name.empty()) name = "Walk-in Customer";

        Order o;
        o.orderId = nextOrderId++;
        o.customer = name;
        cart.forEach([&](int id, int qty, bool) { o.lines.push_back({id, qty}); });
        orders.enqueue(o);
        cart.clear();
        cout << "  Order #" << o.orderId << " placed. Position in checkout queue: "
             << orders.size() << " (FIFO).\n";
    }

    void shoppingCartMenu() {
        while (true) {
            cout << "\n--- SHOPPING CART (Doubly Linked List) --- items: " << cart.size() << '\n'
                 << "  1) Add item by Product ID\n"
                 << "  2) View / navigate cart (next, previous, remove)\n"
                 << "  3) Place order (send cart to checkout queue)\n"
                 << "  0) Back to main menu\n";
            int c = readInt("Choice: ", 0, 3);
            if (c == 0) return;
            if (c == 1) addToCart();
            else if (c == 2) navigateCart();
            else placeOrder();
        }
    }

    // -------------------------------------- 6. checkout queue (Lab 5: FIFO)
    void fulfilOrder(const Order& o) {
        cout << "\n  [Lab 5] Dequeued order #" << o.orderId << " for " << o.customer << '\n';

        bool ok = true;
        for (const OrderLine& line : o.lines) {
            int steps = 0;
            const Product* p = lookup(line.productId, steps);
            if (!p || p->stockQuantity < line.qty) {
                ok = false;
                cout << "    [!] Not enough stock for "
                     << (p ? p->name : "product " + to_string(line.productId))
                     << " (need " << line.qty << ", have " << (p ? p->stockQuantity : 0) << ")\n";
            }
        }
        if (!ok) { cout << "    ORDER #" << o.orderId << " REJECTED - inventory unchanged.\n"; return; }

        double total = 0.0;
        cout << "    ------ RECEIPT #" << o.orderId << " ------\n";
        for (const OrderLine& line : o.lines) {
            int steps = 0;
            Product* p = lookup(line.productId, steps);
            Action a;
            a.type = Action::STOCK;
            a.productId = p->id;
            a.oldValue = p->stockQuantity;
            a.newValue = p->stockQuantity - line.qty;
            ostringstream d;
            d << "Order #" << o.orderId << ": " << p->name << " stock " << p->stockQuantity
              << " -> " << (p->stockQuantity - line.qty);
            a.description = d.str();
            p->stockQuantity -= line.qty;                       // update inventory
            record(a);                                          // so admin can undo it

            double sub = p->price * line.qty;
            total += sub;
            cout << "    " << line.qty << " x " << p->name << " @ " << formatMoney(p->price)
                 << " = " << formatMoney(sub) << '\n';
        }
        cout << "    TOTAL PAID: PHP " << formatMoney(total) << "  - ORDER FULFILLED\n";
    }

    void processCheckout() {
        cout << "\n--- CHECKOUT QUEUE (FIFO) --- pending orders: " << orders.size() << '\n';
        if (orders.isEmpty()) { cout << "  No orders waiting. Use menu 5 to place one.\n"; return; }

        int pos = 0;
        orders.forEach([&](const Order& o) {
            cout << "  " << ++pos << ". Order #" << o.orderId << " - " << o.customer
                 << " (" << o.lines.size() << " item type(s))\n";
        });
        int c = readInt("1) Process next order  2) Process ALL  0) Back : ", 0, 2);
        if (c == 0) return;
        Order o;
        if (c == 1) { if (orders.dequeue(o)) fulfilOrder(o); }
        else while (orders.dequeue(o)) fulfilOrder(o);
    }

    // ------------------------------------- 7. admin undo / redo (Lab 4: stack)
    void restock() {
        Product* p = askProduct("Product ID to restock");
        if (!p) return;
        int q = readInt("Units to add (1-10000, 0 = cancel): ", 0, 10000);
        if (q == 0) return;
        Action a;
        a.type = Action::STOCK;
        a.productId = p->id;
        a.oldValue = p->stockQuantity;
        a.newValue = p->stockQuantity + q;
        ostringstream d;
        d << "Restock " << p->name << ": " << p->stockQuantity << " -> " << (p->stockQuantity + q);
        a.description = d.str();
        p->stockQuantity += q;
        record(a);
        cout << "  Done. " << a.description << '\n';
    }

    void changePrice() {
        Product* p = askProduct("Product ID to reprice");
        if (!p) return;
        double np = readDouble("New price in PHP (0 = cancel): ", 0.0, 10000000.0);
        if (np == 0.0) return;
        Action a;
        a.type = Action::PRICE;
        a.productId = p->id;
        a.oldValue = p->price;
        a.newValue = np;
        ostringstream d;
        d << "Price " << p->name << ": " << formatMoney(p->price) << " -> " << formatMoney(np);
        a.description = d.str();
        p->price = np;
        record(a);
        cout << "  Done. " << a.description << '\n';
    }

    void undo() {
        Action a;
        if (!undoStack.pop(a)) { cout << "  Nothing to undo.\n"; return; }
        applyAction(a, true);
        redoStack.push(a);
        cout << "  [Lab 4] UNDO (stack pop): " << a.description << '\n';
    }

    void redo() {
        Action a;
        if (!redoStack.pop(a)) { cout << "  Nothing to redo.\n"; return; }
        applyAction(a, false);
        undoStack.push(a);
        cout << "  [Lab 4] REDO: " << a.description << '\n';
    }

    void showHistory() {
        cout << "  Undo stack (" << undoStack.size() << ", newest first):\n";
        if (undoStack.isEmpty()) cout << "    (empty)\n";
        undoStack.forEach([](const Action& a) { cout << "    - " << a.description << '\n'; });
        cout << "  Redo stack: " << redoStack.size() << " action(s) available\n";
    }

    void adminMenu() {
        while (true) {
            cout << "\n--- ADMIN PANEL (LIFO Undo / Redo Stack) ---\n"
                 << "  1) Restock a product\n"
                 << "  2) Change a product price\n"
                 << "  3) UNDO last inventory action (stack pop)\n"
                 << "  4) REDO last undone action\n"
                 << "  5) View action history\n"
                 << "  0) Back to main menu\n";
            int c = readInt("Choice: ", 0, 5);
            if (c == 0) return;
            if (c == 1) restock();
            else if (c == 2) changePrice();
            else if (c == 3) undo();
            else if (c == 4) redo();
            else showHistory();
        }
    }

    // --------------------------------- 8. brand showcase (Lab 3: circular)
    void brandShowcase() {
        cout << "\n--- PARTNER BRAND BANNER SHOWCASE (Circular List) ---\n";
        brands.reset();
        while (true) {
            const string brand = brands.currentBrand();
            int models = 0, units = 0;
            for (int i = 0; i < catalog.size(); ++i) {
                if (catalog.at(i).brand == brand) { ++models; units += catalog.at(i).stockQuantity; }
            }
            string bar(46, '=');
            cout << "\n  " << bar << "\n"
                 << "   OFFICIAL PARTNER  " << brands.currentPosition() << "/" << brands.size() << "\n"
                 << "   " << brand << "\n"
                 << "   \"" << brands.currentTagline() << "\"\n"
                 << "   " << models << " products | " << units << " units in stock\n"
                 << "  " << bar << "\n";
            string cmd = toLower(readLine("  [N / Enter] Next banner   [Q] Back to menu > "));
            char c = cmd.empty() ? 'n' : cmd[0];
            if (c == 'n') {
                if (brands.advance())
                    cout << "  ~ End of the ring: wrapping back to the first brand ~\n";
            } else if (c == 'q') return;
            else cout << "  [!] Unknown command.\n";
        }
    }

    // ----------------------------------------- 9. sorted view (Lab 8)
    void sortInventory() {
        cout << "\n--- SORT INVENTORY DISPLAY ---\n";
        int field = readInt("Sort by  1) Price  2) Stock  3) Brand : ", 1, 3);
        int dir = readInt("Order    1) Ascending  2) Descending : ", 1, 2);
        int algo = readInt("Algorithm 1) Insertion Sort  2) Quick Sort : ", 1, 2);

        const LessFn table[3][2] = {{priceAsc, priceDesc}, {stockAsc, stockDesc}, {brandAsc, brandDesc}};
        LessFn cmp = table[field - 1][dir - 1];

        int n = catalog.size();
        Product* copy = catalog.cloneArray();                 // sort a copy, keep master order
        gCompareCount = 0;
        if (algo == 1) insertionSort(copy, n, cmp);
        else quickSort(copy, 0, n - 1, cmp);

        printTable(copy, n);
        cout << "  [Lab 8] " << (algo == 1 ? "Insertion Sort" : "Quick Sort") << " used "
             << gCompareCount << " comparison(s) on " << n << " items.\n";
        delete[] copy;
    }

    // ------------------------------------------------------------------ menu
    void printMenu() const {
        const string bar(66, '=');
        cout << "\n" << bar << "\n"
             << "  BRAND-AFFILIATED E-COMMERCE INVENTORY & SEARCH SYSTEM (CS201)\n"
             << "  Hardware & Power Tools Edition\n"
             << bar << "\n"
             << "1. View Complete Catalog (Tabular View with Brand Affiliations)\n"
             << "2. Filter Products by Official Brand (DeWalt, Makita, Bosch, Stanley)\n"
             << "3. Search Products by Keyword (Linear Search)\n"
             << "4. Filter Products by Price Range (Binary Search)\n"
             << "5. Add Item to Shopping Cart (Doubly Linked List Navigation)\n"
             << "6. Process Customer Checkout Queue (FIFO Order Queue)\n"
             << "7. Perform Admin Undo Action (LIFO Stack Pop)\n"
             << "8. Display Partner Brand Banner Showcase (Circular List Cycle)\n"
             << "9. Sort Inventory Display (By Price, Stock, or Brand)\n"
             << "10. Exit\n"
             << bar << "\n";
    }

public:
    InventorySystem() : nextOrderId(1001) {
        cout << "Starting up...\n";
        loadSampleData();
    }

    void run() {
        while (true) {
            printMenu();
            int choice = readInt("Enter your choice (1-10): ", 1, 10);
            switch (choice) {
                case 1:  viewCatalog();      break;
                case 2:  filterByBrand();    break;
                case 3:  keywordSearch();    break;
                case 4:  priceRangeFilter(); break;
                case 5:  shoppingCartMenu(); break;
                case 6:  processCheckout();  break;
                case 7:  adminMenu();        break;
                case 8:  brandShowcase();    break;
                case 9:  sortInventory();    break;
                case 10:
                    cout << "\nThank you for using the system. Releasing all dynamic memory...\n";
                    return;
            }
            if (choice != 5 && choice != 7 && choice != 8) pauseScreen();
        }
    }
};

int main() {
    try {
        InventorySystem system;   // destructors free every node/array on scope exit
        system.run();
    } catch (const InputClosed&) {
        cout << "\n[Input closed] Exiting...\n";
    }
    return 0;
}
