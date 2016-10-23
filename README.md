# Re2NFA_DFA_Visualization
Convert `Regular Expression` to `NFA`, `DFA`, `Minimum DFA` and visualize `NFA`, `DFA`, `Minimum DFA`.
#Usage:

##1. Get NFA, DFA, Minimum DFA

###MacOS:		
* `cd bin`
* `./Re2DFA "(abc)+d*|efg"`

###Windows:
* `cd bin`	
* `Re2DFA.exe "(abc)+d*|efg"`

NFA, DFA, Minimum DFA are saved in file `bin/dfa.dfa`

##2. Visualization


* `python visualize.py`

#You should know:
##1. Visualization requires `PySide`  
* https://pypi.python.org/pypi/PySide/1.2.4
* http://wiki.qt.io/PySide

