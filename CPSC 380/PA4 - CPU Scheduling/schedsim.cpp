/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 4 - CPU Scheduling

    This C source code file contains the main function and logic for running a CPU scheduling simulation.
    It supports algorithms like FCFS, SJF, Round Robin, and Priority Scheduling. The program reads
    process data from a CSV file, simulates the scheduling based on the selected algorithm, and outputs a
    Gantt chart along with various performance metrics about how the simulation performed overall and for 
    each process.
*/

#include <iostream>
#include <string.h>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <deque> // Using this for the ready queue instead of regular queue since it's easier for sorting and manipulation at start/end
#include <algorithm> // Need this to sort the queue based on different policies in simulation
#include <climits> // Used for INT_MAX in the metrics to calculate the first arrival time
#include <iomanip> // Needed for std::setw which is used for Gantt chart formatting

// The main data structure representing a process
struct Process {
    // Data for process from the input file
    std::string pid;
    int arrival = 0;
    int burst = 0;
    int priority = 0;

    // Current state of process in the scheduling simulation
    int remaining = 0;
    bool admitted = false;

    // Metrics, mainly will be used later for calculating performance
    int start = -1;
    int finish = -1;
    int waiting = 0;
    int response = 0;
    int turnaround = 0;

    // Synchronization thread and semaphores
    pthread_t thread;
    sem_t run_one; // Scheduler lets process run one tick
    sem_t yielded; // When the process is finished, tell scheduler
};

std::vector<Process*> procs;
std::deque<Process*> ready;
pthread_mutex_t mutex;

int clk = 0; // Used for the simulation
Process* current = nullptr;

void *ProcessThread(void* arg) {
    Process* proc = (Process*) arg;

    while (true) {
        // Wait for scheduler to let it run one tick
        sem_wait(&proc->run_one);

        pthread_mutex_lock(&mutex);

        // Update remaining time
        if (proc->remaining > 0) {
            proc->remaining--;
        }

        pthread_mutex_unlock(&mutex);

        // yield back to scheduler; one tick is done
        sem_post(&proc->yielded);

        if (proc->remaining <= 0) {
            break;
        }
    }

    return nullptr;
}

int main(int argc, char* argv[]) {
    std::string workloadFile;
    std::string policy = "FCFS";
    int quantum = 1;

    // Define the various command line arguments possible for running this program
    static struct option long_options[] = {
        {"fcfs", no_argument, 0, 'f'},
        {"sjf", no_argument, 0, 's'},
        {"rr", no_argument, 0, 'r'},
        {"priority", no_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'},
        {"quantum", required_argument, 0, 'q'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option;

    // Parse each of the arguments using getopt_long() function 
    while ((option = getopt_long(argc, argv, "fsrpi:q:h", long_options, NULL)) != -1) {
        switch (option) {
            case 'f':
                policy = "FCFS";

                break;
            case 's':
                policy = "SJF";

                break;
            case 'r':
                policy = "RR";

                break;
            case 'p':
                policy = "PRIORITY";

                break;
            case 'i':
                workloadFile = optarg;
                
                break;
            case 'q':
                quantum = atoi(optarg);

                break;
            case '?':
                // getopt_long already printed an error message

                break;
            case 'h': // Show the help message as default if no arguments are provided
            default:
                fprintf(stderr,
                    "Usage: %s [options]\n"
                    "Options:\n"
                    "-f, --fcfs, Use FCFS scheduling (default policy)\n"
                    "-s, --sjf, Use SJF scheduling\n"
                    "-r, --rr, Use Round Robin scheduling\n"
                    "-p, --priority, Use Priority scheduling\n"
                    "-i, --input <FILE>, Input CSV filename\n"
                    "-q, --quantum <N>, Time quantum for RR (default 1)\n"
                    "-h, --help Show this help message\n",
                    argv[0]);

                exit(0);
        }
    }

    // Make sure user provided an input file for the tasks
    if (workloadFile.empty()) {
        std::cout << "--input <FILE> is required\n";

        return 1;
    }

    std::ifstream infile(workloadFile);

    // Try to open the file provided by the user, otherwise exit if it fails
    if (!infile.is_open()) {
        std::cout << "Unable to open file " << workloadFile << "\n";

        return 1;
    }

    std::string line;

    // Skip the first line because that should only contain the column names (header)
    std::getline(infile, line);

    // Process each line separately of the input file
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string pid, arrival, burst, priority;

        // Load each of the values from one line into their respective variable from the stream
        std::getline(ss, pid, ',');
        std::getline(ss, arrival, ',');
        std::getline(ss, burst, ',');
        std::getline(ss, priority, ',');

        // Create a new process struct and load the info we have read from the file
        Process* proc = new Process;
        proc->pid = pid;
        proc->arrival = std::stoi(arrival);
        proc->burst = std::stoi(burst);
        proc->priority = std::stoi(priority);
        proc->remaining = proc->burst;

        // Initialize the semaphores for this process
        sem_init(&proc->run_one, 0, 0);
        sem_init(&proc->yielded, 0, 0);

        // Add the process to the list of processes to be scheduled
        procs.push_back(proc);
    }

    // Create a thread for each process to be scheduled
    for (Process* proc : procs) {
        pthread_create(&proc->thread, nullptr, ProcessThread, proc);
    }

    // Gantt chart for the end metrics (pair<time, label>)
    std::vector<std::pair<int,std::string>> gantt;

    // Variables for the simulation loop, used for seeing if we've run all processes and for RR quantum tracking / busy time
    int finished = 0;
    int rrQuantum = 0;
    int busyTicks = 0;

    // The main simulation loop
    while (finished < (int)procs.size()) {
        // 1. Check for Arrivals
        for (Process* proc : procs) {
            if (!proc->admitted && proc->arrival <= clk) {
                proc->admitted = true;

                ready.push_back(proc);
            }
        }

        // Check if current process is complete (this is sort of similar to step 4 in the implementation details)
        if (current && current->remaining == 0) {
            current->finish = clk;
            current = nullptr;

            rrQuantum = 0;
            finished++;
        }

        // Preemption check for Priority Scheduling
        bool higherPriority = false;

        if (!current) {
            higherPriority = !ready.empty();
        } else {
            for (Process* proc : ready) {
                if (proc->priority < current->priority) {
                    higherPriority = true;
                }
            }
        }

        if (policy == "PRIORITY" && current && higherPriority) {
            ready.push_back(current);
            current = nullptr;
            rrQuantum = 0;
        }

        // Preemption check for Round Robin Scheduling
        if (policy == "RR" && current && rrQuantum >= quantum) {
            ready.push_back(current);
            current = nullptr;
            rrQuantum = 0;
        }

        // 2. Select the Next Process
        if (!current && !ready.empty()) {
            // Sort the ready queue based on the selected policy, but FCFS and RR can keep insertion order since it's a regular queue
            if (policy == "SJF") {
                std::sort(ready.begin(), ready.end(), [](Process* a, Process* b) { return a->remaining < b->remaining; });
            } else if (policy == "PRIORITY") {
                std::sort(ready.begin(), ready.end(), [](Process* a, Process* b) { return a->priority < b->priority; });
            }

            current = ready.front(); 
            ready.pop_front();

            if (current->start == -1) {
                current->start = clk;
            }

            rrQuantum = 0;
        }

        // 3. Dispatch the Process
        if (current) {
            if (gantt.empty() || gantt.back().second != current->pid) {
                gantt.emplace_back(clk, current->pid);
            }

            // Give one tick to the current process
            sem_post(&current->run_one);
            sem_wait(&current->yielded);

            // Update current stats
            rrQuantum++;
            busyTicks++;

            // All the other ready tasks should increase waiting time
            for (Process* proc : ready) {
                if (proc->admitted && proc->remaining > 0) {
                    proc->waiting++;
                }
            }
        } else { // If no current process, that means CPU is idling
            // Show where the CPU is idiling in the Gantt chart
            if (gantt.empty() || gantt.back().second != "IDLE") {
                gantt.emplace_back(clk, "IDLE"); // IDLE wont actually be displayed in the output, it's just so that I can show the end numbers correctly
            }

            // All the other ready tasks should increase waiting time
            for (Process* proc : ready) {
                if (proc->admitted && proc->remaining > 0) {
                    proc->waiting++;
                }
            }
        }

        // Advance clk tick
        clk++;
    }

    // Join threads
    for (Process* proc : procs) {
        pthread_join(proc->thread, nullptr);
    }

    // Metrics
    double avgWait = 0, avgResponse = 0, avgTurnaround = 0;
    int firstArrival = INT_MAX, lastFinish = 0;

    for (Process* proc : procs) {
        proc->turnaround = proc->finish - proc->arrival;
        proc->response = proc->start - proc->arrival;

        avgWait += proc->waiting;
        avgResponse += proc->response;
        avgTurnaround += proc->turnaround;

        firstArrival = std::min(firstArrival, proc->arrival);
        lastFinish = std::max(lastFinish, proc->finish);
    }

    // Total simulation time can be calculated by difference between first arrival to last finish
    int totalTime = std::max(0, lastFinish - firstArrival);
    double throughput = (double)procs.size() / totalTime;
    double cpuUtilization = 100.0 * busyTicks / totalTime;

    std::cout << "=====" << policy << " SCHEDULING=====\n";

    // Gantt chart output, got help with formatting the various pipes and whatnot of the Gantt chart from AI
    std::cout << "Timeline (Gantt Chart):\n";

    // Merge consecutive same process entries and skip any idle entries
    std::vector<std::pair<int, std::string>> merged;
    for (int i = 0; i < gantt.size(); i++) {
        if (gantt[i].second != "IDLE" && (merged.empty() || merged.back().second != gantt[i].second)) {
            merged.push_back(gantt[i]);
        }
    }

    // Compute segment end times
    std::vector<int> end;
    for (int i = 0; i < merged.size(); i++) {
        if (i + 1 < merged.size()) {
            end.push_back(merged[i + 1].first);
        } else {
            end.push_back(lastFinish);
        }
    }

    // Print time labels
    std::cout << merged[0].first;
    for (int i = 0; i < merged.size(); i++) {
        std::cout << std::setw(8 + 1) << end[i];
    }

    std::cout << "\n";

    // Print top bar
    for (int i = 0; i < merged.size(); i++) {
        std::cout << "|" << std::string(8, '-');
    }
    
    std::cout << "|\n";

    // Print process labels
    for (int i = 0; i < merged.size(); i++) {
        std::cout << "|" << std::setw(4) << merged[i].second << std::setw(4) << "";
    }

    std::cout << "|\n";

    // Print bottom bar
    for (int i = 0; i < merged.size(); i++) {
        std::cout << "|" << std::string(8, '-');
    }

    std::cout << "|\n";

    // The actual numerical metrics output
    std::cout << std::left << std::setw(6) << "PID" << std::setw(6) << "Arr" << std::setw(7) << "Burst" << std::setw(7) << "Start" << std::setw(8) << "Finish" << std::setw(7) << "Wait" << std::setw(7) << "Resp" << std::setw(7) << "Turn" << "\n";

    for (Process* proc : procs) {
        std::cout << std::left << std::setw(6) << proc->pid << std::setw(6) << proc->arrival << std::setw(7) << proc->burst << std::setw(7) << proc->start << std::setw(8) << proc->finish << std::setw(7) << proc->waiting << std::setw(7) << proc->response << std::setw(7) << proc->turnaround << "\n";
    }

    std::cout << "-------------------------------------\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Avg Wait = " << avgWait / procs.size() << "\n";
    std::cout << "Avg Resp = " << avgResponse / procs.size() << "\n";
    std::cout << "Avg Turn = " << avgTurnaround / procs.size() << "\n";
    std::cout << "Throughput = " << throughput << " jobs/unit time\n";
    std::cout << "CPU Utilization = " << cpuUtilization << "%\n";

    // Cleaning up semaphores
    for (Process* proc : procs) {
        sem_destroy(&proc->run_one);
        sem_destroy(&proc->yielded);

        delete proc;
    }

    return 0;
}