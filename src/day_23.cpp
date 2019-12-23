#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include "common.h"
#include "intcode.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <thread>

using namespace std;
using namespace boost::multiprecision;
using namespace boost;


/**
 * Day 23 - Category Six
 *
 * Setting up a thread for each NIC Intcode program
 *
 * Somehow the input queue read (getInputValue) needs to sleep for some ms
 * if no input was in the queue to make this work - why?
 * I have no idea...
 */

// Forward declaration of Nic:
class Nic;
class NAT;

// Our nic container
vector<std::shared_ptr<Nic>> nics;

class NAT {
    private:
        std::mutex m;
        cpp_int packetX = 0;
        cpp_int packetY = 0;
        cpp_int lastYValueSent = 0;

    public:
        void sendPacket(cpp_int x, cpp_int y) {
            m.lock();
            cout << "NAT has received packet: " << x << ":" << y << endl;
            packetX = x;
            packetY = y;
            m.unlock();
        }

    void checkIdle();
};


// The NAT:
NAT nat;

class Nic : public Program
{
    private:
        std::mutex m;

    public:

        Nic(memory_t *mem) : Program(mem) { }

        /**
         * Override getInputValue: If the queue is empty, provide -1 as input value
         *
         * Thread-safe read, and also sleeps for some ms when requesting empty values.
         * Why? No idea - without the sleep it just did not work...
         */
        cpp_int getInputValue() override {
            cpp_int val;
            m.lock();
            if (this->inputValues.empty()) {
                val = cpp_int(-1);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            } else {
                val = this->inputValues.front();
                this->inputValues.pop();
            }
            m.unlock();
            return val;
        }

        bool isIdle()
        {
            bool idle = false;
            m.lock();
            if (this->inputValues.empty()) {
                idle = true;
            }
            m.unlock();
            return idle;
        }

        /**
         * thread-safe way to put things in queue, so that the items are in-order
         */
        void pushInputValues(cpp_int xVal, cpp_int yVal) {
            m.lock();
            this->inputValues.push(xVal);
            this->inputValues.push(yVal);
            m.unlock();
        }

        /**
         * Thread function, used by Thread to start its work
         *
         * Here, we constantly run the program and wait for its output values, sending
         * them to other threads
         */
        void operator()()
        {
            while (true) {
                // 1st output: dest address
                this->runProgram();
                auto destAddr = this->output;

                // 2nd output: x value
                this->runProgram();
                auto xVal = this->output;

                // 3rd output: y value
                this->runProgram();
                auto yVal = this->output;
                // cout << "NIC #" << this->pNr << ": Received values: " << destAddr << ":" << xVal << ":" << yVal <<endl;
                // Have we reached solution 1?
                if (destAddr == 255) {
                    cout << "Solution 1: Y Value sent to Addr 255: " << yVal << endl;
                    // Send value to NAT:
                    nat.sendPacket(xVal, yVal);
                }

                // Send the packet to its destination:
                if (destAddr>= 0 && destAddr < nics.size()) {
                    cout << "NIC #" << this->pNr << ": Send values to NIC # " << destAddr << ":" << xVal << ":" << yVal <<endl;
                    auto addr = static_cast<unsigned int>(destAddr);
                    (nics[addr])->pushInputValues(xVal, yVal);
                }
            }
        }
};

void NAT::checkIdle()
{
    set<cpp_int> seenValues;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        cout << "NAT is looking for idleness ..." << endl;
        bool idle = true;
        if (packetX == 0 && packetY == 0) continue;
        for (auto n : nics) {
            if (n->isIdle() != true)
            {
                idle = false;
                break;
            }
        }
        if (idle) {
            cout << "NAT Detected Idleness... So sad! Send " << packetX << ":" << packetY << " to 0." << endl;
            if (seenValues.count(packetY) > 0) {
                cout << "NAT detected same Y value in a row: " << packetY << endl;
                break;
            }
            seenValues.insert(packetY);
            (nics[0])->pushInputValues(packetX, packetY);
            packetX = 0;
            packetY = 0;
        }
    }
}


void solution1(memory_t &data)
{
    // Create NIC programs
    vector<std::shared_ptr<std::thread>> threads;
    for (unsigned int i = 0; i < 50; i++) {
        auto nic = std::make_shared<Nic>(&data);
        cout << "Nic " << i << " program size: " << nic->memory->size() << endl;
        nic->pNr = i;
        nic->inputValues.push(cpp_int(i));
        nics.push_back(nic);
        cout << "Starting nic #" << nic->pNr << endl;
        threads.push_back(std::make_shared<std::thread>(std::ref(*nic)));
    }

    nat.checkIdle();

    // (threads[0])->join();
}

int main(int argc, char *args[])
{
    // bool display = argc >= 3 && string("--display").compare(args[2]) == 0 ? true : false;
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    memory_t data;
    readIntcodeProgramFromFile(args[1], data);

    solution1(data);
    // solution2(data);
}
