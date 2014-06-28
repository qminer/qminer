import re

fr = open('..\..\..\docjs\jsdocfinal.txt', 'r')
fw = open('intellisense.js', 'w')

for line in fr:
    line = line.strip()
    if re.match('- `', line):
        line = line[3:]
        if line.index('`'):
            jsline = line[:line.index('`')]
            comment ="";            
            try:
                comment = line[line.index('`')+1 :]
            except:
                pass
            print(line)
            fw.write(jsline + '$SEPARATOR$' + comment +'\n')
