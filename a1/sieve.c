#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sieve.h"

primes sieve (int n)
{

        /* Make primes struct */
        primes *p = malloc(sizeof(primes));

        /* We don't know how many primes there will be, so we set a reasonable upper bound as the array size */
        p->values = malloc(sizeof(int) * sqrt(n));
        p->size = 0;
        
        /* Fill a working array with integers from 1 to n (index at 0 will remain blank) */
        int *values = malloc(sizeof(int) * (n + 1));

        /* Mark the number 1 as special */
        values[1] = 0;

        /* Mark all multiples of composite numbers as 0, starting with the number 2 */
        int k;
        for(k = 1; k <= sqrt(n); k++){
                /* Find first number in the array m greater than k that has not been identified as composite */
                int m = k + 1;
                while(values[m] == 0){
                        m++;
                }

                /* Mark multiples of m as composite */
                int i;
                for(i = 2; i*m <= n; i++){
                        values[i*m] = 0;
                }

                /* Put m on list of primes */
                p->values[p->size] = m;
                p->size++;

                k = m;
        }

        return p;
}

int main (int argc, const char * argv[]) {
        primes p = sieve(20);
        int i;
        for(i = 0; i < p->size; i++){
                printf("%d", p->values[i]);
        }

        return 0;
}
