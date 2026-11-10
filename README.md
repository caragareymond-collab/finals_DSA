# Brand-Affiliated E-Commerce Inventory & Search System (CS201)

**Domain:** Hardware & Power Tools  
**Partner brands:** DeWalt, Makita, Bosch, Stanley (5 products each, **20 products** total)  
**Language:** C++17, console application, standard library only  
**Course output:** Data Structure and Algorithm Analysis – Final Output

---

## 1. Build and Run

```bash
g++ -std=c++17 -Wall -Wextra -o inventory main.cpp
./inventory            # Windows: inventory.exe
```

Optional memory check (Linux/macOS):

```bash
g++ -std=c++17 -g -fsanitize=address,undefined -o inventory_asan main.cpp
./inventory_asan
```

---

## 2. Product Model

```cpp
struct Product {
    int    id;             // Unique Identifier
    string name;           // Product Title
    string category;       // Power Tools / Hand Tools / Measuring Tools / Hardware
    string brand;          // Official Partner Brand
    double price;          // Unit Price (PHP)
    int    stockQuantity;  // Available Inventory Level
};
```

### Catalog (20 products)

| ID  |              Product                    |     Category    |  Brand  | Price (PHP) | Stock |
|-----|-----------------------------------------|-----------------|---------|------------:|------:|
| 101 | 20V Cordless Drill/Driver (DCD777)      |   Power Tools   | DeWalt  |   7,495.00  |   15  |
| 102 | 4.5in Angle Grinder (DWE402)            |   Power Tools   | DeWalt  |   3,850.00  |   22  |
| 103 | 20V Circular Saw 6-1/2in (DCS391)       |   Power Tools   | DeWalt  |   9,200.00  |    8  |
| 104 | Sliding Compound Miter Saw 12in (DWS780)|   Power Tools   | DeWalt  |   32,500.00 |    4  |
| 105 | 20V Impact Driver (DCF887)              |   Power Tools   | DeWalt  |   8,350.00  |   12  |
| 106 | 24mm SDS-Plus Rotary Hammer (HR2470)    |   Power Tools   | Makita  |   10,990.00 |    9  |
| 107 | 10mm (3/8in) Corded Drill (M0600B)      |   Power Tools   | Makita  |   2,350.00  |   30  |
| 108 | 4in Angle Grinder (9553NB)              |   Power Tools   | Makita  |   3,150.00  |   25  |
| 109 | Orbital Jigsaw (4329)                   |   Power Tools   | Makita  |   5,450.00  |   11  |
| 110 | 18V LXT Cordless Impact Driver (DTD153) |   Power Tools   | Makita  |   7,980.00  |   14  |
| 111 | Professional Impact Drill (GSB 550)     |   Power Tools   | Bosch   |   2,799.00  |   20  |
| 112 | Professional Angle Grinder (GWS 060)    |   Power Tools   | Bosch   |   2,450.00  |   26  |
| 113 | Laser Distance Measure 40m (GLM 40)     | Measuring Tools | Bosch   |   4,490.00  |   10  |
| 114 | Professional Circular Saw (GKS 190)     |   Power Tools   | Bosch   |   7,350.00  |    7  |
| 115 | Professional Jigsaw (GST 650)           |   Power Tools   | Bosch   |   3,990.00  |   13  |
| 116 | FatMax Tape Measure 25ft                | Measuring Tools | Stanley |     650.00  |   60  |
| 117 | 16oz Claw Hammer                        |   Hand Tools    | Stanley |     520.00  |   45  |
| 118 | Adjustable Wrench 10in                  |   Hand Tools    | Stanley |     780.00  |   38  |
| 119 | 6-Piece Screwdriver Set                 |   Hand Tools    | Stanley |     950.00  |    3  |
| 120 | Heavy-Duty Toolbox 19in                 |   Hardware      | Stanley |   1,650.00  |   16  |

> Prices and stock levels are **sample data** for demonstration. Items at 5 units or fewer are flagged `<- LOW STOCK`.

---

## 3. Lab 1–8 Integration

| Lab | Concept | Class / Function in `main.cpp` | Used for |
| --- | ------- | ------------------------------ | -------- |
| 1 | Dynamic Allocation | `Catalog` | Product array on the heap (`new[]`), starts at capacity 4 and **doubles** when full (4→8→16→32); `delete[]` in the destructor |
| 2 | Doubly Linked List | `ShoppingCart` | Shopping cart with **next / previous** cursor navigation, remove-current, duplicate merging |
| 3 | Circular Linked List | `BrandRing` | Continuous partner-brand banner showcase; last node points back to the first |
| 4 | Stack (LIFO) | `ActionStack` | Admin **undo / redo** of stock and price changes (two stacks) |
| 5 | Queue (FIFO) | `OrderQueue` | Customer checkout line; orders are processed oldest-first |
| 6 | Binary Search Tree | `ProductBST` | Product-ID lookup in O(log n); built median-first so it stays balanced (height 5 for 20 nodes) |
| 7 | Searching | `keywordSearch()`, `lowerBoundPrice()`, `upperBoundPrice()` | **Linear** keyword match vs. **binary** price-range filter |
| 8 | Sorting | `insertionSort()`, `quickSort()` | Sort by price, stock, or brand, ascending or descending |

---

## 4. Menu Guide

| # | Menu item | What it does |
| - | --------- | ------------ |
| 1 | View Complete Catalog | Formatted table with brand, price, stock, total units and stock value |
| 2 | Filter by Official Brand | Pick a brand (listed from the circular brand list) and show its products |
| 3 | Search by Keyword | Case-insensitive linear search across name, category and brand; shows items examined |
| 4 | Filter by Price Range | Quick-sorts a copy by price, then binary-searches the lower and upper bounds; shows steps taken |
| 5 | Shopping Cart | Add by ID (found via BST), navigate/remove items, and place an order into the queue |
| 6 | Process Checkout Queue | Dequeue and fulfil the next order, or all orders; prints receipts and deducts stock |
| 7 | Admin Panel (Undo) | Restock, change price, **undo (stack pop)**, **redo**, view history |
| 8 | Brand Banner Showcase | Cycle through partner banners; wraps from the last brand back to the first |
| 9 | Sort Inventory Display | Choose field, direction and algorithm; shows the number of comparisons |
| 10 | Exit | Frees all dynamic memory and quits |

**Typical demo flow:** `5` → add items → place order → `6` process it → `7` undo the stock deduction → `1` confirm stock is restored.

---

## 5. Complexity Summary

| Operation | Structure / Algorithm | Time |
| --------- | --------------------- | ---- |
| Find product by ID | BST (balanced) | O(log n) |
| Keyword search | Linear scan | O(n) |
| Price-range filter | Quick sort once, then 2 binary searches | O(n log n) + O(log n) |
| Add to cart | DLL append / merge check | O(n) merge check, O(1) append |
| Enqueue / dequeue order | Linked queue | O(1) |
| Push / pop undo action | Linked stack | O(1) |
| Next brand banner | Circular list | O(1) |
| Sort (price/stock) | Quick sort | O(n log n) average |
| Sort (brand) | Insertion sort | O(n²) worst, stable |
| Catalog growth | Doubling array | O(1) amortised per add |

---

## 6. Memory Management and Safety

- Every linked structure (`ShoppingCart`, `BrandRing`, `ActionStack`, `OrderQueue`, `ProductBST`) frees all of its nodes in its destructor.
- `BrandRing` breaks the circular link before deleting so the loop terminates.
- `Catalog` uses `new[]` / `delete[]`; its `cloneArray()` returns a heap copy that the caller `delete[]`s right after use (sorting and price filtering never alter the master catalog order).
- Copy constructors and assignment are deleted on all owning classes to prevent double-free.
- Input is validated: non-numeric, out-of-range and empty input are rejected and re-prompted; end-of-input (Ctrl+D / Ctrl+Z) exits cleanly with destructors running.
- Edge cases handled: empty cart, empty queue, nothing to undo/redo, adding more than available stock, unknown product ID, min price greater than max price, no keyword matches, and orders that become unfulfillable because stock changed (rejected without changing inventory).
- Tested with AddressSanitizer, UndefinedBehaviorSanitizer and LeakSanitizer: no errors, no leaks.

---

## 7. Design Notes and Assumptions

- **Stock is deducted at processing time, not when items are added to the cart.** If stock has dropped by the time an order is processed, the whole order is rejected and inventory is left unchanged.
- **Checkout deductions are recorded on the undo stack**, so an admin can reverse them.
- **Sorting works on a copy** because the BST stores catalog indexes; reordering the master array would invalidate it.
- **Currency** is shown as `PHP` in plain ASCII so the table aligns in any terminal.

---

## 8. Files

| File        | Description                                          |
| ----------- | ---------------------------------------------------- |
| `main.cpp`  | Complete source code (single file, sectioned by lab) |
| `README.md` | This document                                        |