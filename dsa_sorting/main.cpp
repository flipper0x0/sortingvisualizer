#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <cmath>

using namespace std;

// Configuration
const int W = 1500;        // Screen width
const int H = 700;         // Screen height
const int O_BAR_W = 29;
const int BAR_S = 2;
const int BAR_W = O_BAR_W - BAR_S;
const int A_SIZE = 40;
const int MENU_H = 110;
const int BTN_R = 8;
const int F_SIZE = 13;

#define ANIMATION_DELAY 45 // Default slower animation speed

const SDL_Color C_BG = {30, 30, 30, 255};
const SDL_Color C_BAR = {100, 149, 237, 255};
const SDL_Color C_BAR_CMP1 = {255, 215, 0, 255};
const SDL_Color C_BAR_CMP2 = {255, 165, 0, 255};
const SDL_Color C_BAR_SWAP = {255, 69, 0, 255};
const SDL_Color C_BAR_SORTED = {0, 255, 127, 255};
const SDL_Color C_BAR_MIN = {148, 0, 211, 255};
const SDL_Color C_BAR_PIVOT = {255, 0, 0, 255}; // Color for pivot in Quick Sort
const SDL_Color C_TXT = {220, 220, 220, 255};
const SDL_Color C_MENU_BG = {40, 40, 40, 255};
const SDL_Color C_MENU_BTN = {60, 60, 60, 255};
const SDL_Color C_MENU_BTN_H = {80, 80, 80, 255};
const SDL_Color C_MENU_BTN_TXT = C_TXT;

// Globals
SDL_Window* win = nullptr;
SDL_Renderer* ren = nullptr;
TTF_Font* fnt = nullptr;
int arr[A_SIZE];
enum Algo { BUBBLE, INSERTION, SELECTION, MERGE, QUICK, NONE }; // Added MERGE and QUICK
Algo curAlgo = NONE;
bool sorting = false;

// Function Declarations
bool init();
void cleanup();
void genArr();
void drawArr(int cmp1 = -1, int cmp2 = -1, int swp1 = -1, int swp2 = -1, int minIdx = -1, int sorted = 0, int pivotIdx = -1, int leftBound = -1, int rightBound = -1); // Added pivotIdx, leftBound, rightBound for visualization
void drawMenu();
void renderTxt(const string& txt, int x, int y, SDL_Color clr);
bool handleEvents();
void setupMenuBtns();
void drawRRect(SDL_Renderer* ren, SDL_Rect r, int rad, SDL_Color clr);

// Sorting Algorithms
void bubble();
void insertion();
void selection();
void mergeSort();
void quickSort();

void mergeSortHelper(int l, int r, int sortedUntil); // Helper for recursive merge sort
void merge(int l, int m, int r); // Helper for merging subarrays

void quickSortHelper(int low, int high, int sortedUntil); // Helper for recursive quick sort
int partition(int low, int high, int sortedUntil); // Helper for partitioning in quicksort


// Button Structure
struct Btn {
    SDL_Rect r;
    string txt;
    Algo algo;
};
vector<Btn> menuBtns;

// Main
int main(int argc, char* args[]) {
    if (!init()) return 1;

    genArr();
    setupMenuBtns();

    bool quit = false;
    while (!quit) {
        quit = handleEvents();
        if (!quit) {
            SDL_SetRenderDrawColor(ren, C_BG.r, C_BG.g, C_BG.b, C_BG.a);
            SDL_RenderClear(ren);

            drawArr();
            drawMenu();

            SDL_RenderPresent(ren);
            SDL_Delay(ANIMATION_DELAY);
        }

        if (sorting) {
            switch (curAlgo) {
                case BUBBLE: bubble(); break;
                case INSERTION: insertion(); break;
                case SELECTION: selection(); break;
                case MERGE: mergeSort(); break; // Call mergeSort
                case QUICK: quickSort(); break; // Call quickSort
                case NONE: sorting = false; break;
            }
            sorting = false;
        }
    }

    cleanup();
    return 0;
}

// Initialize
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        return false;
    }
    if (TTF_Init() == -1) {
        cerr << "TTF_Init Error: " << TTF_GetError() << endl;
        return false;
    }

    win = SDL_CreateWindow("Sorting Algo Visualizer BY KCE079BCT037", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_SHOWN);
    if (!win) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return false;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        return false;
    }

    fnt = TTF_OpenFont("fonts/Arial.ttf", F_SIZE);
    if (!fnt) {
        cerr << "TTF_OpenFont Error: " << TTF_GetError() << endl;
        return false;
    }

    return true;
}

// Cleanup
void cleanup() {
    if (fnt) TTF_CloseFont(fnt);
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
}

// Generate Array
void genArr() {
    random_device rd;
    default_random_engine eng(rd());
    uniform_int_distribution<int> dist(50, H - MENU_H - 50);

    for (int i = 0; i < A_SIZE; ++i) {
        arr[i] = dist(eng);
    }
}

// Draw Array
void drawArr(int cmp1, int cmp2, int swp1, int swp2, int minIdx, int sorted, int pivotIdx, int leftBound, int rightBound) {
    SDL_SetRenderDrawColor(ren, C_BG.r, C_BG.g, C_BG.b, C_BG.a);
    SDL_RenderClear(ren);

    int x = (W - (A_SIZE * O_BAR_W) + BAR_S) / 2;
    for (int i = 0; i < A_SIZE; ++i) {
        SDL_Rect bar = {x + i * O_BAR_W, H - arr[i], BAR_W, arr[i]};
        SDL_Color clr = C_BAR;

        if (i < sorted) {
            clr = C_BAR_SORTED;
        } else if (i == cmp1 || i == cmp2) {
            clr = C_BAR_CMP1;
            if (i == cmp2) clr = C_BAR_CMP2;
        } else if (i == swp1 || i == swp2) {
            clr = C_BAR_SWAP;
        } else if (i == minIdx) {
            clr = C_BAR_MIN;
        } else if (i == pivotIdx) {
            clr = C_BAR_PIVOT;
        }

        SDL_SetRenderDrawColor(ren, clr.r, clr.g, clr.b, clr.a);
        SDL_RenderFillRect(ren, &bar);

        if (leftBound != -1 && rightBound != -1 && i >= leftBound && i <= rightBound) { // Indicate subarray bounds for merge/quick sort
             SDL_SetRenderDrawColor(ren, C_TXT.r, C_TXT.g, C_TXT.b, C_TXT.a);
             SDL_RenderDrawRect(ren, &bar);
        }


        renderTxt(to_string(arr[i]), x + i * O_BAR_W + (BAR_W - 20) / 2, H - arr[i] - F_SIZE - 5, C_TXT);
    }
}

// Render Text
void renderTxt(const string& txt, int x, int y, SDL_Color clr) {
    SDL_Surface* surf = TTF_RenderText_Solid(fnt, txt.c_str(), clr);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);

    SDL_Rect rect = {x, y, surf->w, surf->h};

    SDL_RenderCopy(ren, tex, nullptr, &rect);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

// Handle Events
bool handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
            return true;
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int x = e.button.x;
            int y = e.button.y;

            for (const auto& btn : menuBtns) {
                if (x >= btn.r.x && x < btn.r.x + btn.r.w && y >= btn.r.y && y < btn.r.y + btn.r.h) {
                    if (btn.algo != NONE) {
                        curAlgo = btn.algo;
                        genArr();
                        sorting = true;
                    }
                    break;
                }
            }
        }
    }
    return false;
}

// Setup Menu Buttons
void setupMenuBtns() {
    int btnW = 140; // Adjusted button width
    int btnH = 40;
    int startX = (W - 5 * btnW) / 6; // Evenly space 5 buttons

    menuBtns.push_back({ {startX, 20, btnW, btnH}, "Bubble Sort", BUBBLE });
    menuBtns.push_back({ {startX * 2 + btnW, 20, btnW, btnH}, "Insertion Sort", INSERTION });
    menuBtns.push_back({ {startX * 3 + btnW * 2, 20, btnW, btnH}, "Selection Sort", SELECTION });
    menuBtns.push_back({ {startX * 4 + btnW * 3, 20, btnW, btnH}, "Merge Sort", MERGE }); // Added Merge Sort button
    menuBtns.push_back({ {startX * 5 + btnW * 4, 20, btnW, btnH}, "Quick Sort", QUICK }); // Added Quick Sort button
}

// Draw Rounded Rectangle
void drawRRect(SDL_Renderer* ren, SDL_Rect rect, int rad, SDL_Color clr) {
    SDL_SetRenderDrawColor(ren, clr.r, clr.g, clr.b, clr.a);

    int x = rect.x, y = rect.y, w = rect.w, h = rect.h;

    for (int cx = 0; cx <= rad; ++cx) {
        int cy = round(sqrt(rad * rad - cx * cx));
        SDL_RenderDrawLine(ren, x + rad - cx, y + rad - cy, x + rad - cx, y + rad);
        SDL_RenderDrawLine(ren, x + w - rad + cx, y + rad - cy, x + w - rad + cx, y + rad);
        SDL_RenderDrawLine(ren, x + rad - cx, y + h - rad, x + rad - cx, y + h - rad + cy);
        SDL_RenderDrawLine(ren, x + w - rad + cx, y + h - rad, x + w - rad + cx, y + h - rad + cy);
    }

    SDL_Rect fillRect = {x + rad, y, w - 2 * rad, h};
    SDL_RenderFillRect(ren, &fillRect);
    fillRect = {x, y + rad, w, h - 2 * rad};
    SDL_RenderFillRect(ren, &fillRect);
}

// Draw Menu
void drawMenu() {
    SDL_SetRenderDrawColor(ren, C_MENU_BG.r, C_MENU_BG.g, C_MENU_BG.b, C_MENU_BG.a);
    SDL_Rect menuRect = {0, 0, W, MENU_H};
    SDL_RenderFillRect(ren, &menuRect);

    for (const auto& btn : menuBtns) {
        SDL_Color btnClr = C_MENU_BTN;
        SDL_Point mPos;
        SDL_GetMouseState(&mPos.x, &mPos.y);
        if (SDL_PointInRect(&mPos, &btn.r)) {
            btnClr = C_MENU_BTN_H;
        }
        drawRRect(ren, btn.r, BTN_R, btnClr);
        renderTxt(btn.txt, btn.r.x + 10, btn.r.y + 10, C_MENU_BTN_TXT);
    }
}

// Sorting Algorithms

void bubble() {
    cout << "bubbleSort() function called!" << endl;
    for (int i = 0; i < A_SIZE - 1; i++) {
        for (int j = 0; j < A_SIZE - i - 1; j++) {
            drawArr(j, j + 1);
            SDL_RenderPresent(ren);
            SDL_Delay(ANIMATION_DELAY);

            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
                drawArr(-1, -1, j, j+1);
                SDL_RenderPresent(ren);
                SDL_Delay(ANIMATION_DELAY);
            }
        }
        drawArr(-1, -1, -1, -1, -1, A_SIZE - i - 1);
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
    }
    drawArr(-1, -1, -1, -1, -1, A_SIZE);
    SDL_RenderPresent(ren);
}

void insertion() {
    cout << "insertionSort() function called!" << endl;
    for (int i = 1; i < A_SIZE; i++) {
        int key = arr[i];
        int j = i - 1;

        drawArr(i, j);
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);

        while (j >= 0 && arr[j] > key) {
            drawArr(i, j);
            SDL_RenderPresent(ren);
            SDL_Delay(ANIMATION_DELAY);

            arr[j + 1] = arr[j];
            j = j - 1;
            drawArr(i, j, j+1, j+2);
            SDL_RenderPresent(ren);
            SDL_Delay(ANIMATION_DELAY);
        }
        arr[j + 1] = key;
        drawArr(-1, -1, j+1, -1);
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
        drawArr(-1, -1, -1, -1, -1, i + 1);
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
    }
    drawArr(-1, -1, -1, -1, -1, A_SIZE);
    SDL_RenderPresent(ren);
}

void selection() {
    cout << "selectionSort() function called!" << endl;
    for (int i = 0; i < A_SIZE - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < A_SIZE; j++) {
            drawArr(i, j, -1, -1, minIdx);
            SDL_RenderPresent(ren);
            SDL_Delay(ANIMATION_DELAY);

            if (arr[j] < arr[minIdx]) {
                minIdx = j;
                drawArr(i, j, -1, -1, minIdx);
                SDL_RenderPresent(ren);
                SDL_Delay(ANIMATION_DELAY);
            }
        }
        swap(arr[i], arr[minIdx]);
        drawArr(-1, -1, i, minIdx, -1, i + 1);
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
    }
    drawArr(-1, -1, -1, -1, -1, A_SIZE);
    SDL_RenderPresent(ren);
}


void mergeSort() {
    cout << "mergeSort() function called!" << endl;
    mergeSortHelper(0, A_SIZE - 1, 0);
    drawArr(-1, -1, -1, -1, -1, A_SIZE);
    SDL_RenderPresent(ren);
}

void mergeSortHelper(int l, int r, int sortedUntil) {
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSortHelper(l, m, sortedUntil);
        mergeSortHelper(m + 1, r, sortedUntil);
        merge(l, m, r);
        drawArr(-1, -1, -1, -1, -1, sortedUntil); // Update sorted portion after each merge (not precise for merge sort but visually indicates progress)
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
    }
}

void merge(int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    int L[n1], R[n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;

    while (i < n1 && j < n2) {
        drawArr(l + i, m + 1 + j, -1, -1, -1, 0, -1, l, r); // Visualize comparison in merge, highlight subarrays
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        drawArr(-1, -1, k, -1, -1, 0, -1, l, r); // Visualize swap in merge, highlight subarrays
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}


void quickSort() {
    cout << "quickSort() function called!" << endl;
    quickSortHelper(0, A_SIZE - 1, 0);
    drawArr(-1, -1, -1, -1, -1, A_SIZE);
    SDL_RenderPresent(ren);
}

void quickSortHelper(int low, int high, int sortedUntil) {
    if (low < high) {
        int pi = partition(low, high, sortedUntil);

        quickSortHelper(low, pi - 1, sortedUntil);
        quickSortHelper(pi + 1, high, sortedUntil);
    }
}

int partition(int low, int high, int sortedUntil) {
    int pivot = arr[high];
    int i = (low - 1);

    drawArr(-1, -1, -1, -1, -1, sortedUntil, high, low, high); // Visualize pivot selection, highlight subarray
    SDL_RenderPresent(ren);
    SDL_Delay(ANIMATION_DELAY);


    for (int j = low; j <= high - 1; j++) {
        drawArr(j, high, -1, -1, -1, sortedUntil, high, low, high); // Visualize comparison, highlight subarray
        SDL_RenderPresent(ren);
        SDL_Delay(ANIMATION_DELAY);

        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
            drawArr(-1, -1, i, j, -1, sortedUntil, high, low, high); // Visualize swap, highlight subarray
            SDL_RenderPresent(ren);
            SDL_Delay(ANIMATION_DELAY);
        }
    }
    swap(arr[i + 1], arr[high]);
    drawArr(-1, -1, i+1, high, -1, sortedUntil, low, high); // Visualize final pivot swap, highlight subarray
    SDL_RenderPresent(ren);
    SDL_Delay(ANIMATION_DELAY);
    return (i + 1);
}
