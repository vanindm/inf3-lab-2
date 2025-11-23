import random

n = 10 ** 8

with open("testdata.input", "w") as f:
#    f.write(str(n))
#    f.write("\n")
    for _ in range(n):
        f.write(str(random.gauss()))
        f.write(" ")
