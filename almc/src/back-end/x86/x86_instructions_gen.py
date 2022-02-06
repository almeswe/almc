import sys

with open('x86_instructions.txt', 'r') as file:
	print('enum x86_instruction_set\n{')

	line_bound   = 3
	line_counter = 0

	for line in file:
		line = line[:-1]
		if line_counter > line_bound:
			sys.stdout.write('\n')
			line_counter = 0
		if line_counter == 0:
			sys.stdout.write(f'\t{line}, ')
		else:
			sys.stdout.write(f'{line}, ')
		line_counter += 1
	print('\n}')
	file.close()

with open('x86_instructions.txt', 'r') as file:
	# to gen the string repr for each enum member
	print('\nstatic char* instructions_str[] = {')
	for line in file:
		line = line[:-1]
		print(f'\t[{line}]\t = \"{line.lower()}\",')
	print('};')