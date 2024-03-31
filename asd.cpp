#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

mutex mutex1;

int p_num_threads = 2;
int c_num_threads = 2;

int hour_ind = 48;
int ccount = 0;
int con_count = 0;
int m = 0;

condition_variable producer_cv, consumer_cv;

string ind, t_stamp, tr_light_id, no_of_cars;
vector<int> in;
vector<int> tr_light;
vector<int> no_cars;
vector<string> tstamp;

struct TrafficSignal {
    int ind;
    string t_stamp;
    int tr_id;
    int num_cars;
};

TrafficSignal tlSorter[4] = {{0, "", 1, 0}, {0, "", 2, 0}, {0, "", 3, 0}, {0, "", 4, 0}};
queue<TrafficSignal> tr_sig_queue;
TrafficSignal sig;

bool sort_method(TrafficSignal first, TrafficSignal second) {
    return (first.num_cars > second.num_cars);
}

void* produce(void* args) {
    while (ccount < m) {
        unique_lock<mutex> lk(mutex1);

        if (ccount < m) {
            tr_sig_queue.push({in[ccount], tstamp[ccount], tr_light[ccount], no_cars[ccount]});
            consumer_cv.notify_all();
            ccount++;
        } else {
            producer_cv.wait(lk, []{ return ccount < m; });
        }

        lk.unlock();
        this_thread::sleep_for(chrono::seconds(rand() % 3));
    }
}

void* consume(void* args) {
    while (con_count < m) {
        unique_lock<mutex> lk(mutex1);

        if (!tr_sig_queue.empty()) {
            sig = tr_sig_queue.front();

            if (sig.tr_id == 1) {
                tlSorter[0].num_cars += sig.num_cars;
            } else if (sig.tr_id == 2) {
                tlSorter[1].num_cars += sig.num_cars;
            } else if (sig.tr_id == 3) {
                tlSorter[2].num_cars += sig.num_cars;
            } else if (sig.tr_id == 4) {
                tlSorter[3].num_cars += sig.num_cars;
            }

            tr_sig_queue.pop();
            producer_cv.notify_all();
            con_count++;
        } else {
            consumer_cv.wait(lk, []{ return !tr_sig_queue.empty(); });
        }

      if (con_count % hour_ind == 0) {
          sort(tlSorter, tlSorter + 4, sort_method);
          cout << "Time: " << sig.t_stamp << endl;
          for (int i = 0; i < 4; i++) {
              cout << "Traffic Light " << tlSorter[i].tr_id << " - " << tlSorter[i].num_cars << " cars" << endl;
          }
      }

        lk.unlock();
        this_thread::sleep_for(chrono::seconds(rand() % 3));
    }
}

void get_traff_data() {
    ifstream infile;

    string file = "test-data.csv";

    infile.open(file);

    if (infile.is_open()) {
        string line;
        getline(infile, line);

        while (!infile.eof()) {
            getline(infile, ind, ',');
            in.push_back(stoi(ind));
            getline(infile, t_stamp, ',');
            tstamp.push_back(t_stamp);
            getline(infile, tr_light_id, ',');
            tr_light.push_back(stoi(tr_light_id));
            getline(infile, no_of_cars, '\n');
            no_cars.push_back(stoi(no_of_cars));

            m += 1;
        }
        infile.close();
    } else {
        cout << "File not found" << endl;
    }
}

int main() {
    srand(time(0));
    get_traff_data();

    vector<thread> producers(p_num_threads);
    vector<thread> consumers(c_num_threads);

    for (int i = 0; i < p_num_threads; i++) {
        producers[i] = thread(produce, (void*)i);
    }
    for (int i = 0; i < c_num_threads; i++) {
        consumers[i] = thread(consume, (void*)i);
    }

    for (int i = 0; i < p_num_threads; i++) {
        producers[i].join();
    }
    for (int i = 0; i < c_num_threads; i++) {
        consumers[i].join();
    }

    return 0;
}
