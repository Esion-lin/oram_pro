import sys
mmmp = {
    "ANSWER":0,
    "AND":1,
    "OR":2,
    "EXOR":3,
    "NOT":4,
    "SHL":5,
    "SHR":6,
    "ADD":7,
    "SUB":8,
    "MULL":9,
    "MULLH":10,
    "COMPE":11,
    "COMPA":12,
    "COMPAE":13,
    "MOV":14,
    "CMOV":15,
    "JMP":16,
    "CJMP":17,
    "CNJMP":18,
    "STORE":19,
    "LOAD":20,
    "READ":21,
    }
arg_map = {"MEM":512}
ret = []
ret_list = []
def deal_token(arr, amap, idx):
    if len(arr) == 1:
        # label
        amap[arr[0][:-1]] = idx[0]
    else:
        idx[0] += 1
def scan_list(arr):
    idx = [0]
    for line in arr:
        if line[0] == '#':
            continue
        words = line.split()      
        if len(words) >= 1:
            deal_token(words, arg_map, idx)
def replace(arr):
    for i in range(len(arr)):
        if arr[i] in arg_map:
            arr[i] = arg_map[arr[i]]

def dump_ins(file, arr):
    with open(file,"w") as f:
        for ele in arr:
            for word in ele:
                f.write(str(word))
                f.write(' ')
            f.write('\n')


if __name__ == '__main__':
    with open(sys.argv[1],"r") as f:
        lines = f.readlines()
        scan_list(lines)
        for line in lines:
            if line[0] == '#':
                continue
            words = line.split() 
            if len(words) > 1:
                replace(words)
                ret_list.append(words)
    for ele in ret_list:
        ri = 0
        rj = 0
        A = 0
        idtr = 0
        if ele[0] in {"AND", "OR", "EXOR", "NOT", "SHL", "SHR", "ADD", "SUB", "MULL", "MULLH"}:
            ri = int(ele[1][1:])
            rj = int(ele[2][1:])
            if isinstance(ele[3], str) and ele[3][0] == '%':
                A = int(ele[3][1:])
                idtr = 32 + A
            else:
                A = int(ele[3])
                idtr = 0
        elif ele[0] in {"COMPE", "COMPA", "COMPAE", "MOV", "CMOV", "LOAD", "READ"}:
            ri = int(ele[1][1:])
            if isinstance(ele[2], str) and ele[2][0] == '%':
                A = int(ele[2][1:])
                idtr = 32 + A
            else:
                A = int(ele[2])
                idtr = 0
            
        elif ele[0] == "STORE":
            ri = int(ele[1][1:])
            rj = 0
            A = 0
            idtr = 0
            if isinstance(ele[2], str) and ele[2][0] == '%':
                A = int(ele[2][1:])
                idtr = 32 + A
            else:
                A = int(ele[2])
                idtr = 0
        else:
            if isinstance(ele[1], str) and ele[1][0] == '%':
                A = int(ele[1][1:])
                idtr = 32 + A
            else:
                A = int(ele[1])
                idtr = 0
        tmp = [mmmp[ele[0]], idtr, ri, rj, 0, A]
        ret.append(tmp)
    dump_ins("test.ins", ret)