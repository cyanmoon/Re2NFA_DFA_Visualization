import time
import re
t = time.time()
ret = ""
#for i in range(1000):
ret = re.match("(1|2|3|4|5|6|7|8|9)+(0|1|2|3|4|5|6|7|8|9)*", "1234590abcerrrf")
print "%.10f %s"%(time.time() - t, ret.group())