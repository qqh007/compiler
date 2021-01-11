#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "error.h"

using namespace std;

int main() {
	infile.open("testfile.txt");
	outfile.open("output.txt");
	errorfile.open("error.txt");

	analyzeLexcial();
	program();

	infile.close();
	outfile.close();
	errorfile.close();

    return 0;
}