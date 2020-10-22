#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define HEAVY 100000
#define SHORT 1
#define LONG 10
#define N 20

enum tags {
	WORK_TAG, END_TAG
};

enum ranks {
	ROOT, SLAVE
};

void masterProcess(int num_procs);
double heavy(int x, int y);
void slaveProcess();

int main(int argc, char *argv[]) {

	int my_rank, num_procs;
	/* start up MPI */
	MPI_Init(&argc, &argv);
	/* start up MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	if (my_rank == ROOT) {
		masterProcess(num_procs);
	} else if (my_rank < N + 1) {
		//If there are more processes than number of calculations
		slaveProcess();
	}
	MPI_Finalize();
	return 0;
}

void masterProcess(int num_procs) {
	MPI_Status status;
	double slaveAns = 0, total_sum = 0, start_time;
	int num_slave, tag, source, count = 0;

	start_time = MPI_Wtime();

	int loop_count = num_procs;
	if (num_procs > N + 1) {
		loop_count = N + 1;
	}
	for (num_slave = 1; num_slave < loop_count; num_slave++) {
		//If there are more processes than calculations-first N processes get one job
		if (num_procs >= N) {
			tag = END_TAG;
		} else {
			tag = WORK_TAG;
		}

		//send indexes to slaves
		MPI_Send(&count, 1, MPI_INT, num_slave, tag, MPI_COMM_WORLD);
		count++;
	}

	int jobs_sent;

	for (jobs_sent = num_procs - 1; jobs_sent < N; jobs_sent++) {
		//Decide when to kill the slaves: if there are less jobs then slaves
		if (N - count > num_procs - 1) {
			tag = WORK_TAG;
		} else {
			tag = END_TAG;
		}

		//Wait until a process finishes, save the status and then send it another job
		MPI_Recv(&slaveAns, 1, MPI_DOUBLE, MPI_ANY_SOURCE, WORK_TAG,
		MPI_COMM_WORLD, &status);
		total_sum += slaveAns;

		source = status.MPI_SOURCE;

		MPI_Send(&count, 1, MPI_INT, source, tag, MPI_COMM_WORLD);
		count++;
	}

	for (num_slave = 1; num_slave < loop_count; num_slave++) {
		//Receive the last calculations before killing the processes
		MPI_Recv(&slaveAns, 1, MPI_DOUBLE, MPI_ANY_SOURCE, END_TAG,
		MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		total_sum += slaveAns;
	}

	printf("Parallel solution time:: %f\n", MPI_Wtime() - start_time);
	printf("Parallel Answer: %e\n", total_sum);

	//control
	start_time = MPI_Wtime();
	double ans = 0;
	int x, y;
	for (x = 0; x < N; x++) {
		for (y = 0; y < N; y++) {
			ans += heavy(x, y);
		}
	}
	printf("Serial solution time: %f\n", MPI_Wtime() - start_time);
	printf("Control Answer: %e\n", ans);
}

void slaveProcess() {

	MPI_Status status;
	int index, tag;
	int j;
	double answer;

	do {
		answer = 0;
		// receive indexes from masterProcess
		MPI_Recv(&index, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD,
				&status);

		tag = status.MPI_TAG;
		//calculate answer
		for (j = 0; j < N; j++) {
			answer += heavy(index, j);
		}
		//send answer to root
		MPI_Send(&answer, 1, MPI_DOUBLE, ROOT, tag, MPI_COMM_WORLD);

	} while (tag == WORK_TAG);
}

// This function performs heavy computations,
// its run time depends on x and y values
double heavy(int x, int y) {
	int i, loop = SHORT;
	double sum = 0;

	// Super heavy tasks
	if (x < 3 || y < 3)
		loop = LONG;
	// Heavy calculations
	for (i = 0; i < loop * HEAVY; i++)
		sum += cos(exp(sin((double) i / HEAVY)));

	return sum;
}
