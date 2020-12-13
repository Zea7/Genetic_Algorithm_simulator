#include <iostream>
#include <random>
#include <queue>
#include <utility>
#include <Windows.h>
using namespace std;

void print_queue(queue<pair<int,int>> q);

struct Creature {
    int fly;                            //한 번 시행마다 움직일 수 있는 칸의 수
    int lifespan;                       //수명
    int baby;                           //자식의 수
    int stamina;                        //함정과 마주쳐도 버틸 수 있는 수
    int hp;                             //남은 체력의 수
    int stress=0;                       //같은 칸에 있는 creature 수. 5가 넘어가면 죽는다.
    int age = 0;                        //생성시 나이
    Creature* next;
};

class Life {
public:
    int length=0;
    Creature* head = NULL;
    Creature* tail = NULL;
    void add(Creature* n);
    Creature del(int n);
};

void Life::add(Creature* n) {
    if (this->length == 0) {
        this->head = n;
        this->tail = this->head;
        this->length++;
        return;
    }
    this->tail->next = n;
    this->tail = this->tail->next;
    this->length++;
}

Creature Life::del(int n) {
    Creature* p = this->head;
    if (n == 1 && length == 1) {
        Creature q = *p;
        this->head = 0;
        this->tail = 0;
        length--;
        return q;
    }
    if (n == 1) {
        Creature q = *p;
        this->head = p->next;
        this->length--;
        return q;
    }
    for (int i = 1; i < (n-1); i++) {
        p = p->next;
    }
    Creature q = *(p->next);
    p->next = p->next->next;
    this->length--;
    return q;
    
}

class Simulator {
private:
    Life** life;                       //각 칸의 생물체들의 집합
    int** board;                       //생물체들이 살아갈 공간(정사각형)
    bool** check;                      //이미 시행한 칸인지 확인
    bool** trap;                       //함정의 위치
    int x;                             //board 한 변의 길이
    int lives;                         //총 creature의 수
    int act;                           //진행한 시행의 수
    int f = 10;                        //최대 fly
    int l = 5;                         //최대 lifespan
    int bb = 2;                        //최대 baby
    int s = 1;                         //최대 stamina
    queue<pair<int, int>> q;           //Creature들이 존재하는 좌표값
public:
    Simulator(int n, int t, int b);                   //생성자(n은 시작시 creature의 수, t는 함정의 수, b는 board 한 변의 길이
    void turn(int n);                   //한 번의시뮬레이션 
    void print_mean();                  //현재 생존해 있는 creature들의 status의 평균 출력
    void move(int x, int y);            //(x,y) 좌표의 creature들이 1~fly 칸만큼 움직임.
    void print_status();                //디버깅용 함수
    void make_up();                     //시행마다 배열들을 정리
};

void Simulator::turn(int n) {
    if (n == 0) return;
    act++;    
    int qs = q.size();

    for (int i = 0; i < qs;i++) {
        pair<int, int> r = q.front();
        q.pop();
        int x = r.first, y = r.second;
        if (this->check[x][y] == true) continue;
        this->move(x, y);
    }
    this->make_up();
    cout << "---------------------------" << endl;
    cout << this->act << "번째 시행 : " << endl;
    this->print_mean();
    this->print_status();
    cout << "---------------------------" << endl;
    Sleep(2000);
    this->turn(--n);
}

void Simulator::print_status() {
    int number = 0;
    cout << this->lives << "개의 Creature" << endl;
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < x; j++) {
            cout << this->board[i][j] << " ";
            number += this->board[i][j];
        }
        cout << endl;
    }
    cout << "Board 내의 Creature 수는 " << number << endl;
    if (number != this->lives) {
        cout << "오류가 있음" << endl;
        exit(0);
    }
}

void Simulator::move(int x, int y) {
    if (this->check[x][y] == true) return;
    this->check[x][y] = true;
    int n = this->board[x][y],a,x_h,y_h;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> r(1, 4);
    Creature* h = this->life[x][y].head;
    for (int i = 1; i <= n; i++) {
        bool living = true;
        h->age++;
        Creature* o = new Creature();
        o->fly = h->fly;
        o->baby = h->baby;
        o->lifespan = h->lifespan;
        o->hp = h->hp;
        o->stamina = h->stamina;
        o->age = h->age;
        a = h->fly;
        x_h = x;
        y_h = y;
        bool p = false;
        while (a != 0) {
            switch (r(gen)) {
            case 1:
                if ((x_h + 1) >= this->x) break;
                x_h++;
                a--;
                p = true;
                break;
            case 2:
                if ((x_h - 1) < 0) break;
                x_h--;
                a--;
                p = true;
                break;
            case 3:
                if ((y_h + 1) >= this->x) break;
                y_h++;
                a--;
                p = true;
                break;
            case 4:
                if ((y_h - 1) < 0)break;
                y_h--;
                a--;
                p = true;
                break;
            }
            if (p == true) {
                if (h->age == h->lifespan) {
                    this->lives--;
                    living = false;
                    break;
                }
                else if (trap[x_h][y_h] == true) {
                    h->hp--;
                    if (h->hp == 0) {
                        this->lives--;
                        living = false;
                        break;
                    }
                }
                if (life[x_h][y_h].length > 0&&((x_h!=x)||(y_h!=y))) {
                    this->lives += h->baby;
                    Creature* aa = this->life[x_h][y_h].head;
                    uniform_int_distribution<int> rf((aa->fly < h->fly ? aa->fly : h->fly), (aa->fly >= h->fly ? aa->fly : h->fly));
                    uniform_int_distribution<int> rl((aa->lifespan < h->lifespan ? aa->lifespan : h->lifespan), (aa->lifespan >= h->lifespan ? aa->lifespan : h->lifespan));
                    uniform_int_distribution<int> rb((aa->baby < h->baby ? aa->baby : h->baby), (aa->baby >= h->baby ? aa->baby : h->baby));
                    uniform_int_distribution<int> rs((aa->stamina < h->stamina ? aa->stamina : h->stamina), (aa->stamina >= h->stamina ? aa->stamina : h->stamina));
                    for (int j = 0; j < h->baby; j++) {
                        Creature* z = new Creature();
                        z->fly = rf(gen);
                        z->lifespan = rl(gen);
                        z->baby = rb(gen);
                        z->stamina = rs(gen);
                        z->hp = z->stamina;
                        life[x_h][y_h].add(z);
                    }
                    q.push(pair<int, int>(x_h, y_h));
                }
                else if (life[x_h][y_h].length > 1 && (x_h == 0) && (y_h == 0)) {
                    this->lives += h->baby;
                    Creature* aa = this->life[x_h][y_h].head->next;
                    uniform_int_distribution<int> rf((aa->fly < h->fly ? aa->fly : h->fly), (aa->fly >= h->fly ? aa->fly : h->fly));
                    uniform_int_distribution<int> rl((aa->lifespan < h->lifespan ? aa->lifespan : h->lifespan), (aa->lifespan >= h->lifespan ? aa->lifespan : h->lifespan));
                    uniform_int_distribution<int> rb((aa->baby < h->baby ? aa->baby : h->baby), (aa->baby >= h->baby ? aa->baby : h->baby));
                    uniform_int_distribution<int> rs((aa->stamina < h->stamina ? aa->stamina : h->stamina), (aa->stamina >= h->stamina ? aa->stamina : h->stamina));
                    for (int j = 0; j < h->baby; j++) {
                        Creature* z = new Creature();
                        z->fly = rf(gen);
                        z->lifespan = rl(gen);
                        z->baby = rb(gen);
                        z->stamina = rs(gen);
                        z->hp = z->stamina;
                        life[x_h][y_h].add(z); 
                    }
                    q.push(pair<int, int>(x_h, y_h));
                }
            }
            p = false;
        }

        h = h->next;
        if (living == true) {
            life[x_h][y_h].add(o);
            this->q.push(pair<int, int>(x_h, y_h));
        }
        life[x][y].del(1);

    }
}

void Simulator::print_mean() {
    int f = 0, b = 0, l = 0, s = 0,x,y,qs=q.size();
    for (int i = 0; i < qs;i++) {
        pair<int, int> p = q.front();
        q.pop();
        x = p.first;
        y = p.second;
        q.push(pair<int, int>(x, y));
        if (this->check[x][y] == true) {
            continue;
        }
        check[x][y] = true;
        Creature* h = life[x][y].head;
        for (int j = 0; j < life[x][y].length; j++) {
            f += h->fly;
            b += h->baby;
            l += h->lifespan;
            s += h->stamina;
            cout << h->fly << " " << h->baby << " " << (h->lifespan-h->age) << " " << h->hp << endl;
            h = h->next;
        }
    }
    printf("평균적으로 움직일 수 있는 칸의 수 : %.2f\n평균적으로 낳는 아이의 최대 수 : %.2f\n평균 수명 : %.2f\n평균 체력 : %.2f\n", ((double)f / lives), ((double)b / lives), ((double) l / lives), ((double)s /lives));
    this->make_up();
}

Simulator::Simulator(int n, int t, int b) {
    this->lives = n;
    this->x = b;
    random_device rd;
    mt19937 gen(rd());

    this->life = new Life * [b];
    for (int i = 0; i < b; i++) {
        this->life[i] = new Life[b];
    }
    this->board = new int* [b];
    for (int i = 0; i < b; i++) {
        this->board[i] = new int[b];
    }
    this->check = new bool* [b];
    for (int i = 0; i < b; i++) {
        check[i] = new bool[b];
    }
    this->trap = new bool* [b];
    for (int i = 0; i < b; i++) {
        this->trap[i] = new bool[b];
    }
    cout << "Creature가 한 번에 최대로 움직일 수 있는 칸의 수는?" << endl;
    cout << f << endl;
    cout << "Creature의 수명은?" << endl;
    cout << l << endl;
    cout << "Creature가 한 번에 낳을 수 있는 최대 자식의 수는?" << endl;
    cout << bb << endl;
    cout << "Creature가 함정과 마주쳤을 때 버틸 수 있는 최대 횟수는?" << endl;
    cout << s << endl;
    uniform_int_distribution<int> rf(1, f);
    uniform_int_distribution<int> rl(1, l);
    uniform_int_distribution<int> rb(1, bb);
    uniform_int_distribution<int> rs(1, s);
    uniform_int_distribution<int> rx(0, (b-1));
    uniform_int_distribution<int> ry(0, (b - 1));
    int x, y;
    for (int i = 0; i < b; i++) {
        for (int j = 0; j < b; j++) {
            this->trap[i][j] = false;
        }
    }
    for (int i = 0; i < t;) {
        x = rx(gen);
        y = ry(gen);
        if (this->trap[x][y] == false) {
            this->trap[x][y] = true;
            i++;
        }
    }
    for (int i = 0; i < n; i++) {
        Creature* c = new Creature();
        c->fly = rf(gen);
        c->baby = rb(gen);
        c->stamina = rs(gen);
        c->hp = c->stamina;
        c->lifespan = rl(gen);
        x = rx(gen);
        y = ry(gen);
        this->life[x][y].add(c);
        this->q.push(pair<int, int>(x, y));
    }
    this->make_up();
}
void Simulator::make_up() {
    for (int i = 0; i < this->x; i++) {
        for (int j = 0; j < this->x; j++) {
            this->board[i][j] = this->life[i][j].length;
            this->check[i][j] = false;
        }
    }
}
int main()
{
    int n=10, t, x=10;
    cout << "시작할 때의 Creature 수는?" << endl;
    cout << n << endl;
    cout << "총 함정의 개수는?" << endl;
    cin >> t;
    cout << "Creature들이 살아갈 터전의 한변의 길이는?" << endl;
    cout << x << endl;
    cout << "---------------------------" << endl;
    Simulator s = Simulator(n,t,x);
    s.print_status();
    s.print_mean();
    Sleep(2000);
    s.turn(10);
}

void print_queue(queue<pair<int,int>> q) {
    while (!q.empty()) {
        cout << q.front().first << " " << q.front().second << " ";
        q.pop();
    }
    cout << endl;
}
