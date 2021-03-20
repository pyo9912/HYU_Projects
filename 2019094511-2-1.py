import numpy as np

M = np.arange(2,27,1)
print(M)
print()
M = M.reshape(5,5)
print(M)
print()
M[:,0] = 0
print(M)
print()
M = M@M
print(M)
print()
v = M[0,0:]
mag = np.sqrt(sum(i**2 for i in v))
print(mag)