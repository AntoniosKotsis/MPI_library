# MPI_library
Parallel programming using library MPI

You need to download the MPI library. It is not installed with compiler gcc.

The configuration file c_cpp_configurations.json is used in order to appear functions of MPI in different editors like VS Code.
Some editors have not extension of MPI library like VS Code. You need to use a configuration file.

We use the file myhosts when we are running amount processes greater than total cores of computer.
We declare the amount of localhosts.

- compile: mpicc exercise.c -o exercise -lm -Wall
- -lm is parameter for library math
- -Wall appears all warnings 
- run: mpirun -np 4 --hostfile myhosts ./exercise OR mpiexec -n 4 --hostfile myhosts ./exercise
- 4 is amount of processes.

**DESIGN–IMPLEMENTATION EXERCISE 1**

**Design of communication between processes running in parallel on the processors:**

We use the function **MPI_Init** to initialize the MPI environment,
**MPI_Comm_rank** to obtain the rank (ID) of each process, and
**MPI_Comm_size** to determine the total number of processes.

Process 0 initially sends to all other processes the complete set of numbers of the vector selected by the user.
The user enters the numbers that will compose the vector.

We implemented an **Abort** function which accepts three parameters: the communicator, the return value of the MPI function (MPI_Send, MPI_Recv), and a string that is passed as a parameter to **perror** in order to display the error.
Using this function, if an error occurs, the MPI environment is terminated.

---

### (a)

**Design:**

1. We divide the entire set of vector elements among all processes with equal distribution.
2. Each process computes its own local sum.
3. Each process sends its sum to process 0.
4. Process 0 receives the results from all processes and computes the final sum.

**Implementation:**

1. We implement the methodology for the case where the total number of elements is not an integer multiple of the number of processors.
2. Each process computes its sum based on the elements of its local array.

The implementation uses the **MPI_Send** function for sending data and **MPI_Recv** for receiving messages.

---

### (b)

**Design:**

1. Each process computes its local maximum.
2. Each process sends its local maximum to process 0.
3. Process 0 receives the local maxima from the other processes.
4. Process 0 computes the global maximum.

**Implementation:**
Each process knows the complete set of vector elements and which elements it will use for its local computation.

1. Each process locally computes the maximum of the elements assigned to it.
2. Each process sends its local maximum to process 0.
3. In process 0, we initialize the maximum with the largest value computed locally by process 0.
4. Each time process 0 receives a local maximum, it checks whether it is greater than the current maximum found so far.
5. Finally, we display the largest element of the vector.

---

### (c)

**Design:**

1. After process 0 computes the mean value, it sends it to all other processes.
   Before sending the mean value, process 0 computes its own partial variance sum.
2. Each process that receives the mean value computes its partial variance sum using its own elements and the mean.
3. Each process sends its partial variance sum to process 0.
4. Process 0 computes the total variance by summing the partial variance sums received from the other processes.
5. Process 0 displays the total variance.

**Implementation:**

1. We use a `for` loop to compute the local partial variance sum.
   We also use a `for` loop so that process 0 sends the mean value to the other processes using **MPI_Send**.
2. Each process receives the mean value using **MPI_Recv** in order to compute its own partial variance sum.
3. Each process uses **MPI_Send** to send its partial variance sum.
4. Process 0 uses **MPI_Recv** inside a `for` loop to receive each partial variance sum.
5. Process 0 computes the total variance by adding the partial variance sums.

---

### (d)

**Design:**

1. Process 0 computes its own elements, since it knows the maximum value.
   These elements are stored in a local array.
   Each element is stored in a shared-memory array.
2. Process 0 must send the maximum value to the other processes.
3. Each process has a subset of the numbers assigned to it. Each process computes the elements it is responsible for and stores them in a local array.
4. Each process sends the elements of the new vector to process 0.
5. We store the elements from all processes in a shared-memory array.
6. As elements are sent to process 0, we maintain an array to store them.

**Implementation:**

1. Using a `for` loop, process 0 computes the local results for its assigned elements and then stores them in a shared-memory array.
2. Process 0 uses a `for` loop to send the maximum value to each process using **MPI_Send**.
   The other processes receive it using **MPI_Recv**.
3. We follow exactly the same procedure as in step 1, without yet storing the elements in shared memory.
4. Processes 1 to p−1 use **MPI_Send**, while process 0 uses **MPI_Recv**.
   5–6. In process 0, we store the elements from each process in a shared-memory array.
   We display the elements of the new vector as well as the processes in which they were computed.

We use **MPI_Send** and **MPI_Recv** for inter-process communication.
Each process uses a local array to store its own data.
A shared-memory array is used to store the data from all processes.

---

### Menu

**Design:**

1. We create a menu with two options: continue or stop.
2. The user chooses whether to continue or not.
3. If an invalid value is entered, an error message is displayed, and the user must re-enter a value until it is valid.
4. The user’s choice is read by process 0.
5. Process 0 sends the choice to all other processes.
6. The other processes receive the user’s choice.

**Implementation:**

1. We implement a `menu` function with no parameters. It returns the user’s choice.
2. The user enters the desired option using `scanf()`.
3. We use a `while` loop to check for invalid input and ensure that no invalid menu option is executed.
4. We call the `menu` function in process 0.
5. Process 0 uses **MPI_Send** to send the user’s choice to the other processes.
6. The other processes use **MPI_Recv** to receive the choice from process 0.

We use **MPI_Send** and **MPI_Recv** for inter-process communication.
If process 0 does not send the user’s choice to stop (option 2 from the menu), then process 0 will terminate while the other processes will continue using their previous choice.
This will lead to a deadlock when the processes attempt to exchange messages.
Therefore, process 0 must send the user’s choice to all other processes so that all processes either continue or terminate together.

---

### Number of elements is not an integer multiple of the number of processors

**Design:**

1. We distribute the numbers based on the number of processes.
   We observe that the elements cannot be evenly distributed among all processes.

2. We apply the following methodology:

   * a) Compute the integer part of the division (ignore the fractional part).
   * b) Compute the remainder of the division.
   * c) Processes with rank smaller than the remainder receive a number of elements equal to the integer part plus one additional element.
   * d) Processes with rank greater than or equal to the remainder receive a number of elements equal to the integer part.

**Implementation:**

1. In each process, we compute the base count (integer part of the division) and the remainder.
   If the remainder is not zero, the elements cannot be evenly distributed among all processes.
2. We implement the above methodology.
