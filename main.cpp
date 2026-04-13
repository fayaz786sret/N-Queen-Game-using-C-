/*
 =============================================================
   N-QUEENS  |  LinkedIn Style  |  C++ + SFML
 =============================================================
   BUILD (Windows VSCode terminal):
     g++ main.cpp -o NQueens.exe -IC:/SFML-2.6.1/include
         -LC:/SFML-2.6.1/lib -lsfml-graphics -lsfml-window
         -lsfml-system -std=c++17
   BUILD (Linux):
     sudo apt install libsfml-dev
     g++ main.cpp -o NQueens -lsfml-graphics -lsfml-window
         -lsfml-system -std=c++17
 =============================================================
*/

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <vector>
#include <set>
#include <stack>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>
#include <array>
#include <cstring>

using namespace sf;

// ──────────────────────────────────────────────────────────────
//  COLORS  (matching the HTML CSS variables exactly)
// ──────────────────────────────────────────────────────────────
namespace Col {
    Color bg        {0xF4,0xF2,0xEE,255};
    Color surface   {0xFF,0xFF,0xFF,255};
    Color border    {0xE0,0xDC,0xD4,255};
    Color text      {0x1A,0x1A,0x1A,255};
    Color muted     {0x6B,0x6B,0x6B,255};
    Color accent    {0x0A,0x66,0xC2,255};
    Color accentLt  {0xE8,0xF0,0xFB,255};
    Color success   {0x05,0x76,0x42,255};
    Color cellL     {0xF0,0xEB,0xE0,255};
    Color cellD     {0xC9,0xB9,0x9A,255};
    Color cellHL    {0xE8,0xE0,0xCC,255};
    Color cellHD    {0xBF,0xAF,0x8E,255};
    Color cellCfL   {0xF5,0xD0,0xD0,255};
    Color cellCfD   {0xE0,0xB0,0xB0,255};
    Color cellHint  {0xC8,0xE0,0xFF,255};
    Color qBody     {0x2C,0x1A,0x0E,255};
    Color qJewel    {0xE8,0xC9,0x6E,255};
    Color qRing     {0xC8,0xA0,0x30,255};
    Color streakBg  {0xFF,0xF8,0xE6,255};
    Color streakBd  {0xF0,0xC0,0x30,255};
    Color streakTxt {0x7A,0x55,0x00,255};
    Color toast     {0x1A,0x1A,0x1A,220};
    Color statBox   {0xF4,0xF2,0xEE,255};
}

// ──────────────────────────────────────────────────────────────
//  GLOBALS
// ──────────────────────────────────────────────────────────────
Font   gFont;
int    WIN_W = 1060, WIN_H = 800;

// ──────────────────────────────────────────────────────────────
//  DRAWING UTILITIES
// ──────────────────────────────────────────────────────────────

// Filled rectangle
void fillRect(RenderTarget& rt, float x, float y, float w, float h, Color c) {
    RectangleShape r({w, h});
    r.setPosition(x, y);
    r.setFillColor(c);
    rt.draw(r);
}

// Outlined rectangle
void outlineRect(RenderTarget& rt, float x, float y, float w, float h,
                 Color outlineC, float thickness = 1.5f) {
    RectangleShape r({w - thickness, h - thickness});
    r.setPosition(x + thickness/2, y + thickness/2);
    r.setFillColor(Color::Transparent);
    r.setOutlineColor(outlineC);
    r.setOutlineThickness(thickness);
    rt.draw(r);
}

// Rounded rectangle (filled) — approximated with rects + circles
void roundRect(RenderTarget& rt, float x, float y, float w, float h,
               float r, Color c) {
    if (r <= 0 || r > w/2 || r > h/2) { fillRect(rt,x,y,w,h,c); return; }
    fillRect(rt, x+r,  y,    w-2*r, h,   c);
    fillRect(rt, x,    y+r,  r,     h-2*r, c);
    fillRect(rt, x+w-r,y+r,  r,     h-2*r, c);
    auto corner = [&](float cx, float cy){
        CircleShape cs(r);
        cs.setPosition(cx-r, cy-r);
        cs.setFillColor(c);
        rt.draw(cs);
    };
    corner(x+r,   y+r);
    corner(x+w-r, y+r);
    corner(x+r,   y+h-r);
    corner(x+w-r, y+h-r);
}

// Card (white surface with border + drop shadow)
void drawCard(RenderTarget& rt, float x, float y, float w, float h, float radius = 12) {
    // shadow layers
    for (int i = 4; i >= 1; i--)
        roundRect(rt, x+i*0.6f, y+i*1.2f, w, h, radius, Color(0,0,0,10));
    // border
    roundRect(rt, x, y, w, h, radius, Col::border);
    // surface
    roundRect(rt, x+1, y+1, w-2, h-2, radius-1, Col::surface);
}

// Draw text
Text makeText(const std::string& s, unsigned size, Color c,
              bool bold = false, bool italic = false) {
    Text t;
    t.setFont(gFont);
    t.setString(s);
    t.setCharacterSize(size);
    t.setFillColor(c);
    uint32_t st = Text::Regular;
    if (bold)   st |= Text::Bold;
    if (italic) st |= Text::Italic;
    t.setStyle(st);
    return t;
}

void drawText(RenderTarget& rt, const std::string& s, float x, float y,
              unsigned size, Color c, bool bold = false,
              bool centerX = false, bool centerY = false) {
    auto t = makeText(s, size, c, bold);
    auto b = t.getLocalBounds();
    float ox = centerX ? -(b.left + b.width/2.f)  : -b.left;
    float oy = centerY ? -(b.top  + b.height/2.f) : -b.top;
    t.setPosition(x+ox, y+oy);
    rt.draw(t);
}

// Center text in a rect
void drawTextCenter(RenderTarget& rt, const std::string& s,
                    float rx, float ry, float rw, float rh,
                    unsigned size, Color c, bool bold = false) {
    drawText(rt, s, rx+rw/2, ry+rh/2, size, c, bold, true, true);
}

float textWidth(const std::string& s, unsigned size, bool bold = false) {
    auto t = makeText(s, size, Color::White, bold);
    auto b = t.getLocalBounds();
    return b.left + b.width;
}

bool mouseIn(Vector2i m, float x, float y, float w, float h) {
    return m.x >= x && m.x <= x+w && m.y >= y && m.y <= y+h;
}

std::string formatTime(int sec) {
    std::ostringstream o;
    o << sec/60 << ":" << std::setw(2) << std::setfill('0') << sec%60;
    return o.str();
}

// ──────────────────────────────────────────────────────────────
//  QUEEN DRAWING (matches the HTML SVG polygon exactly)
// ──────────────────────────────────────────────────────────────
void drawQueen(RenderTarget& rt, float cx, float cy, float cellSz) {
    float s = cellSz * 0.42f;
    // Map from 100x100 SVG space to screen space
    auto pt = [&](float px, float py) -> Vector2f {
        return { cx + (px-50.f)/100.f * s*2.2f,
                 cy + (py-50.f)/100.f * s*2.2f };
    };

    // Crown polygon: 50,10 62,38 92,38 68,56 78,85 50,68 22,85 32,56 8,38 38,38
    ConvexShape crown(10);
    crown.setPoint(0, pt(50,10)); crown.setPoint(1, pt(62,38));
    crown.setPoint(2, pt(92,38)); crown.setPoint(3, pt(68,56));
    crown.setPoint(4, pt(78,85)); crown.setPoint(5, pt(50,68));
    crown.setPoint(6, pt(22,85)); crown.setPoint(7, pt(32,56));
    crown.setPoint(8, pt( 8,38)); crown.setPoint(9, pt(38,38));
    crown.setFillColor(Col::qBody);
    crown.setOutlineColor(Color(0x0A,0x06,0x03,255));
    crown.setOutlineThickness(1.2f);
    rt.draw(crown);

    // Base bar
    auto b1 = pt(18,85), b2 = pt(82,96);
    RectangleShape bar({b2.x-b1.x, b2.y-b1.y});
    bar.setPosition(b1);
    bar.setFillColor(Col::qBody);
    rt.draw(bar);

    // Jewels
    auto jewel = [&](float jx, float jy, float jr) {
        float r2 = jr * s / 50.f;
        CircleShape c(r2);
        c.setOrigin(r2, r2);
        c.setPosition(pt(jx,jy));
        c.setFillColor(Col::qJewel);
        c.setOutlineColor(Col::qRing);
        c.setOutlineThickness(1.2f);
        rt.draw(c);
    };
    jewel(50,10, 7);
    jewel( 8,38, 5); jewel(92,38, 5);
    jewel(22,85, 5); jewel(78,85, 5);
}

// X marker for "excluded" cells
void drawX(RenderTarget& rt, float cx, float cy, float cellSz) {
    float o = cellSz * 0.22f;
    Color c(100,80,60,100);
    for (float d : {-0.8f, 0.f, 0.8f}) {
        VertexArray v(Lines, 4);
        v[0] = {{cx-o+d, cy-o}, c}; v[1] = {{cx+o+d, cy+o}, c};
        v[2] = {{cx+o,   cy-o+d}, c}; v[3] = {{cx-o, cy+o+d}, c};
        rt.draw(v);
    }
}

// ──────────────────────────────────────────────────────────────
//  BUTTON HELPER
// ──────────────────────────────────────────────────────────────
enum BtnStyle { BTN_PRIMARY, BTN_OUTLINE, BTN_GHOST };

struct Button {
    float x, y, w, h;
    std::string label;
    BtnStyle style;

    bool contains(Vector2i m) const {
        return mouseIn(m, x, y, w, h);
    }

    void draw(RenderTarget& rt, Vector2i mouse) const {
        bool hov = contains(mouse);
        Color fill, tc, bc;

        if (style == BTN_PRIMARY) {
            fill = hov ? Color(0x08,0x4F,0x99,255) : Col::accent;
            tc   = Col::surface;
            bc   = Color::Transparent;
        } else if (style == BTN_OUTLINE) {
            fill = hov ? Col::accentLt : Col::surface;
            tc   = hov ? Col::accent   : Col::text;
            bc   = hov ? Col::accent   : Col::border;
        } else { // GHOST
            fill = hov ? Color(0xF4,0xF2,0xEE,255) : Color::Transparent;
            tc   = hov ? Col::text     : Col::muted;
            bc   = hov ? Col::border   : Color::Transparent;
        }

        roundRect(rt, x, y, w, h, h/2, fill);
        if (bc != Color::Transparent) {
            outlineRect(rt, x, y, w, h, bc, 1.5f);
        }
        drawTextCenter(rt, label, x, y, w, h, 13, tc, true);
    }
};

// ──────────────────────────────────────────────────────────────
//  GAME STATE
// ──────────────────────────────────────────────────────────────
struct GameState {
    int N = 6;
    int board[9][9] = {};       // 0=empty 1=queen 2=marked(X)
    std::stack<std::array<std::array<int,9>,9>> history;
    std::set<std::pair<int,int>> conflicts;
    std::pair<int,int> hover   = {-1,-1};
    std::pair<int,int> hintCell= {-1,-1};

    // Timer
    float elapsed  = 0.f;
    bool  running  = true;
    bool  won      = false;

    // Stats
    int   wins      = 0;
    int   streak    = 3;
    int   hintsUsed = 0;
    float bestTime  = -1.f;

    // Win overlay
    float winFade   = 0.f;
    bool  showWin   = false;

    // Hint timer
    float hintTimer = 0.f;

    // Toast
    std::string toastMsg;
    float toastTimer = 0.f;

    // Confetti
    struct Particle {
        Vector2f pos, vel;
        float rot, rotSpd, life, maxLife;
        Color col;
    };
    std::vector<Particle> particles;
    std::mt19937 rng{(unsigned)std::time(nullptr)};

    void recalcConflicts() {
        conflicts.clear();
        std::vector<std::pair<int,int>> qs;
        for (int r=0;r<N;r++) for (int c=0;c<N;c++)
            if (board[r][c]==1) qs.push_back({r,c});
        for (int i=0;i<(int)qs.size();i++)
            for (int j=i+1;j<(int)qs.size();j++) {
                int r1 = qs[i].first;
                int c1 = qs[i].second;
                int r2 = qs[j].first;
                int c2 = qs[j].second;
                if (r1==r2||c1==c2||std::abs(r1-r2)==std::abs(c1-c2)) {
                    conflicts.insert(std::make_pair(r1, c1));
                    conflicts.insert(std::make_pair(r2, c2));
                }
            }
    }

    bool isSolved() {
        int cnt=0;
        for (int r=0;r<N;r++) for (int c=0;c<N;c++) if(board[r][c]==1) cnt++;
        if (cnt!=N || !conflicts.empty()) return false;
        std::set<int> rows,cols;
        for (int r=0;r<N;r++) for (int c=0;c<N;c++)
            if (board[r][c]==1) { rows.insert(r); cols.insert(c); }
        return (int)rows.size()==N && (int)cols.size()==N;
    }

    int queensPlaced() {
        int k=0;
        for (int r=0;r<N;r++) for (int c=0;c<N;c++) if(board[r][c]==1) k++;
        return k;
    }

    bool isSafe(int r, int c) {
        for (int rr=0;rr<N;rr++) for (int cc=0;cc<N;cc++)
            if (board[rr][cc]==1)
                if (rr==r||cc==c||std::abs(rr-r)==std::abs(cc-c)) return false;
        return true;
    }

    void clickCell(int r, int c) {
        if (won) return;
        // save undo snapshot
        std::array<std::array<int,9>,9> snap;
        for (int i=0;i<N;i++) for (int j=0;j<N;j++) snap[i][j]=board[i][j];
        history.push(snap);
        // cycle: 0→1→2→0
        int& v = board[r][c];
        v = (v==0) ? 1 : (v==1) ? 2 : 0;
        recalcConflicts();
        if (isSolved()) triggerWin();
    }

    void undo() {
        if (history.empty()) return;
        auto snap = history.top(); history.pop();
        for (int i=0;i<N;i++) for (int j=0;j<N;j++) board[i][j]=snap[i][j];
        recalcConflicts();
        won=false; running=true; showWin=false; winFade=0;
    }

    void hint() {
        for (int r=0;r<N;r++) {
            bool has=false;
            for (int c=0;c<N;c++) if(board[r][c]==1){has=true;break;}
            if (has) continue;
            for (int c=0;c<N;c++) {
                if (isSafe(r,c)) {
                    hintsUsed++;
                    hintCell={r,c}; hintTimer=2.f;
                    showToast("Try row "+std::to_string(r+1)+
                              ", col "+std::to_string(c+1));
                    return;
                }
            }
        }
        showToast("No hint available — try undoing!");
    }

    void reset(int n=-1) {
        if (n>0) N=n;
        std::memset(board,0,sizeof(board));
        while (!history.empty()) history.pop();
        conflicts.clear();
        hover={-1,-1}; hintCell={-1,-1}; hintTimer=0;
        elapsed=0; running=true; won=false;
        hintsUsed=0; showWin=false; winFade=0;
        particles.clear();
    }

    void triggerWin() {
        won=true; running=false;
        wins++;
        if (bestTime<0||elapsed<bestTime) bestTime=elapsed;
        showWin=true;
        spawnConfetti();
        showToast("Congratulations! You solved it!");
    }

    void showToast(const std::string& msg) {
        toastMsg=msg; toastTimer=3.f;
    }

    void spawnConfetti() {
        static const Color cols[] = {
            Col::accent, Col::success, Col::qJewel,
            Color(0xCC,0x10,0x16,255), Color(0x7C,0x3A,0xED,255), Color(0xEA,0x58,0x0C,255)
        };
        for (int i=0;i<80;i++) {
            Particle p;
            p.pos  = {float(rng()%WIN_W), float(-20-int(rng()%80))};
            p.vel  = {float(int(rng()%300)-150), float(rng()%250+100)};
            p.rot  = float(rng()%360);
            p.rotSpd= float(int(rng()%400)-200);
            p.life = p.maxLife = 1.5f + float(rng()%100)/100.f;
            p.col  = cols[rng()%6];
            particles.push_back(p);
        }
    }

    void update(float dt) {
        if (running && !won) elapsed += dt;

        // Hint timer
        if (hintTimer>0) { hintTimer-=dt; if(hintTimer<=0) hintCell={-1,-1}; }

        // Toast fade
        if (toastTimer>0) toastTimer-=dt;

        // Win overlay fade
        if (showWin)  winFade = std::min(1.f, winFade + dt*2.5f);
        else          winFade = std::max(0.f, winFade - dt*3.f);

        // Confetti physics
        for (auto& p : particles) {
            p.life -= dt;
            p.pos  += p.vel * dt;
            p.vel.y+= 400.f*dt;
            p.rot  += p.rotSpd*dt;
        }
        particles.erase(
            std::remove_if(particles.begin(),particles.end(),
                [](const Particle& p){ return p.life<=0; }),
            particles.end()
        );
    }
};

// ──────────────────────────────────────────────────────────────
//  BOARD LAYOUT (auto-sizes per N)
// ──────────────────────────────────────────────────────────────
const int  NAV_H = 56;
const int  PAD   = 20;

int cellSize(int n) {
    if (n<=4) return 82;
    if (n<=6) return 70;
    return 60;
}

float boardPixels(int n) { return float(n * cellSize(n)); }

// The game card occupies the left ~520px
const float GC_X = 30.f;
const float GC_W = 520.f;

float boardX(int n) {
    float inner = GC_W - 2*PAD;
    return GC_X + PAD + (inner - boardPixels(n))/2.f;
}
float boardY(int n) {
    // Header card: y=NAV_H+16, h=80
    // Size row: h=32
    // Content row y = NAV_H+16+80+12+32+14 = NAV_H+154
    return float(NAV_H + 154 + PAD);
}
float gcY(int n) { return float(NAV_H+154); }
float gcH(int n) { return float(2*PAD + boardPixels(n) + 16 + 40); }

// Side panel
const float SP_X = GC_X + GC_W + 18.f;
const float SP_W = 260.f;

// ──────────────────────────────────────────────────────────────
//  RENDER
// ──────────────────────────────────────────────────────────────

void renderNav(RenderTarget& rt, Vector2i mouse) {
    // Bar background
    fillRect(rt, 0, 0, float(WIN_W), float(NAV_H), Col::surface);
    fillRect(rt, 0, float(NAV_H)-1, float(WIN_W), 1.f, Col::border);

    // LinkedIn "in" box logo
    roundRect(rt, 30, 14, 28, 28, 5, Col::accent);
    drawTextCenter(rt, "Game", 30, 14, 28, 28, 14, Col::surface, true);

    // "LinkedIn" word
    drawText(rt, "LinkedGame", 66, 16, 15, Col::accent, true);

    // Divider
    fillRect(rt, 132, 18, 1.5f, 20, Col::border);

    // "Games"
    drawText(rt, "Games", 140, 19, 13, Col::muted);

    // Share button (right)
    float sbx = float(WIN_W-120), sby = 14;
    bool hov = mouseIn(mouse, sbx, sby, 90, 28);
    roundRect(rt, sbx, sby, 90, 28, 14, hov ? Col::border : Col::surface);
    outlineRect(rt, sbx, sby, 90, 28, Col::border, 1.5f);
    drawTextCenter(rt, "Share", sbx, sby, 90, 28, 13, hov?Col::text:Col::muted);
}

void renderHeaderCard(RenderTarget& rt, int secs) {
    float x=GC_X, y=float(NAV_H+16), w=float(WIN_W-60), h=80.f;
    drawCard(rt, x, y, w, h);

    // Title
    drawText(rt, "Queens", x+50, y+14, 24, Col::text, true);
    // Crown icon (hand-drawn small)
    ConvexShape crown(7);
    float cx=x+38, cy2=y+26;
    float cs=12;
    crown.setPoint(0,{cx-cs,    cy2+cs});
    crown.setPoint(1,{cx-cs,    cy2});
    crown.setPoint(2,{cx-cs/2,  cy2+cs*0.4f});
    crown.setPoint(3,{cx,       cy2-cs});
    crown.setPoint(4,{cx+cs/2,  cy2+cs*0.4f});
    crown.setPoint(5,{cx+cs,    cy2});
    crown.setPoint(6,{cx+cs,    cy2+cs});
    crown.setFillColor(Col::accent);
    rt.draw(crown);

    // Date
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%A, %B %d", std::localtime(&now));
    drawText(rt, buf, x+24, y+46, 13, Col::muted);

    // Timer area (right side)
    float tx2 = x + w - 220;

    // Clock circle icon
    CircleShape clk(10);
    clk.setPosition(tx2, y+20);
    clk.setFillColor(Color::Transparent);
    clk.setOutlineColor(Col::muted);
    clk.setOutlineThickness(1.8f);
    rt.draw(clk);
    VertexArray hands(Lines, 4);
    hands[0] = {{tx2+10, y+30}, Col::muted};
    hands[1] = {{tx2+10, y+23}, Col::muted};
    hands[2] = {{tx2+10, y+30}, Col::muted};
    hands[3] = {{tx2+15, y+30}, Col::muted};
    rt.draw(hands);

    std::string ts = formatTime(secs);
    drawText(rt, ts, tx2+26, y+14, 24, Col::text, true);

    // Streak badge
    float bx=x+w-90, by2=y+22, bw=76, bh=30;
    roundRect(rt, bx, by2, bw, bh, 15, Col::streakBg);
    outlineRect(rt, bx, by2, bw, bh, Col::streakBd, 1.5f);
    drawTextCenter(rt, "Fire  3", bx, by2, bw, bh, 13, Col::streakTxt, true);
}

void renderSizeRow(RenderTarget& rt, int N, Vector2i mouse,
                   Button sizeBtns[3]) {
    int sizes[] = {4,6,8};
    const char* labels[] = {"4 x 4","6 x 6","8 x 8"};
    float totalW = 3*80.f + 2*8.f;
    float sx = (WIN_W - totalW)/2.f;
    float sy = float(NAV_H + 16 + 80 + 12);

    for (int i=0;i<3;i++) {
        float bx = sx + i*88.f;
        sizeBtns[i] = {bx, sy, 80.f, 30.f, labels[i],
                       sizes[i]==N ? BTN_PRIMARY : BTN_OUTLINE};
        sizeBtns[i].draw(rt, mouse);
    }
}

void renderBoard(RenderTarget& rt, GameState& g) {
    float bx = boardX(g.N), by = boardY(g.N);
    int   cs = cellSize(g.N);
    float bw = boardPixels(g.N);

    // Board shadow
    for (int i=5;i>=1;i--)
        roundRect(rt, bx-1+i*0.5f, by+i, bw+2, bw, 5,
                  Color(0,0,0,12));

    for (int r=0;r<g.N;r++) for (int c=0;c<g.N;c++) {
        float cx = bx + c*cs, cy = by + r*cs;
        bool  light  = (r+c)%2==0;
        bool  conf   = g.conflicts.count({r,c})>0;
        bool  hint   = (g.hintCell.first==r && g.hintCell.second==c);
        bool  hover  = (g.hover.first==r && g.hover.second==c);

        Color bg;
        if      (hint)  bg = Col::cellHint;
        else if (conf)  bg = light ? Col::cellCfL : Col::cellCfD;
        else if (hover && g.board[r][c]==0)
                        bg = light ? Col::cellHL  : Col::cellHD;
        else            bg = light ? Col::cellL   : Col::cellD;

        fillRect(rt, cx, cy, float(cs), float(cs), bg);

        // subtle grid lines
        fillRect(rt, cx, cy,         float(cs), 0.5f, Color(160,140,100,40));
        fillRect(rt, cx, cy,         0.5f, float(cs), Color(160,140,100,40));

        float ccx = cx + cs/2.f, ccy = cy + cs/2.f;
        if      (g.board[r][c]==1) drawQueen(rt, ccx, ccy, float(cs));
        else if (g.board[r][c]==2) drawX(rt, ccx, ccy, float(cs));

        // Hint highlight ring
        if (hint) {
            RectangleShape ring({float(cs)-4, float(cs)-4});
            ring.setPosition(cx+2, cy+2);
            ring.setFillColor(Color::Transparent);
            ring.setOutlineColor(Col::accent);
            ring.setOutlineThickness(3.f);
            rt.draw(ring);
        }
    }

    // Outer board border
    RectangleShape border({bw, bw});
    border.setPosition(bx, by);
    border.setFillColor(Color::Transparent);
    border.setOutlineColor(Color(0xC8,0xC0,0xB0,255));
    border.setOutlineThickness(2.f);
    rt.draw(border);
}

void renderGameCard(RenderTarget& rt, GameState& g, Vector2i mouse,
                    Button& undoBtn, Button& hintBtn, Button& resetBtn) {
    float cx = GC_X, cy = gcY(g.N), cw = GC_W, ch = gcH(g.N);
    drawCard(rt, cx, cy, cw, ch);

    renderBoard(rt, g);

    // Action buttons centered below board
    float bw=110, bh=36;
    float totalBW = bw*3 + 8*2;
    float bx = cx + (cw - totalBW)/2.f;
    float by = cy + ch - bh - PAD;

    undoBtn  = {bx,            by, bw, bh, "< Undo",   BTN_OUTLINE};
    hintBtn  = {bx+bw+8,       by, bw, bh, "* Hint",   BTN_OUTLINE};
    resetBtn = {bx+(bw+8)*2,   by, bw, bh, "~ Reset",  BTN_GHOST};

    undoBtn.draw(rt, mouse);
    hintBtn.draw(rt, mouse);
    resetBtn.draw(rt, mouse);
}

void renderProgressCard(RenderTarget& rt, GameState& g,
                         float x, float y, float w) {
    float h = 82.f;
    drawCard(rt, x, y, w, h);

    drawText(rt, "PROGRESS", x+18, y+14, 11, Col::muted, true);
    drawText(rt, "Queens placed", x+18, y+36, 13, Col::text);

    std::string prog = std::to_string(g.queensPlaced()) + " / " +
                       std::to_string(g.N);
    float pw = textWidth(prog, 13, true);
    drawText(rt, prog, x+w-18-pw, y+36, 13, Col::accent, true);

    // Progress bar
    float bx=x+18, by2=y+62, bw2=w-36, bh=6;
    roundRect(rt, bx, by2, bw2, bh, 3, Col::statBox);
    if (g.N>0) {
        float fill = bw2 * g.queensPlaced() / float(g.N);
        if (fill>0) roundRect(rt, bx, by2, fill, bh, 3, Col::accent);
    }
}

void renderStatsCard(RenderTarget& rt, GameState& g,
                      float x, float y, float w) {
    float h = 126.f;
    drawCard(rt, x, y, w, h);
    drawText(rt, "TODAY'S STATS", x+18, y+14, 11, Col::muted, true);

    struct Stat { std::string val, lbl; };
    Stat stats[4] = {
        {std::to_string(g.wins), "Wins"},
        {g.bestTime<0 ? "--" : formatTime(int(g.bestTime)), "Best"},
        {std::to_string(g.streak), "Streak"},
        {std::to_string(g.hintsUsed), "Hints"}
    };

    float sw = (w-36)/2.f, sh = 44.f;
    float gx = x+10, gy = y+36;
    for (int i=0;i<4;i++) {
        float bx = gx + (i%2)*(sw+8);
        float by2 = gy + (i/2)*(sh+6);
        roundRect(rt, bx, by2, sw, sh, 8, Col::statBox);
        drawTextCenter(rt, stats[i].val, bx, by2, sw, sh*0.55f, 20,Col::text,true);
        drawTextCenter(rt, stats[i].lbl, bx, by2+sh*0.55f, sw, sh*0.45f, 11,Col::muted);
    }
}

void renderRulesCard(RenderTarget& rt, float x, float y, float w) {
    float h = 178.f;
    drawCard(rt, x, y, w, h);
    drawText(rt, "HOW TO PLAY", x+18, y+14, 11, Col::muted, true);

    struct Rule { std::string num, text; };
    Rule rules[4] = {
        {"1", "Place one queen per row."},
        {"2", "No two queens share a column."},
        {"3", "No two queens share a diagonal."},
        {"Q", "Click: empty -> queen -> x -> empty"}
    };
    for (int i=0;i<4;i++) {
        float ry = y + 36 + i*34.f;
        // Badge circle
        roundRect(rt, x+18, ry, 22, 22, 11, Col::accentLt);
        drawTextCenter(rt, rules[i].num, x+18, ry, 22, 22, 11, Col::accent, true);
        drawText(rt, rules[i].text, x+48, ry+5, 12, Col::text);
    }
}

void renderSidePanel(RenderTarget& rt, GameState& g, Vector2i mouse) {
    float x=SP_X, y=gcY(g.N), w=SP_W;
    renderProgressCard(rt, g, x, y,        w); y += 82+12;
    renderStatsCard   (rt, g, x, y,        w); y += 126+12;
    renderRulesCard   (rt,    x, y,        w);
}

void renderWinOverlay(RenderTarget& rt, GameState& g, Vector2i mouse,
                       Button& playAgainBtn) {
    if (g.winFade <= 0.f) return;
    float bx=boardX(g.N), by=boardY(g.N), bw=boardPixels(g.N);
    uint8_t a = uint8_t(g.winFade * 240);

    // Frosted white overlay
    roundRect(rt, bx, by, bw, bw, 6, Color(255,255,255,a));

    float cx = bx+bw/2, cy = by+bw/2;

    // Crown
    drawQueen(rt, cx, cy-bw*0.22f, float(cellSize(g.N))*2.8f);

    // "Solved!" text
    Color sc = Col::success; sc.a = a;
    drawText(rt, "Solved!", cx - textWidth("Solved!",28,true)/2, cy+bw*0.1f,
             28, sc, true);

    // Time text
    Color mc = Col::muted; mc.a = a;
    std::string ts = "Solved in " + formatTime(int(g.elapsed));
    drawText(rt, ts, cx - textWidth(ts,14)/2, cy+bw*0.18f+4, 14, mc);

    // Play Again button
    float pbw=140, pbh=40;
    playAgainBtn = {cx-pbw/2, cy+bw*0.28f, pbw, pbh, "Play Again", BTN_PRIMARY};
    if (g.winFade > 0.5f) playAgainBtn.draw(rt, mouse);
}

void renderConfetti(RenderTarget& rt, const GameState& g) {
    for (auto& p : g.particles) {
        float alpha = p.life / p.maxLife;
        RectangleShape s({9,9});
        s.setOrigin(4.5f, 4.5f);
        s.setPosition(p.pos);
        s.setRotation(p.rot);
        Color c = p.col;
        c.a = uint8_t(alpha*255);
        s.setFillColor(c);
        rt.draw(s);
    }
}

void renderToast(RenderTarget& rt, const GameState& g) {
    if (g.toastTimer <= 0) return;
    float alpha = std::min(1.f, g.toastTimer / 0.4f);
    float tw = textWidth(g.toastMsg, 13) + 40;
    tw = std::max(tw, 220.f);
    float tx = (WIN_W - tw)/2.f, ty = float(WIN_H-60);
    Color bg = Col::toast; bg.a = uint8_t(alpha*220);
    roundRect(rt, tx, ty, tw, 36, 8, bg);
    Color tc(255,255,255, uint8_t(alpha*255));
    drawTextCenter(rt, g.toastMsg, tx, ty, tw, 36, 13, tc);
}

// ──────────────────────────────────────────────────────────────
//  MAIN
// ──────────────────────────────────────────────────────────────
int main() {
    // Load font — try common system paths
    const char* fontPaths[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        nullptr
    };
    bool fontLoaded = false;
    for (int i = 0; fontPaths[i] && !fontLoaded; i++)
        fontLoaded = gFont.loadFromFile(fontPaths[i]);
    if (!fontLoaded) gFont.loadFromFile("arial.ttf");  // last resort

    RenderWindow window(
        VideoMode(WIN_W, WIN_H),
        "Queens - Daily Puzzle",
        Style::Titlebar | Style::Close
    );
    window.setFramerateLimit(60);

    GameState gs;
    gs.reset(6);

    Button sizeBtns[3];
    Button undoBtn, hintBtn, resetBtn, playAgainBtn;
    Clock  clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        Vector2i mouse = Mouse::getPosition(window);

        // ── EVENTS ──────────────────────────────────────────
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed)
                window.close();

            // Track hover cell
            if (ev.type == Event::MouseMoved) {
                float bx = boardX(gs.N), by = boardY(gs.N);
                int   cs = cellSize(gs.N);
                float bw = boardPixels(gs.N);
                int mx=ev.mouseMove.x, my=ev.mouseMove.y;
                if (mx>=bx && mx<bx+bw && my>=by && my<by+bw) {
                    int col=(mx-bx)/cs, row=(my-by)/cs;
                    if (col>=0&&col<gs.N&&row>=0&&row<gs.N)
                        gs.hover = {row,col};
                    else gs.hover = {-1,-1};
                } else gs.hover = {-1,-1};
            }

            if (ev.type == Event::MouseButtonPressed &&
                ev.mouseButton.button == Mouse::Left) {
                Vector2i mp(ev.mouseButton.x, ev.mouseButton.y);

                // Board cell click
                if (!gs.showWin || gs.winFade < 0.3f) {
                    float bx=boardX(gs.N), by=boardY(gs.N);
                    float bw=boardPixels(gs.N);
                    int   cs=cellSize(gs.N);
                    if (mouseIn(mp, bx, by, bw, bw)) {
                        int col=(mp.x-bx)/cs, row=(mp.y-by)/cs;
                        if (row>=0&&row<gs.N&&col>=0&&col<gs.N)
                            gs.clickCell(row, col);
                    }
                }

                // Size buttons
                int szVals[]={4,6,8};
                for (int i=0;i<3;i++)
                    if (sizeBtns[i].contains(mp)) gs.reset(szVals[i]);

                // Action buttons
                if (undoBtn.contains(mp))    gs.undo();
                if (hintBtn.contains(mp))    gs.hint();
                if (resetBtn.contains(mp))   gs.reset();

                // Play again
                if (gs.winFade>0.5f && playAgainBtn.contains(mp))
                    gs.reset();

                // Share
                float sbx=float(WIN_W-120); float sby=14;
                if (mouseIn(mp, sbx, sby, 90, 28))
                    gs.showToast("Result: Queens "+
                        std::to_string(gs.N)+"x"+
                        std::to_string(gs.N)+" in "+
                        formatTime(int(gs.elapsed)));
            }
        }

        gs.update(dt);

        // ── RENDER ──────────────────────────────────────────
        window.clear(Col::bg);

        renderNav        (window, mouse);
        renderHeaderCard (window, int(gs.elapsed));
        renderSizeRow    (window, gs.N, mouse, sizeBtns);
        renderGameCard   (window, gs, mouse, undoBtn, hintBtn, resetBtn);
        renderSidePanel  (window, gs, mouse);
        renderConfetti   (window, gs);
        renderWinOverlay (window, gs, mouse, playAgainBtn);
        renderToast      (window, gs);

        window.display();
    }
    return 0;
}
