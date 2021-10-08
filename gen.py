with open('opts.txt', 'r') as file:
    for line in file:
        print(f'{line[:-1]};')
