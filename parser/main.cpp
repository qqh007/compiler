#include <fstream>

#include "lexAna.h"
#include "parser.h"

using namespace std;

int main() {
	infile.open("testfile.txt");
	outfile.open("output.txt");

	analyzeLexcial();
	program();

    return 0;
}