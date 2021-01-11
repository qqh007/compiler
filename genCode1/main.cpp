#include <fstream>

#include "lexer.h"
#include "parser.h"
#include "error.h"
#include "interCode.h"
#include "mipsCode.h"

using namespace std;

int main() {
	infile.open("testfile.txt");
	outfile.open("output.txt");
	errorfile.open("error.txt");
	interfile.open("interCode.txt");
	mipsfile.open("mips.txt");

	analyzeLexcial();
	program();
	translate();

	infile.close();
	outfile.close();
	errorfile.close();
	interfile.close();
	mipsfile.close();

    return 0;
}