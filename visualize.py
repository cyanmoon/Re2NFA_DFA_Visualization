#_*_coding:gbk_*_
"""
 visualize NFA DFA
"""
from PySide.QtGui import *
from PySide.QtCore import *
import sys
import re
import os
import math
import copy

ellipse_radius = 15
wid = 100
hei = 80
arrowcolor = QColor(91, 1, 2) # QColor(255, 155, 213)
linecolor = QColor(92, 155, 213)
textcolor = QColor(91, 1, 2)
statebgcolor = QColor(112, 173, 71)
statecirclecolor = QColor(91, 154, 213)
stateacceptcolor = QColor(112, 173, 71) # QColor(91, 1, 2)
statetextcolor = QColor(255, 255, 255)
linetextpos = 0.5
curvetextpos = 0.5

def iswin():
    return len(re.findall("win32|cygwin", sys.platform.lower())) > 0


def normalize(l):
    sumnum = 0
    for i in l:
        sumnum += i * i
    ret = []
    for i in l:
        ret.append(i / math.sqrt(sumnum))
    return ret


def mullf(l, f):
    ret = []
    for i in l:
        ret.append(i*f)
    return ret


def addl(la, lb):
    ret = []
    for i in range(len(la)):
        ret.append(la[i] + lb[i])
    return ret


def addTrans(fromstate, tostate, posdic, graphicsscene, textstr = None):
    global wid, hei, ellipse_radius, arrowcolor, linecolor, textcolor, statebgcolor, statecirclecolor, stateacceptcolor, statetextcolor, linetextpos, curvetextpos
    fromx, fromy = posdic[fromstate]
    tox, toy = posdic[tostate]
    radius = ellipse_radius
    linevec = [tox - fromx, toy - fromy]
    vec = [-radius, radius]
    if fromstate == tostate:
        fromx -= radius
        fromy += 0.2*radius
        tox -= radius
        vec = mullf(vec, 8)
    else:
        linevec = normalize(linevec)
        fromx, fromy = addl([fromx, fromy], mullf(linevec, ellipse_radius))
        tox, toy = addl([tox, toy], mullf(linevec, -1 * ellipse_radius))
        linevec = mullf(normalize([tox - fromx, toy - fromy]), radius*2)
        vec = [-linevec[1], linevec[0]]
        if fromy > toy:
            vec = mullf(vec, -1)
        vec = mullf(vec, (fromy - toy) / hei)

    paintpath = QPainterPath()
    paintpath.moveTo(fromx, fromy)
    paintpath.cubicTo(fromx + vec[0], fromy + vec[1], tox + vec[0], toy - vec[1], tox, toy)
    paintpath.moveTo(tox, toy)
    graphicsscene.addPath(paintpath, QPen(linecolor))

    # add arrow
    nearendpos = paintpath.pointAtPercent(0.8)
    linvec = [nearendpos.x() - tox, nearendpos.y() - toy]
    angle = math.pi / 6
    uparrowvec = [linvec[0] * math.cos(angle) - linvec[1] * math.sin(angle)
        , linvec[0] * math.sin(angle) + linvec[1] * math.cos(angle)]
    downarrowvec = [linvec[0] * math.cos(-angle) - linvec[1] * math.sin(-angle)
        , linvec[0] * math.sin(-angle) + linvec[1] * math.cos(-angle)]
    arrowlen = 0.8 * radius
    normalizedUp = normalize(uparrowvec)
    normalizedDown = normalize(downarrowvec)
    uparrowendpos = [tox + arrowlen * normalizedUp[0], toy + arrowlen * normalizedUp[1]]
    downarrowendpos = [tox + arrowlen * normalizedDown[0], toy + arrowlen * normalizedDown[1]]
    line = QGraphicsLineItem(uparrowendpos[0], uparrowendpos[1], tox, toy)
    line.setPen(QPen(arrowcolor))
    graphicsscene.addItem(line)
    line = QGraphicsLineItem(downarrowendpos[0], downarrowendpos[1], tox, toy)
    line.setPen(QPen(arrowcolor))
    graphicsscene.addItem(line)
    # add text
    if textstr is not None:
        text = QGraphicsTextItem()
        text.setPlainText("%s" % (textstr))
        font = QFont()
        font.setPointSizeF(ellipse_radius * 1.2)
        text.setFont(font)
        text.adjustSize()
        boudingRect = text.boundingRect()
        textpos = paintpath.pointAtPercent(curvetextpos)
        text.setPos(textpos.x(), textpos.y() - boudingRect.height() * 0.5)
        if fromy > toy:
            text.setPos(textpos.x() - boudingRect.width(), textpos.y() - boudingRect.height() * 0.5)
        text.setDefaultTextColor(textcolor)
        graphicsscene.addItem(text)


def parseSerializedFile(filename = "./bin/dfa.dfa"):
    serializedcontent = ""
    with open(filename) as f:
        serializedcontent = f.read()
    # print serializedcontent
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
        translinedic = {}
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

            if ifrom in translinedic:
                if ito in translinedic[ifrom]:
                    translinedic[ifrom][ito] += "," + cha
                else:
                    translinedic[ifrom][ito] = cha
            else:
                todic = {}
                todic[ito] = cha
                translinedic[ifrom] = todic

        mergedtransdic = {}
        for ifrom in translinedic.keys():
            line = translinedic[ifrom]
            for ito in line.keys():
                cha = line[ito]
                if ifrom in mergedtransdic:
                    if cha in mergedtransdic[ifrom]:
                        mergedtransdic[ifrom][cha].append(ito)
                    else:
                        mergedtransdic[ifrom][cha] = [ito]

                else:
                    chadic = {}
                    chadic[cha] = [ito]
                    mergedtransdic[ifrom] = chadic

        ret[i[0]]["transitions"] = mergedtransdic
        accpat = r"Accept:\n*([\d,]+)"
        accepts = re.findall(accpat, seg)
        accepts = accepts[0].replace(",", " ").strip()
        acceptsint = [int(x) for x in accepts.split(" ")]
        ret[i[0]]["accept"] = acceptsint

    return ret


def getpathtoaccepts(trans, curstate, processed, accpetstates, circletimes = 1):
    ret = []
    if curstate in accpetstates:
        ret.append([curstate])

    curstatetrans = trans.get(curstate, {})
    for key in curstatetrans.keys():
        itos = curstatetrans[key]
        for ito in itos:
            if ito in processed and processed[ito] > circletimes and ito not in accpetstates:
                continue
            else:
                copedprocessed = copy.deepcopy(processed)
                if ito in copedprocessed:
                    copedprocessed[ito] += 1
                else:
                    copedprocessed[ito] = 1

                if ito in accpetstates:
                    ret.append([curstate, ito])
                else:
                    l = getpathtoaccepts(trans, ito, copedprocessed, accpetstates)
                    if l is not None:
                        for item in l:
                            newpath = [curstate]
                            newpath.extend(item)
                            ret.append(newpath)

    return ret


def draw(graphicsscene, fa):
    global wid, hei, ellipse_radius, arrowcolor, linecolor, textcolor, statebgcolor, statecirclecolor, stateacceptcolor, statetextcolor, linetextpos, curvetextpos
    fapaths = []
    for acceptstate in fa["accept"]:
        tmp = getpathtoaccepts(fa["transitions"], 0, {0: 1}, [acceptstate])
        fapaths.extend(tmp)
    fapaths.sort(cmp=lambda x, y: len(x) - len(y))
    length = len(fapaths)
    maxlenpath = fapaths[-1]
    index = 0
    pathstorm = []
    for item in fapaths:
        if index == length - 1:
            continue

        statestorm = []
        for state in item:
            if state in maxlenpath:
                statestorm.append(state)

        for state in statestorm:
            item.remove(state)

        if len(item) == 0:
            pathstorm.append(item)
        index += 1

    for item in pathstorm:
        fapaths.remove(item)

    length = len(fapaths)
    if length > 1:
        if length % 2 == 0:
            patha = fapaths[0:length / 2]
            pathb = fapaths[length / 2:]
            patha.sort(cmp=lambda x, y: len(x) - len(y))
            pathb.sort(cmp=lambda x, y: len(y) - len(x))
            if len(pathb[0]) > len(patha[-1]):
                tmp = patha[-1]
                patha[-1] = pathb[0]
                pathb[0] = tmp
            fapaths = patha + pathb
        else:
            patha = fapaths[0:length / 2]
            pathb = fapaths[length / 2: -1]
            patha.sort(cmp=lambda x, y: len(x) - len(y))
            pathb.sort(cmp=lambda x, y: len(y) - len(x))
            fapaths = patha + [fapaths[-1]] + pathb

    print fapaths
    posdic = {}
    maxlen = len(maxlenpath) * hei
    row = 0
    for item in fapaths:
        col = 0
        statenum = len(item)
        for state in item:
            if not posdic.has_key(state):
                offset = 0
                if length > 1:
                    offset = math.fabs(row - (length - 1)/2)/(length - 1)
                posdic[state] = [row * wid, offset * maxlen +  col * maxlen * 1.0 / statenum]

            col += 1

        row += 1

    posdic[0] = [(len(fapaths) - 1) / 2.0 * wid, posdic[0][1]]
    # draw states
    accepts = fa["accept"]
    for key in posdic.keys():
        pos = posdic[key]
        ellipse = QGraphicsEllipseItem(None, None)
        ellipse.setRect(-ellipse_radius, -ellipse_radius, ellipse_radius * 2, ellipse_radius * 2)
        ellipse.setPos(pos[0], pos[1])
        ellipse.setBrush(QBrush(statebgcolor, style = Qt.SolidPattern))
        ellipse.setPen(QPen(statecirclecolor))
        graphicsscene.addItem(ellipse)
        if key in accepts:
            ellipseacc = QGraphicsEllipseItem(None, None)
            ellipseacc.setRect(-1.3 * ellipse_radius, -1.3 * ellipse_radius, 1.3 * 2 * ellipse_radius,
                               1.3 * 2 * ellipse_radius)
            ellipseacc.setPos(pos[0], pos[1])
            graphicsscene.addItem(ellipseacc)
            ellipseacc.setPen(QPen(stateacceptcolor))

        textItem = QGraphicsTextItem()
        textItem.setPlainText(str(key))
        font = QFont()
        font.setPointSizeF(ellipse_radius * 1.5)
        textItem.setFont(font)
        textItem.adjustSize()
        textItem.setDefaultTextColor(statetextcolor)
        boundingRect = textItem.boundingRect()
        textItem.setPos(pos[0] - boundingRect.width() / 2, pos[1] - boundingRect.height() / 2)
        graphicsscene.addItem(textItem)

    # draw transitions
    trans = fa["transitions"]
    for ifromstate in trans.keys():
        # frompos = posdic[ifromstate]
        transto = trans[ifromstate]
        for cha in transto.keys():
            itos = transto[cha]
            for itostate in itos:
                # topos = posdic[itostate]
                addTrans(ifromstate, itostate, posdic, graphicsscene, cha)


if __name__ == "__main__":
    wkpath = os.path.abspath(os.path.dirname(__file__))
    oldpwd = os.getcwd()
    os.chdir(wkpath)
    binpath = os.path.join(os.path.abspath(wkpath), "bin")
    restr = "(abc)+(dp)*q|efg"
    if len(sys.argv) > 1:
        restr = sys.argv[1]
    binpath = os.path.join(binpath, "Re2DFA")
    ret = os.popen('%s "%s"' % (binpath, restr)).read()
    print "cmdret: ", ret
    dfanfa = parseSerializedFile(os.path.join(wkpath, "dfa.dfa"))
    app = QApplication(sys.argv)
    scene = QGraphicsScene()
    draw(scene, dfanfa["MinDFA"])
    view = QGraphicsView(scene)
    view.setWindowTitle(restr)
    # + "   Re2NFA_DFA_MinDFA Visualization")
    view.show()
    os.chdir(oldpwd)
    sys.exit(app.exec_())
