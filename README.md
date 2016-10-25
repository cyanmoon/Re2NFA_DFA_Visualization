# Re2NFA_DFA_Visualization
Convert `Regular Expression` to `NFA`, `DFA`, `Minimum DFA` and visualize `NFA`, `DFA`, `Minimum DFA`.
#Sample
####Visualization of Regular Expression `"a(bc)+|ef8*"`
![alt text][logo]

[logo]: https://raw.githubusercontent.com/cyanmoon/Re2NFA_DFA_Visualization/master/sample.png "Sample"

#Usage:

##1. Get NFA, DFA, Minimum DFA

###MacOS:		
* `cd bin`
* `./Re2DFA "(abc)+(dp)*q|efg"`

###Windows:
* `cd bin`	
* `Re2DFA.exe "(abc)+(dp)*q|efg"`

NFA, DFA, Minimum DFA are saved in file `dfa.dfa`

##2. Visualization


* `python visualize.py "(abc)+(dp)*q|efg"`

#You should know:
##1. Visualization requires `PySide`  
* https://pypi.python.org/pypi/PySide/1.2.4
* http://wiki.qt.io/PySide

##2. Regular Expression supported operators
* `()`
* `|`
* `*`
* `+`


