import sys
for line in sys.stdin:
    if 'Exit' == line.rstrip():
        break
    print(f'{line}')
sys.stderr.write("This is an error msg")
