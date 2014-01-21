def dictswap(olddict):
    newdict = {}
    for k in olddict:
        newdict[olddict[k]] = k
    return newdict

if __name__ == '__main__':
    olddict = {'1':'a',
               '2':'b',
               '3':'c'}
    print dictswap(olddict)
