import os
import sys
import getopt
import math

def sieve(n):

    primes = []

    # Fill working list with 1s.
    numbers = [1 for i in range(n+1)]

    # Mark the number 1 as special.
    numbers[1] = 0
    
    # Mark all multiples of composite numbers as 0, starting with the number 2 .
    for k in range(1, int(math.sqrt(n))):

        # Find the first number in the list greater than k that has not been identified as composite and call it m.
        for m in range(1, n+1):
            if m > k and numbers[m] != 0:
                break

        # Mark multiples of m as composite.
        i = 2
        while i*m <= n: 
            numbers[i*m] = 0
            i += 1

    # Add all left over numbers to the list of primes.
    for i in range(1, n):
        if numbers[i] != 0:
            primes.append(i)

    return primes

if __name__ == '__main__':
    primes = sieve(100)
    print "primes: %d" % len(primes)
    for p in primes:
        print p

