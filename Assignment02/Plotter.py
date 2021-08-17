
import matplotlib.pyplot as plt
import math
import numpy as np

x = []
CDF = []

file=open("CDF.out", "r")
if file.mode == 'r':
	entries = file.readlines()
	for entry in entries:
		entry = entry.split("\t")
		x.append(math.log10(int(entry[1])))
		s = entry[2]
		CDF.append(float(s))

plt.plot(x, CDF, 'm.')
plt.plot(x, CDF, 'k-')

plt.title('Prog4')
plt.xlabel('Access distance(in log base 10)')
plt.ylabel('CDF')

plt.show()


