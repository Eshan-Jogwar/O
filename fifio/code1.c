#include <stdio.h>

int main() {
    int n, i;
    int at[10], bt[10], ct[10], tat[10], wt[10];

    printf("Enter number of processes: ");
    scanf("%d", &n);

    // 1. Input Loop
    for(i = 0; i < n; i++) {
        printf("Enter AT and BT for P%d: ", i);
        scanf("%d %d", &at[i], &bt[i]);
    }

    // 2. The Core Logic (The part to memorize!)
    for(i = 0; i < n; i++) {
        if (i == 0) {
            ct[i] = at[i] + bt[i]; // First process
        } else {
            // If process arrives after the previous one finished
            if (at[i] > ct[i-1]) 
                ct[i] = at[i] + bt[i];
            else 
                ct[i] = ct[i-1] + bt[i];
        }
        
        tat[i] = ct[i] - at[i];
        wt[i] = tat[i] - bt[i];
    }

    // 3. Print results (Gantt chart style)
    printf("\nP\tAT\tBT\tCT\tTAT\tWT\n");
    for(i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\n", i, at[i], bt[i], ct[i], tat[i], wt[i]);
    }
    return 0;
}
