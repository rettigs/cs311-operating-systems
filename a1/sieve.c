/*
 * Original Author: Sean Rettig (rettigs)
 * File: HW2rettigs_14.java
 * Created: Feb 15th 2012 by rettigs
 * Last Modified: Feb 16th 2012 by rettigs
 *
 * This class implements the Sieve of Eratosthenes, a method used to find all prime numbers under a certain value (referred to by this program as the max).
 */

public class HW2rettigs_14{

        /*
         * This method implements the Sieve of Eratosthenes, a method used to find all prime numbers under a certain value (referred to by this program as the max).
         * It takes the max as an argument, and prints all of the primes numbers equal to or below the max, in addition to the number of prime numbers.
         */
        public static void main(String[] args){

                //Gets the max number to check.
                int max;
                try{
                         max = Integer.parseInt(args[0]);
                }catch(Exception e){
                        System.out.printf("No integer specified as argument for max prime size. Program terminated.");
                        return;
                }

                //Fills an array with all integers from 2 to the max, where each integer value is equal to its array index number. I allow index locations 0 and 1 to default to 0,
                //because they are special numbers that are not used in the prime number calculation, and I use the number 0 to denote numbers in the array which are not prime.
                int[] numbers = new int[max + 1];
                for(int i = 2; i <= max; ++i){
                        numbers[i] = i;
                }

                //Sets all non-prime numbers in the array to 0, checking for non-primes by eliminating multiples of all previously established primes (starting with 2).
                for(int i = 2; i <= max; ++i){
                        if(numbers[i] != 0){
                                int multiple = 0;
                                int count = 2;
                                while(true){
                                        multiple = numbers[i] * count;
                                        try{
                                                numbers[multiple] = 0;
                                        }catch(ArrayIndexOutOfBoundsException e){
                                                break; //Stops checking for multiples of established primes to eliminate once the max has been hit, and moves on to the next prime.
                                        }
                                        ++count;
                                }
                        }
                }

                //Prints all numbers in the array that are not 0 (i.e. only prime numbers). Also prints the number of primes.
                int numberOfPrimes = 0;
                for(int i = 0; i <= max; ++i){
                        if(numbers[i] != 0){
                                System.out.printf("%d\n", numbers[i]);
                                ++numberOfPrimes;
                        }
                }
                System.out.printf("Number of primes: %d\n", numberOfPrimes);
        }
}
