#_*_coding:gbk_*_
"""
 visualize NFA DFA
"""
from PySide.QtGui import *
from PySide.QtCore import *
import sys
import re
import math


def addArrow(fromx, fromy, tox, toy, graphicsscene):
    line = QGraphicsLineItem(fromx, fromy, tox, toy)
    graphicsscene.addItem(line)

    linvec = [fromx - tox, fromy - toy]
    angle = math.pi/6
    uparrowvec = [linvec[0]*math.cos(angle) - linvec[1]*math.sin(angle)
        , linvec[0]*math.sin(angle) + linvec[1]*math.cos(angle)]
    downarrowvec = [linvec[0] * math.cos(-angle) - linvec[1] * math.sin(-angle)
        , linvec[0] * math.sin(-angle) + linvec[1] * math.cos(-angle)]
    arrowlen = 0.2
    uparrowendpos = [tox + arrowlen*uparrowvec[0], toy + arrowlen*uparrowvec[1]]
    downarrowendpos = [tox + arrowlen*downarrowvec[0], toy + arrowlen*downarrowvec[1]]
    line = QGraphicsLineItem(uparrowendpos[0], uparrowendpos[1], tox, toy)
    graphicsscene.addItem(line)
    line = QGraphicsLineItem(downarrowendpos[0], downarrowendpos[1], tox, toy)
    graphicsscene.addItem(line)

def parseSerializedFile(filename = "/Users/CERN/Library/Developer/Xcode/DerivedData/CompilerXcode-cpfctfpetjgqfrckuuzzggvxlekf/Build/Products/Debug/dfa.dfa"):
    serializedcontent = ""
    with open(filename) as f:
        serializedcontent = f.read()
    print serializedcontent
    ret = {}
    ret["NFA"] = {}
    ret["DFA"] = {}
    ret["MinDFA"] = {}
    nfaseg = ""
    dfaseg = ""
    mindfaseg = ""
    dfaindex = serializedcontent.find("DFA:")
    mindfaindex = serializedcontent.find("Minimum DFA:")
    if dfaindex >= 0:
        nfaseg = serializedcontent[:dfaindex]
        
    if mindfaindex >= 0 and mindfaindex > dfaindex:
        dfaseg = serializedcontent[dfaindex:mindfaindex]
        mindfaseg = serializedcontent[mindfaindex:]
    else:
        dfaseg = serializedcontent[dfaindex:]
        mindfaseg = dfaseg

    for i in zip(["NFA", "DFA", "MinDFA"], [nfaseg, dfaseg, mindfaseg]):
        seg = i[1].replace(" ", "")
        transpat = r"([\d]+)====([.\w\W\d\D\s\S\b\B]+?)====>([\d]+)"
        transitions = re.findall(transpat, seg)
        transdic = {}
        for line in transitions:
            ifrom = int(line[0])
            cha = line[1].strip()
            ito = int(line[2])
            if ifrom in transdic:
                if cha in transdic[ifrom]:
                    transdic[ifrom][cha].append(ito)
                else:
                    transdic[ifrom][cha] = [ito]

            else:
                chadic = {}
                chadic[cha] = [ito]
                transdic[ifrom] = chadic

        ret[i[0]]["transitions"] = transdic
        accpat = r"Accept:\n*([\d,]+)"
        accepts = re.findall(accpat, seg)
        accepts = accepts[0].replace(",", " ").strip()
        acceptsint = [int(x) for x in accepts.split(" ")]
        ret[i[0]]["accept"] = acceptsint

    return ret

def getpathtoaccepts(trans, curstate, processed, accpetstates):
    ret = []
    if curstate in accpetstates:
        ret.append([curstate])

    curstatetrans = trans.get(curstate, {})
    for key in curstatetrans.keys():
        itos = curstatetrans[key]
        for ito in itos:
            if ito in processed and ito not in accpetstates:
                continue
            else:
                processed.append(ito)
                if ito in accpetstates:
                    ret.append([curstate, ito])
                else:
                    l = getpathtoaccepts(trans, ito, processed, accpetstates)
                    if not l == None:
                        for item in l:
                            newpath = [curstate]
                            newpath.extend(item)
                            ret.append(newpath)
    return ret

def draw(data, graphicsscene):
    wid = 50
    hei = 60

    for faitemname in ["MinDFA"]:# , "MinDFA", "NFA"]:
        fa = data[faitemname]
        fapaths = getpathtoaccepts(fa["transitions"], 0, [0], fa["accept"])
        fapaths.sort(cmp=lambda x, y: len(y) - len(x))
        print fapaths
        posdic = {}
        maxlen = len(fapaths[0]) * wid
        row = 0
        for item in fapaths:
            col = 0
            statenum = len(item)
            for state in item:
                if not posdic.has_key(state):
                    posdic[state] = [row * wid, col * (maxlen) * 1.0 / statenum]
                col += 1

            row += 1

        posdic[0] = [(len(fapaths) - 1)/2.0 * wid, posdic[0][1]]
        # draw states
        for key in posdic.keys():
                pos = posdic[key]
                textItem = QGraphicsTextItem()
                textItem.setPlainText(str(key))
                textItem.setPos(pos[0], pos[1])
                textItem.setScale(1.0)
                graphicsscene.addItem(textItem)

                ellipse = QGraphicsEllipseItem(None, None)
                ellipse.setRect(0, 0, 20, 20)
                ellipse.setPos(pos[0], pos[1])
                graphicsscene.addItem(ellipse)

        # draw transitions
        # for ifromstate in trans.keys():
        #     frompos = posdic[ifromstate]
        #     transto = trans[ifromstate]
        #     for cha in transto.keys():
        #         itos = transto[cha]
        #         for itostate in itos:
        #             topos = posdic[itostate]
        #             addArrow(frompos[0], frompos[1], topos[0], topos[1], graphicsscene)


if __name__ == "__main__":
    dfanfa = parseSerializedFile()
    app = QApplication(sys.argv)
    scene = QGraphicsScene()
    draw(dfanfa, scene)
    # text = QGraphicsTextItem()
    # text.setPlainText("123")
    # scene.addItem(text)
    # ellipse = QGraphicsEllipseItem(None, None)
    # gradient = QRadialGradient(50, 50, 50, 50, 50)
    # gradient.setColorAt(0, QColor.fromRgbF(0, 1, 0, 1))
    # gradient.setColorAt(1, QColor.fromRgbF(0, 0, 0, 0))
    # brush = QBrush(gradient)
    # ellipse.setBrush(brush)
    # ellipse.setRect(0, 0, 15, 15)
    # ellipse.setPos(100,10)
    # ellipse.setBrush(QtGui.QBrush.)
    # scene.addItem(ellipse)
    view = QGraphicsView(scene)
    view.show()
    sys.exit(app.exec_())
