#include <stdio.h>
#include <mpi.h>
#include <math.h>

// function which abort the MPI environment due to a problem
void Abort(MPI_Comm comm, int rc, const char* wrong_message) 
{
    if (rc != 0)
    {
        perror(wrong_message);
        MPI_Abort(comm, rc); // Abort the communicator in case of error
    }
}

int menu()
{
    printf("Menu: \n");
    printf("1. Continue \n");
    printf("2. Exit \n");

    printf("\n");

    int choice;
    printf("Type a choice: \n");
    scanf("%d",&choice);

    while(choice < 1 || choice > 2)
    {
        printf("\n");
        
        printf("Error choice!!! \n");
        printf("Type a choice: \n");
        scanf("%d",&choice);
    }
    printf("\n");

    return choice;
}

int main(int argc, char** argv)
{
    int choice;
    int X[100]; 
    int localX[100]; // local array for every process
    int D[100]; // new array
    int localD[100]; // local array so as to calculate the new array
    int d; // result in square 
    int total_numbers;
    int process_rank;
    int p;
    // Three variables specifies the amount numbers of every process
    int base_count,remainder,temp_count;
    int result; // We check the result of every function of MPI
    // tags for the communication between processes
    int tag1 = 1, tag2 = 2, tag3 = 3, tag4 = 4, tag5 = 5, tag6 = 6, tag7 = 7, tag8 = 8, tag9 = 9;
    int numbers;
    int numbers_per_task;
    int sum;
    int final_sum;
    double average;
    int max;
    int final_max;
    double sub_var;
    double local_var;
    double var;
    MPI_Status status; // we take information for receiving data

    result = MPI_Init(&argc,&argv); // MPI Initialization environment
    Abort(MPI_COMM_WORLD, result, "MPI_Init");

    result = MPI_Comm_rank(MPI_COMM_WORLD, &process_rank); // rank for every process
    Abort(MPI_COMM_WORLD, result, "MPI_Comm_rank");

    result = MPI_Comm_size(MPI_COMM_WORLD, &p); // total of processes
    Abort(MPI_COMM_WORLD, result, "MPI_Comm_size");
    
    choice = 1; // Initially, we consider that the user continues the procedure
    while(choice == 1)
    {
        if(process_rank == 0) // process 0
        {
            // User types the total of numbers and numbers in array X
            printf("Type total numbers for calculation, until 100 numbers \n");
            scanf("%d", &total_numbers);
            printf("\n");
            printf("Type %d integer numbers \n", total_numbers);
            
            for (int i = 0; i < total_numbers; i++)
            {
                scanf("%d", &X[i]);
            }

            printf("\n");

            // Process 0 sends to other processes the total of numbers
            for(int process=1; process<p; process++)
            {
                result = MPI_Send(&total_numbers, 1, MPI_INT, process, tag1, MPI_COMM_WORLD);
                Abort(MPI_COMM_WORLD, result, "MPI_Send");
            }

            // We specify the amount numbers for every process

            base_count = total_numbers / p;
            remainder = total_numbers % p;

            if(process_rank < remainder)
            {
                numbers_per_task = base_count + 1;
            }
            else
            {
                numbers_per_task = base_count;
            }

            temp_count = numbers_per_task; // numbers for particular process
            /* first position of every process which receives the data from this position
            first time we save the position of process 1 */
            numbers = temp_count;

            // Process 0 shares elements of array to other processes
            for (int process=1; process<p; process++)
            {
                /* We specify amount numbers for every process except from process 0
                in order to know how elements process 0 must send in every process */
                if(process < remainder)
                {
                    temp_count = base_count + 1;
                }
                else
                {
                    temp_count = base_count;
                }

                result = MPI_Send(&X[numbers],temp_count,MPI_INT,process,tag2,MPI_COMM_WORLD);
                Abort(MPI_COMM_WORLD, result, "MPI_Send");

                numbers += temp_count; // increase the position (first) for next process
            }

            // usage local array for calculation of every process
            for(int i=0; i<numbers_per_task; i++)
            {
                localX[i] = X[i];
            }
        }

        else // every process except from process 0
        {
            // Every process receives from process 0 the total of numbers
            result = MPI_Recv(&total_numbers, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status);
            Abort(MPI_COMM_WORLD, result, "MPI_Recv");

            // We specify the amount numbers for every process

            base_count = total_numbers / p;
            remainder = total_numbers % p;

            if(process_rank < remainder)
            {
                numbers_per_task = base_count + 1;
            }
            else
            {
                numbers_per_task = base_count;
            }

            // Every process receives elements which must calculate from process 0
            result = MPI_Recv(&localX[0],numbers_per_task,MPI_INT,0,tag2,MPI_COMM_WORLD,&status);
            Abort(MPI_COMM_WORLD, result, "MPI_Recv");
        }

        // Every process calculate locally its sub-sum   
        sum = 0;
        for(int i=0; i<numbers_per_task; i++)
        {
            sum = sum + localX[i];
        }

        // Every process calculate locally its sub-max
        max = localX[0];
        for(int i=1; i<numbers_per_task; i++)
        {
            if(localX[i] > max)
            {
                max = localX[i];
            }
        }

        if(process_rank != 0) // every process except from process 0
        {
            // Every process sends its sub-sum to process 0
            result = MPI_Send(&sum,1,MPI_INT,0,tag3,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD, result, "MPI_Send");

            // Every process sends its sub-max to process 0
            result = MPI_Send(&max,1,MPI_INT,0,tag4,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD, result, "MPI_Send");

            // every process receives the average of process 0
            result = MPI_Recv(&average,1,MPI_DOUBLE,0,tag5,MPI_COMM_WORLD,&status);
            Abort(MPI_COMM_WORLD, result, "MPI_Recv");

            // every process calculates its sub-sum of var except from process 0
            sub_var = 0;
            for(int i=0; i<numbers_per_task; i++)
            {
                local_var = localX[i] - average;
                sub_var += pow(local_var,2);
            }

            // every process sends to process 0 its sub-sum of var
            result = MPI_Send(&sub_var,1,MPI_DOUBLE,0,tag6,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD, result, "MPI_Send");

            // every process receives the maximum value from process 0
            result = MPI_Recv(&final_max,1,MPI_INT,0,tag7,MPI_COMM_WORLD,&status);
            Abort(MPI_COMM_WORLD, result, "MPI_Recv");

            // Every process calculates its elements using the maximum value
            for(int i=0; i<numbers_per_task; i++)
            {
                d = localX[i] - final_max;
                localD[i] = pow(d,2); 
            }

            // Every process sends calculations of its elements to process 0
            result = MPI_Send(&localD[0],numbers_per_task,MPI_INT,0,tag8,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD, result, "MPI_Send");
        }

        else // process 0
        {
            // Procedures and results of process 0
            printf("Process 0 calculate the average of %d numbers \n",total_numbers);
            printf("Process 0 calculate the maximum number of %d numbers \n \n",total_numbers);
            printf("Sum of process %d: %d \n",process_rank,sum);
            printf("Max of process %d: %d \n \n",process_rank,max);

            final_sum = sum; // initialization the sum of process 0 so as to find the final sum
            final_max = max; // initialization the max of process 0 so as to find the final max

            for(int process=1; process<p; process++)
            {
                // Process 0 receives sub-sums from other processes
                result = MPI_Recv(&sum,1,MPI_INT,process,tag3,MPI_COMM_WORLD,&status);
                Abort(MPI_COMM_WORLD, result, "MPI_Recv");

                printf("Sum of process %d: %d \n",process,sum);

                // Calculate the total sum
                final_sum = final_sum + sum;

                // Process 0 receives sub-max from other processes
                result = MPI_Recv(&max,1,MPI_INT,process,tag4,MPI_COMM_WORLD,&status);
                Abort(MPI_COMM_WORLD, result, "MPI_Recv");

                printf("Max of process %d: %d \n",process,max);

                // Calculation of maximum number of array
                if(max > final_max)
                {
                    final_max = max;
                }

                printf("\n");
            }

            // Calculate the average of total numbers
            average = (double) final_sum / total_numbers;
            printf("Average of array: %lf \n",average); 
        
            //Calculate the maximum number of total numbers
            printf("Final max of array: %d \n \n",final_max);   
        
            // process 0 calculates its sub-sum of var
            sub_var = 0;
            for(int i=0; i<numbers_per_task; i++)
            {
                local_var = localX[i] - average;
                sub_var += pow(local_var,2);
            }
            printf("Sub var of process %d: %lf \n",process_rank,sub_var);

            var = sub_var; // we declare as initial sum the sum var of process 0
            for(int process=1; process<p; process++)
            {
                // send average in all other processes
                result = MPI_Send(&average,1,MPI_DOUBLE,process,tag5,MPI_COMM_WORLD);
                Abort(MPI_COMM_WORLD,result,"MPI_Send");

                // process 0 receives sub-sums of var from other processes
                result = MPI_Recv(&sub_var,1,MPI_DOUBLE,process,tag6,MPI_COMM_WORLD,&status);
                Abort(MPI_COMM_WORLD,result,"MPI_Recv");

                printf("Sub var of process %d: %lf \n",process,sub_var);

                var += sub_var; // implementation of total sum var
            }

            printf("Var of array: %lf \n",var); // final result (var)
            printf("\n");

            printf("Array D: \n");
            printf("\n");
            // Process 0 calculates its elements using the maximum value
            int i; // index for array D
            for(i=0; i<numbers_per_task; i++)
            {
                d = localX[i] - final_max;
                localD[i] = pow(d,2); 
                D[i] = localD[i];
                printf("Process 0: %d \n",D[i]);
            }

            for(int process=1; process<p; process++)
            {
                // process 0 sends the maximum value to other processes
                result = MPI_Send(&final_max,1,MPI_INT,process,tag7,MPI_COMM_WORLD);
                Abort(MPI_COMM_WORLD,result,"MPI_Send");

                // We specify the amount numbers for every process
                /* Process 0 needs to know the amount numbers for every process
                because it will receive elements from other processes */

                if(process < remainder)
                {
                    temp_count = base_count + 1;
                }
                else
                {
                    temp_count = base_count;
                }

                // process 0 receives elements from other processes
                result = MPI_Recv(&localD[0],temp_count,MPI_INT,process,tag8,MPI_COMM_WORLD,&status);
                Abort(MPI_COMM_WORLD,result,"MPI_Recv");

                // We save elements of every process in array D
                for(int j=0; j<temp_count; j++)
                {
                    D[i] = localD[j];
                    printf("Process %d: %d \n",process,D[i]);
                    i++; // index for array D
                }
            }
        }

        if(process_rank == 0) // process 0
        {
            printf("\n \n");

            choice = menu(); // function menu returns the choice of user

            // Process 0 sends the choice of user to other processes
            for(int process=1; process<p; process++)
            {
                result = MPI_Send(&choice,1,MPI_INT,process,tag9,MPI_COMM_WORLD);
                Abort(MPI_COMM_WORLD,result,"MPI_Send");
            }
        }

        else // every process except from process 0
        {
            // every process receives the choice of user from process 0
            result = MPI_Recv(&choice,1,MPI_INT,0,tag9,MPI_COMM_WORLD,&status);
            Abort(MPI_COMM_WORLD,result,"MPI_Recv");
        }
    }

    MPI_Finalize(); // Termination of MPI environment
}
