#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <latch>
#include <syncstream>

using namespace std;

// Варіант 6: схема №6, nt=5, a=9, b=5, c=7, d=8, e=9, f=5, g=5, h=9, i=6, j=7
//
// Залежності схеми №6:
//   j  - після всіх a, d
//   h  - після всіх b, e
//   i  - після всіх c, g
//   f  - після всіх j, h, i

const int nt = 5;
const int A = 9, B = 5, C = 7, D = 8, E = 9, F = 5, G = 5, H = 9, I = 6, J = 7;

void f(char x, int i)
{
    this_thread::sleep_for(1s);
    osyncstream(cout) << "З набору " << x << " виконано дію " << i << ".\n";
}

int main()
{

    latch done_ad(A + D);
    latch done_be(B + E);
    latch done_cg(C + G);
    latch done_jhi(J + H + I);  

    vector<vector<pair<char, int>>> plan = {
        {{'a',1}, {'a',6}, {'d',2}, {'d',7}, {'b',4}, {'e',4}, {'e',9}, {'g',5}, {'c',5}, {'j',3}, {'h',1}, {'h',6}, {'i',2}, {'f',1}},
        {{'a',2}, {'a',7}, {'d',3}, {'d',8}, {'b',5}, {'e',5}, {'g',1}, {'c',1}, {'c',6}, {'j',4}, {'h',2}, {'h',7}, {'i',3}, {'f',2}},
        {{'a',3}, {'a',8}, {'d',4}, {'b',1}, {'e',1}, {'e',6}, {'g',2}, {'c',2}, {'c',7}, {'j',5}, {'h',3}, {'h',8}, {'i',4}, {'f',3}},
        {{'a',4}, {'a',9}, {'d',5}, {'b',2}, {'e',2}, {'e',7}, {'g',3}, {'c',3}, {'j',1}, {'j',6}, {'h',4}, {'h',9}, {'i',5}, {'f',4}},
        {{'a',5}, {'d',1}, {'d',6}, {'b',3}, {'e',3}, {'e',8}, {'g',4}, {'c',4}, {'j',2}, {'j',7}, {'h',5}, {'i',1}, {'i',6}, {'f',5}},
    };

    auto worker = [&](int t) {
        for (auto [x, i] : plan[t])
        {
            // очікування завершення попередніх наборів
            switch (x)
            {
                case 'j': done_ad.wait(); break;
                case 'h': done_be.wait(); break;
                case 'i': done_cg.wait(); break;
                case 'f': done_jhi.wait(); break;
            }

            f(x, i);

            // повідомлення про виконання дії
            switch (x)
            {
                case 'a': case 'd': done_ad.count_down(); break;
                case 'b': case 'e': done_be.count_down(); break;
                case 'c': case 'g': done_cg.count_down(); break;
                case 'j': case 'h': case 'i': done_jhi.count_down(); break;
            }
        }
    };

    cout << "Обчислення розпочато." << endl;
    auto start_time = chrono::steady_clock::now();

    {
        vector<jthread> vt;
        for (int t = 0; t < nt; t++)
        {
            vt.emplace_back(worker, t);
        }
    }

    chrono::duration<double> d = chrono::steady_clock::now() - start_time;
    cout << "Обчислення завершено. Тривалість обчислення " << d.count() << " секунд." << endl;
}
