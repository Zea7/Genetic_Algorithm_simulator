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

class Life {                                    //한 칸에 있는 creature들을 linked-list의 형태로 저장하기 위해
public:                                         //creature를 node로 가지는 linked-list class, Life를 정의
    int length=0;
    Creature* head = NULL;
    Creature* tail = NULL;
    void add(Creature* n);
    Creature del(int n);
};

void Life::add(Creature* n) {                   //Creature 추가하는 함수
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

Creature Life::del(int n) {                     //칸에 있는 creature들 중 n 번째로 칸에 들어온 creature를 제거하고, 이를 반환
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

class Simulator {                       //Simulator class. 본 프로그램의 본체
private:
    Life** life;                       //각 칸의 생물체들의 집합
    int** board;                       //생물체들이 살아갈 공간(정사각형)
    bool** check;                      //이미 시행한 칸인지 확인
    bool** trap;                       //함정의 위치
    int x;                             //board 한 변의 길이
    int lives;                         //총 creature의 수
    int act;                           //진행한 시행의 수
    int f = 10;                        //최대 fly
    int l = 2;                         //최대 lifespan
    int bb = 2;                        //최대 baby
    int s = 1;                         //최대 stamina
    queue<pair<int, int>> q;           //Creature들이 존재하는 좌표값
public:
    Simulator(int n, int t, int b);                   //생성자(n은 시작시 creature의 수, t는 함정의 수, b는 board 한 변의 길이
    Simulator(int n, int t, int b, bool f);            //모든 기본 값을 변경하는 생성자.
    void turn(int n, bool b);                   //한 번의시뮬레이션 
    void print_mean(bool b);                  //현재 생존해 있는 creature들의 status의 평균 출력
    void move(int x, int y);            //(x,y) 좌표의 creature들이 1~fly 칸만큼 움직임.
    void print_status();                //디버깅용 함수
    void make_up();                     //시행마다 배열들을 정리
};

void Simulator::turn(int n,bool b) {           //한번한번의 시행마다 할 수행을 정의한 함수. recursion의 형태로 정의되어있다.
    if (n == 0) return;                         //end state
    act++;    
    int qs = q.size();

    for (int i = 0; i < qs;i++) {               //queue 내에 있는 좌표들에서만 move함수를 실행
        pair<int, int> r = q.front();
        q.pop();                                //queue 내에 있는 좌표에서 move 실행 이후에는 해당 좌표가 필요 없으므로 queue에서 삭제(새로운 좌표가 queue에 들어감)
        int x = r.first, y = r.second;
        if (this->check[x][y] == true) continue;
        this->move(x, y);                               //move 실행
    }
    this->make_up();                                    //move 이후 출력을 위하여 배열들 재정리
    cout << "---------------------------" << endl;      //turn 마다 현재 상황 출력
    cout << this->act << "번째 시행 : " << endl;
    this->print_mean(b);
    this->print_status();
    cout << "---------------------------" << endl;
    Sleep(2000);
    this->turn(--n,b);                                  //recursion
}

void Simulator::print_status() {                        //Board를 출력함으로써 creature들이 어떤 곳에 분포해 있는지 확인
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
    if (number != this->lives) {                        //디버깅용. 내부 충돌로 인해 class내의 변수인 lives로 저장된 creature 수와 board내의 creature수가 맞지 않으면
        cout << "오류가 있음" << endl;                  //시뮬레이션에 오류가 있다 판단, 시뮬레이션을 종료한다.
        exit(0);
    }
}

void Simulator::move(int x, int y) {                    //creature들을 턴마다 움직이는 함수
    if (this->check[x][y] == true) return;              //이미 move 함수를 실행한 칸일 경우, 스킵한다.
    this->check[x][y] = true;                           //해당 칸이 move 함수를 실행했음을 저장해줌
    int n = this->board[x][y],a,x_h,y_h;                //해당 칸에 몇개의 creature가 있는 지를 n에 저장, 각 creature들의 실시간 좌표를 x_h, y_h에 저장
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> r(1, 4);              //위 아래 오른쪽 왼쪽으로 나누어 한칸씩 움직이도록 함.
    Creature* h = this->life[x][y].head;                //creature list의 head를 가져옴
    for (int i = 1; i <= n; i++) {                      
        bool living = true;                             //해당 creature가 움직이는 도중 사망할지를 확인.
        h->age++;                                       //move를 시작함과 동시에 trun이 시작한 것이므로, 나이를 먹음
        Creature* o = new Creature();                   //move가 끝난 이후, 해당 creature가 갈 위치의 list에 creature의 정보를 저장하기 위해, creature의 정보를 복제.
        o->fly = h->fly;
        o->baby = h->baby;
        o->lifespan = h->lifespan;
        o->hp = h->hp;
        o->stamina = h->stamina;
        o->age = h->age;
        a = h->fly;
        x_h = x;                                        //시작 위치
        y_h = y;
        bool p = false;                                 //움직인 위치가 board내인지 아닌지를 판단해줄 변수
        while (a != 0) {                                //a에 최대 움직일 수 있는 칸을 저장, 0이 되기 전까지 1씩 줄이며 움직임
            switch (r(gen)) {                           //r의 랜더머 결과를 통해 움직일 방향 결정
            case 1:                                     //움직일 수 있는 곳이 board내일 경우, 움직여주고 a--를 해주며, p를 true로 바꾸어 움직였음을 확인
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
            if (p == true) {                                //움직이는 도중 해당 칸에 특이사항이 있을 경우 이를 실행
                if (h->age == h->lifespan) {                //만약 나이가 최대수명에 도달할시, 사망 처리
                    this->lives--;
                    living = false;
                    break;
                }
                else if (trap[x_h][y_h] == true) {          //함정에 걸릴 시, 체력을 깎고,
                    h->hp--;
                    if (h->hp == -1) {                       //체력이 다 떨어지면 사망처리
                        this->lives--;
                        living = false;
                        break;
                    }
                }
                if (life[x_h][y_h].length > 0&&((x_h!=x)||(y_h!=y))) { //움직이던 중, 해당 칸에 다른 creature가 있을 시, 자식을 생성(원래 있던 칸이 아닌경우)
                    this->lives += h->baby;
                    Creature* aa = this->life[x_h][y_h].head;
                    uniform_int_distribution<int> rf((aa->fly < h->fly ? aa->fly : h->fly), (aa->fly >= h->fly ? aa->fly : h->fly));
                    uniform_int_distribution<int> rl((aa->lifespan < h->lifespan ? aa->lifespan : h->lifespan), (aa->lifespan >= h->lifespan ? aa->lifespan : h->lifespan));
                    uniform_int_distribution<int> rb((aa->baby < h->baby ? aa->baby : h->baby), (aa->baby >= h->baby ? aa->baby : h->baby));
                    uniform_int_distribution<int> rs((aa->stamina < h->stamina ? aa->stamina : h->stamina), (aa->stamina >= h->stamina ? aa->stamina : h->stamina));
                    uniform_int_distribution<int> rm(0, 10);
                    uniform_int_distribution<int> rmf(1, this->f);
                    uniform_int_distribution<int> rml(1, this->l);
                    uniform_int_distribution<int> rmb(1, this->bb);
                    uniform_int_distribution<int> rms(1, this->s);
                    for (int j = 0; j < h->baby; j++) {
                        Creature* z = new Creature();
                        int mm = rm(gen);
                        if (mm == 1) {
                            z->fly = rmf(gen);
                            z->lifespan = rml(gen);
                            z->baby = rmb(gen);
                            z->stamina = rms(gen);
                            z->hp = z->stamina;
                        }
                        else {
                            z->fly = rf(gen);
                            z->lifespan = rl(gen);
                            z->baby = rb(gen);
                            z->stamina = rs(gen);
                            z->hp = z->stamina;
                        }
                        life[x_h][y_h].add(z);
                    }
                    q.push(pair<int, int>(x_h, y_h));    //생성한 자식의 위치를 queue에 저장해줌
                }
                else if (life[x_h][y_h].length > 1 && (x_h == 0) && (y_h == 0)) { //움직이던 중, 해당 칸에 다른 creature가 있을 시, 자식을 생성(원래 있던 칸인경우)
                    this->lives += h->baby;
                    Creature* aa = this->life[x_h][y_h].head->next;
                    uniform_int_distribution<int> rf((aa->fly < h->fly ? aa->fly : h->fly), (aa->fly >= h->fly ? aa->fly : h->fly));
                    uniform_int_distribution<int> rl((aa->lifespan < h->lifespan ? aa->lifespan : h->lifespan), (aa->lifespan >= h->lifespan ? aa->lifespan : h->lifespan));
                    uniform_int_distribution<int> rb((aa->baby < h->baby ? aa->baby : h->baby), (aa->baby >= h->baby ? aa->baby : h->baby));
                    uniform_int_distribution<int> rs((aa->stamina < h->stamina ? aa->stamina : h->stamina), (aa->stamina >= h->stamina ? aa->stamina : h->stamina));
                    uniform_int_distribution<int> rm(0, 10);
                    uniform_int_distribution<int> rmf(1, this->f);
                    uniform_int_distribution<int> rml(1, this->l);
                    uniform_int_distribution<int> rmb(1, this->bb);
                    uniform_int_distribution<int> rms(1, this->s);
                    for (int j = 0; j < h->baby; j++) {
                        Creature* z = new Creature();
                        int mm = rm(gen);
                        if (mm == 1) {
                            z->fly = rmf(gen);
                            z->lifespan = rml(gen);
                            z->baby = rmb(gen);
                            z->stamina = rms(gen);
                            z->hp = z->stamina;
                        }
                        else {
                            z->fly = rf(gen);
                            z->lifespan = rl(gen);
                            z->baby = rb(gen);
                            z->stamina = rs(gen);
                            z->hp = z->stamina;
                        }
                        life[x_h][y_h].add(z);
                    }
                    q.push(pair<int, int>(x_h, y_h));
                }
            }
            p = false;
        }

        h = h->next;    
        if (living == true) {               //해당 creautre가 죽지 않았다면,
            life[x_h][y_h].add(o);              //이동해준 좌표의 creature list에 해당 creature를 복제한 object를 저장
            this->q.push(pair<int, int>(x_h, y_h));             //queue에 이동해준 좌표를 저장하고
        }
        life[x][y].del(1);                  //원래 있던 위치에서 해당 creature를 제거

    }
}

void Simulator::print_mean(bool t) {            //현재 살아있는 creature들의 status의 평균을 출력
    int f = 0, b = 0, l = 0, s = 0,x,y,qs=q.size();
    if (t) cout << "최대 움직일 수 있는 칸    최대 자식의 수     남은 수명      현재 남은 체력" << endl;
    for (int i = 0; i < qs;i++) {
        pair<int, int> p = q.front();           //creature들이 존재하는 위치를 저장하는 queue에서 위치 정보를 하나씩 받아옴
        q.pop();
        x = p.first;
        y = p.second;
        q.push(pair<int, int>(x, y));
        if (this->check[x][y] == true) {        //이미 출력한 칸은 스킵
            continue;
        }
        check[x][y] = true;
        Creature* h = life[x][y].head;
        
        for (int j = 0; j < life[x][y].length; j++) {
            f += h->fly;
            b += h->baby;
            l += h->lifespan;
            s += h->stamina;
            if(t) cout << h->fly << " " << h->baby << " " << (h->lifespan-h->age) << " " << h->hp << endl;
            h = h->next;
        }
    }
    printf("평균적으로 움직일 수 있는 칸의 수 : %.2f\n평균적으로 낳는 아이의 최대 수 : %.2f\n평균 수명 : %.2f\n평균 체력 : %.2f\n", ((double)f / lives), ((double)b / lives), ((double) l / lives), ((double)s /lives));
    this->make_up();                        //check를 썼으므로, 배열들을 재정리
}

Simulator::Simulator(int n, int t, int b) {
    this->lives = n;
    this->x = b;
    random_device rd;
    mt19937 gen(rd());
    //배열들 초기화
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
    uniform_int_distribution<int> rf(1, f);             //랜덤으로 creature들 생성
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
    for (int i = 0; i < t;) {               //랜덤으로 함정 설치
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

Simulator::Simulator(int n, int t, int b, bool p) {
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
    cin >> this->f;
    cout << "Creature의 수명은?" << endl;
    cin >> this->l;
    cout << "Creature가 한 번에 낳을 수 있는 최대 자식의 수는?" << endl;
    cin >> this->bb;
    cout << "Creature가 함정과 마주쳤을 때 버틸 수 있는 최대 횟수는?" << endl;
    cin >> this->s;
    uniform_int_distribution<int> rf(1, this->f);
    uniform_int_distribution<int> rl(1, this->l);
    uniform_int_distribution<int> rb(1, this->bb);
    uniform_int_distribution<int> rs(1, this->s);
    uniform_int_distribution<int> rx(0, (b - 1));
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
    int n=40, t, x=10,b=false;
    char a;
    cout << "총 함정의 개수는?" << endl;
    cin >> t;
    Simulator s = Simulator(n, t, x);
    cout << "기본 설정 값(모든 status의 최소는 1) : " << endl;
    cout << "시작 시 creature의 수 : 40" << endl;
    cout << "Creature들이 살아갈 터전의 너비(터전은 정사각형) : 10" << endl;
    cout << "Creature들이 움직일 수 있는 최대 칸 수 : 10" << endl;
    cout << "Creature들의 최대 수명(버틸 수 있는 턴의 수) : 2" << endl;
    cout << "Creature들이 낳을 수 있는 최대 자식의 수 : 2" << endl;
    cout << "Creature들의 최대 체력(함정을 밟고 견딜 수 있는 횟수) : 1" << endl;
    cout << "---------------------------" << endl;
    cout << "기본 설정 값을 바꾸시겠습니까? (y/n)" << endl;
    cin >> a;
    if (a == 'y') {
        cout << "터전의 너비는?" << endl;
        cin >> b;
        cout << "총 함정의 개수는?" << endl;
        cin >> t;
        cout << "시작 시 creature의 수는?" << endl;
        cin >> n;
        Simulator m = Simulator(n, t, x, b);
        s = m;
    }
    cout << "----------------------------" << endl;
    cout << "각 creature들의 status를 턴마다 출력하시겠습니까? (y/n)" << endl;
    cin >> a;
    if (a == 'y') b = true;
    s.print_status();
    s.print_mean(b);
    Sleep(2000);
    s.turn(10,b);
}

void print_queue(queue<pair<int,int>> q) {
    while (!q.empty()) {
        cout << q.front().first << " " << q.front().second << " ";
        q.pop();
    }
    cout << endl;
}
