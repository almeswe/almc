# generates test cases for keywords
with open('TOKENS.txt', 'r') as file:
	with open('gen-output.txt', 'w') as output:
		i = -1		
		for line in file:
			if line[:-2] != '':
				i += 1
				output.write(f'assert(tokens[{i}]->type == {line[:-2]});\n')