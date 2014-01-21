import os
import sys
import getopt
import math
import numpy

def matrix_mult(a, b):
    arows = a.shape[0]
    acols = a.shape[1]
    brows = b.shape[0]
    bcols = b.shape[1]

    if acols != brows:
        return "Cannot multiply matrices: size mismatch."
    else:
        rows = arows
        cols = bcols
        iters = acols # = brows
        c = [[0 for x in range(cols)] for y in range(rows)]

        for i in range(rows):
            for j in range(cols):
                for k in range(iters):
                    c[i][j] += a[i][k] * b[k][j]
        return c

if __name__ == '__main__':
    a = numpy.array([[1, 2, 3],
                     [4, 5, 6]])
    b = numpy.array([[1, 2],
                     [3, 4],
                     [5, 6]])
    print matrix_mult(a, b)
