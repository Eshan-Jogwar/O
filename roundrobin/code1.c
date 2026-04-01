#include <stdio.h>
#include <string.h>

#define MAX_PROC 10

typedef struct {
  char pid[5];
  int arrival;
  int total_cpu;
  int io_interval;
  int io_dur;
  int remaining_cpu;
  int current_cpu_burst;
  int current_io_burst;
  int total_io_time;
  int completion_time;
} Process;

void print_metrics(const char *algo_name, Process *procs, int n,
                   int total_time) {
  printf("--- %s Scheduling ---\n", algo_name);
  printf("%-5s | %-10s | %-8s | %-15s\n", "PID", "Turnaround", "Waiting",
         "Penalty Ratio");
  for (int i = 0; i < n; i++) {
    int turnaround = procs[i].completion_time - procs[i].arrival;
    int waiting = turnaround - procs[i].total_cpu - procs[i].total_io_time;
    float penalty =
        procs[i].total_cpu > 0 ? (float)turnaround / procs[i].total_cpu : 0.0f;
    printf("%-5s | %-10d | %-8d | %.2f\n", procs[i].pid, turnaround, waiting,
           penalty);
  }
  float throughput = total_time > 0 ? (float)n / total_time : 0.0f;
  printf("System Throughput: %.4f processes/unit time\n\n", throughput);
}

int main() {
  Process procs[MAX_PROC];
  int n = 0;
  int quantum = 3;

  FILE *file = fopen("data.txt", "r");
  if (file) {
    char line[100];
    while (fgets(line, sizeof(line), file)) {
      sscanf(line, "%[^;];%d;%d;%d;%d", procs[n].pid, &procs[n].arrival,
             &procs[n].total_cpu, &procs[n].io_interval, &procs[n].io_dur);
      n++;
    }
    fclose(file);
  } else {
    Process default_data[] = {{"P0", 0, 24, 5, 2, 0, 0, 0, 0, 0},
                              {"P1", 3, 17, 6, 3, 0, 0, 0, 0, 0},
                              {"P2", 8, 50, 5, 2, 0, 0, 0, 0, 0},
                              {"P3", 15, 10, 6, 3, 0, 0, 0, 0, 0}};
    n = 4;
    memcpy(procs, default_data, sizeof(default_data));
  }

  for (int i = 0; i < n; i++) {
    procs[i].remaining_cpu = procs[i].total_cpu;
    procs[i].current_cpu_burst = procs[i].io_interval < procs[i].total_cpu
                                     ? procs[i].io_interval
                                     : procs[i].total_cpu;
  }

  Process *ready_queue[MAX_PROC * 10];
  Process *io_queue[MAX_PROC * 10];
  int rq_count = 0, iq_count = 0;
  Process *cpu_proc = NULL;
  Process *io_proc = NULL;

  int time = 0, completed = 0;
  int current_q = 0;

  while (completed < n) {
    if (cpu_proc) {
      if (cpu_proc->remaining_cpu == 0) {
        cpu_proc->completion_time = time;
        completed++;
        cpu_proc = NULL;
      } else if (cpu_proc->current_cpu_burst == 0) {
        cpu_proc->current_io_burst = cpu_proc->io_dur;
        io_queue[iq_count++] = cpu_proc;
        cpu_proc = NULL;
      } else if (current_q == quantum) {
        ready_queue[rq_count++] = cpu_proc;
        cpu_proc = NULL;
      }
    }

    if (io_proc) {
      if (io_proc->current_io_burst == 0) {
        if (io_proc->remaining_cpu > 0) {
          io_proc->current_cpu_burst =
              io_proc->io_interval < io_proc->remaining_cpu
                  ? io_proc->io_interval
                  : io_proc->remaining_cpu;
          ready_queue[rq_count++] = io_proc;
        } else {
          io_proc->completion_time = time;
          completed++;
        }
        io_proc = NULL;
      }
    }

    for (int i = 0; i < n; i++) {
      if (procs[i].arrival == time) {
        ready_queue[rq_count++] = &procs[i];
      }
    }

    if (!cpu_proc && rq_count > 0) {
      cpu_proc = ready_queue[0];
      for (int i = 0; i < rq_count - 1; i++)
        ready_queue[i] = ready_queue[i + 1];
      rq_count--;
      current_q = 0; // Reset quantum
    }

    if (!io_proc && iq_count > 0) {
      io_proc = io_queue[0];
      for (int i = 0; i < iq_count - 1; i++)
        io_queue[i] = io_queue[i + 1];
      iq_count--;
    }

    if (cpu_proc) {
      cpu_proc->remaining_cpu--;
      cpu_proc->current_cpu_burst--;
      current_q++;
    }
    if (io_proc) {
      io_proc->current_io_burst--;
      io_proc->total_io_time++;
    }

    time++;
  }

  print_metrics("Round Robin (Q=3)", procs, n, time - 1);
  return 0;
}
