#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <string>

#include "lexAna.h"

using namespace std;

ifstream infile;
ofstream outfile;

Token words[1000];
char theChar;
string token;

int line = 1;

int num;

Symbol symbol;

int Token::count;
int Token::pos;


Token::Token()
{
	this->str = "";
	this->symbol = INTCON;
}
Token::Token(string str, Symbol symbol)
{
	this->str = str;
	this->symbol = symbol;
}


bool isNewLine()
{
	return theChar == '\n';
}

void getChar()
{
	theChar = infile.get();
	if (isNewLine()) line++;
}

void clearToken()
{
	token = "";
}


bool isWhitespace()
{
	return isspace(theChar);
}

bool isBar()
{
	return theChar == '_';
}

bool isLetter()
{
	return isalpha(theChar);
}

bool isDigit()
{
	return isdigit(theChar);
}

bool isPlus()
{
	return theChar == '+';
}

bool isMinus()
{
	return theChar == '-';
}

bool isMult()
{
	return theChar == '*';
}

bool isDivi()
{
	return theChar == '/';
}

bool isLess()
{
	return theChar == '<';
}

bool isGreater()
{
	return theChar == '>';
}

bool isEqual()
{
	return theChar == '=';
}

bool isNot()
{
	return theChar == '!';
}

bool isColon()
{
	return theChar == ':';
}

bool isSemi()
{
	return theChar == ';';
}

bool isComma()
{
	return theChar == ',';
}


bool isLpar()
{
	return theChar == '(';
}

bool isRpar()
{
	return theChar == ')';
}

bool isLbrack()
{
	return theChar == '[';
}

bool isRbrack()
{
	return theChar == ']';
}

bool isLbrace()
{
	return theChar == '{';
}

bool isRbrace()
{
	return theChar == '}';
}

bool isOneQuote()
{
	return theChar == '\'';
}

bool isTwoQuote()
{
	return theChar == '\"';
}

void catToken()
{
	token = token + theChar;
}

void retract()
{
	infile.seekg(-1, ios::cur);
	if (isNewLine()) line--;
}

void reserver()
{
	string lowerToken = token;
	transform(lowerToken.begin(), lowerToken.end(), lowerToken.begin(), ::tolower);
	if (lowerToken == "const")
	{
		symbol = CONSTTK;
	}
	else if (lowerToken == "int")
	{
		symbol = INTTK;
	}
	else if (lowerToken == "char")
	{
		symbol = CHARTK;
	}
	else if (lowerToken == "void")
	{
		symbol = VOIDTK;
	}
	else if (lowerToken == "main")
	{
		symbol = MAINTK;
	}
	else if (lowerToken == "if")
	{
		symbol = IFTK;
	}
	else if (lowerToken == "else")
	{
		symbol = ELSETK;
	}
	else if (lowerToken == "switch")
	{
		symbol = SWITCHTK;
	}
	else if (lowerToken == "case")
	{
		symbol = CASETK;
	}
	else if (lowerToken == "default")
	{
		symbol = DEFAULTTK;
	}
	else if (lowerToken == "while")
	{
		symbol = WHILETK;
	}
	else if (lowerToken == "for")
	{
		symbol = FORTK;
	}
	else if (lowerToken == "scanf")
	{
		symbol = SCANFTK;
	}
	else if (lowerToken == "printf")
	{
		symbol = PRINTFTK;
	}
	else if (lowerToken == "return")
	{
		symbol = RETURNTK;
	}
	else
	{
		symbol = IDENFR;
	}
}


int transNum(string token)
{
	return stoi(token);
}


int getsym()
{
	clearToken();
	getChar();

	while (isWhitespace())
	{
		getChar();
	}

	if (isLetter() || isBar())
	{
		while (isLetter() || isDigit() || isBar())
		{
			catToken();
			getChar();
		}
		retract();
		reserver();
	}
	else if (isDigit())
	{
		while (isDigit())
		{
			catToken();
			getChar();
		}
		retract();
		num = transNum(token);
		symbol = INTCON;
	}
	else if (isPlus())
	{
		catToken();
		symbol = PLUS;
	}
	else if (isMinus())
	{
		catToken();
		symbol = MINU;
	}
	else if (isMult())
	{
		catToken();
		symbol = MULT;
	}
	else if (isDivi())
	{
		catToken();
		symbol = DIV;
	}
	else if (isColon())
	{
		catToken();
		symbol = COLON;
	}
	else if (isSemi())
	{
		catToken();
		symbol = SEMICN;
	}
	else if (isComma())
	{
		catToken();
		symbol = COMMA;
	}
	else if (isLpar())
	{
		catToken();
		symbol = LPARENT;
	}
	else if (isRpar())
	{
		catToken();
		symbol = RPARENT;
	}
	else if (isLbrack())
	{
		catToken();
		symbol = LBRACK;
	}
	else if (isRbrack())
	{
		catToken();
		symbol = RBRACK;
	}
	else if (isLbrace())
	{
		catToken();
		symbol = LBRACE;
	}
	else if (isRbrace())
	{
		catToken();
		symbol = RBRACE;
	}
	else if (isLess())
	{
		catToken();
		getChar();
		if (isEqual())
		{
			catToken();
			symbol = LEQ;
		}
		else
		{
			retract();
			symbol = LSS;
		}
	}
	else if (isGreater())
	{
		catToken();
		getChar();
		if (isEqual())
		{
			catToken();
			symbol = GEQ;
		}
		else
		{
			retract();
			symbol = GRE;
		}
	}
	else if (isEqual())
	{
		catToken();
		getChar();
		if (isEqual())
		{
			catToken();
			symbol = EQL;
		}
		else
		{
			retract();
			symbol = ASSIGN;
		}
	}
	else if (isNot())
	{
		catToken();
		getChar();
		if (isEqual())
		{
			catToken();
			symbol = NEQ;
		}
		else
		{
			retract();
		}
	}
	else if (isOneQuote())
	{
		getChar();
		catToken();
		getChar();
		symbol = CHARCON;

	}
	else if (isTwoQuote())
	{
		getChar();
		while (!isTwoQuote())
		{
			catToken();
			getChar();
		}
		symbol = STRCON;
	}
	else if (theChar == EOF)
	{
		return -1;
	}
	else
	{
		//error();
	}

	return 0;
}

string symbolStr()
{
	switch (symbol)
	{
	case IDENFR:
		return "IDENFR";
	case INTCON:
		return "INTCON";
	case CHARCON:
		return "CHARCON";
	case STRCON:
		return "STRCON";
	case CONSTTK:
		return "CONSTTK";
	case INTTK:
		return "INTTK";
	case CHARTK:
		return "CHARTK";
	case VOIDTK:
		return "VOIDTK";
	case MAINTK:
		return "MAINTK";
	case IFTK:
		return "IFTK";
	case ELSETK:
		return "ELSETK";
	case SWITCHTK:
		return "SWITCHTK";
	case CASETK:
		return "CASETK";
	case DEFAULTTK:
		return "DEFAULTTK";
	case WHILETK:
		return "WHILETK";
	case FORTK:
		return "FORTK";
	case SCANFTK:
		return "SCANFTK";
	case PRINTFTK:
		return "PRINTFTK";
	case RETURNTK:
		return "RETURNTK";
	case PLUS:
		return "PLUS";
	case MINU:
		return "MINU";
	case MULT:
		return "MULT";
	case DIV:
		return "DIV";
	case LSS:
		return "LSS";
	case LEQ:
		return "LEQ";
	case GRE:
		return "GRE";
	case GEQ:
		return "GEQ";
	case EQL:
		return "EQL";
	case NEQ:
		return "NEQ";
	case COLON:
		return "COLON";
	case ASSIGN:
		return "ASSIGN";
	case SEMICN:
		return "SEMICN";
	case COMMA:
		return "COMMA";
	case LPARENT:
		return "LPARENT";
	case RPARENT:
		return "RPARENT";
	case LBRACK:
		return "LBRACK";
	case RBRACK:
		return "RBRACK";
	case LBRACE:
		return "LBRACE";
	case RBRACE:
		return "RBRACE";
	default:
		return "";
	}
}

void analyzeLexcial()
{
	Token::count = 0;
	while (theChar != EOF)
	{
		getsym();
		if (token != "")
		{
			words[Token::count] = Token(token, symbol);
			Token::count++;
		}
	}
}

void getsymInWords()
{
	if (Token::pos >= 1)
	{
		outfile << symbolStr() << " " + token << endl;
		cout << symbolStr() << " " + token << endl;
	}

	symbol = words[Token::pos].symbol;
	token = words[Token::pos].str;
	Token::pos++;
}