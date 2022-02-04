with open('sse_instructions_list.txt', 'r') as file:
    i: int = 0
    lines = []
    for line in file:
        lines.append(line[:-1])
    for line in lines:
        print(f'#define\t{line.upper()}\t\t{i}')
        i += 1
    print('\nstatic char* instructions_str[] = {')
    for line in lines:
        print(f'\t[{line.upper()}]\t\t= \"{line.lower()}\",')
    print('};')
