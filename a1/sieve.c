#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sieve.h"

/* Compiled with "gcc sieve.c -o sieve -lm" */

primes * sieve (int n)
{

        /* Make primes struct */
        primes *p = malloc(sizeof(primes));

        /* We don't know how many primes there will be, so we set a reasonable upper bound as the array size */
        p->values = malloc(sizeof(int) * sqrt(n));
        p->size = 0;
        
        /* Fill a working array with integers from 1 to n (index at 0 will remain blank) */
        int *values = malloc(sizeof(int) * (n + 1));
        int i;
        for(i = 1; i <= n; i++){
                values[i] = i;
        }

        /* Mark the number 1 as special */
        values[1] = 0;

        /* Mark all multiples of composite numbers as 0, starting with the number 2 */
        int k;
        for(k = 1; k <= sqrt(n); k++){
                /* Find first number in the array m greater than k that has not been identified as composite */
                int m;
                for(m = 1; m <= n; m++){
                    if(m > k && values[m] != 0){
                        break;
                    }
                }

                /* Mark multiples of m as composite */
                int i;
                for(i = 2; i*m <= n; i++){
                        values[i*m] = 0;
                }
        }

        /* Add all left over numbers to the list of primes */
        for(i = 1; i < n; i++){
                if(values[i] != 0){
                        p->values[p->size] = values[i];
                        p->size++;
                }
        }

        return p;
}

int main (int argc, const char * argv[]) {
        primes *p = sieve(100);
        int i;

        printf("primes: %d\n", p->size);
        for(i = 0; i < p->size; i++){
                printf("%d\n", p->values[i]);
        }

        return 0;
}
