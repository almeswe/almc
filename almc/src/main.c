#include "../test/test.h"

#include "front-end/import.h"
#include "utils/os.h"

void main(int argc, char** argv)
{
	parse_import_path(NULL);
	run_tests();
	system("pause");
}