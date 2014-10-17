
%{
//senquack - complete conversion to floats:
//typedef double NumType;
typedef float NumType;

//senquack - complete conversion to floats:
//#define YYSTYPE double
#define YYSTYPE float
#define YYERROR_VERBOSE

#include <cmath>
#include <cctype>

#include <vector>
#include <sstream>

#include "calc.h"
#include "formula.h"
#include "formula-variables.h"

int yyerror(char* s);
int yylex();

const char* yyinStr;

typedef Formula<NumType> CalcFormula;
typedef Number<NumType> CalcNumber;
typedef Random<NumType> CalcRandom;
typedef Rank<NumType> CalcRank;
typedef Param<NumType> CalcParam;
typedef Operator CalcOperator;

namespace {
	CalcFormula* formula;
	std::vector<CalcFormula*> formulas;

//senquack - complete conversion to floats:
//	CalcFormula* f(double d) { return formulas[(int)d]; }
	CalcFormula* f(float d) { return formulas[(int)d]; }

	int paramId;
}

%}

%token NUM RAND RANK PARAM
%left '-' '+'
%left '*' '/'
%left NEG     
%right '^'   

%%
input:    
        | input line
;

line:     '\n'
| exp '\n'  { formula = f($1); return 0; }
;

exp:    NUM {
	        $$ = formulas.size();
	        formulas.push_back(new CalcFormula(new CalcNumber($1)));
        }
        | RAND {
	        $$ = formulas.size();
            formulas.push_back(new CalcFormula(new CalcRandom()));
        }
        | RANK {
			$$ = formulas.size();
			formulas.push_back(new CalcFormula(new CalcRank()));
		}
        | PARAM {
			$$ = formulas.size();
			formulas.push_back(new CalcFormula(new CalcParam(paramId)));
		}
        | exp '+' exp {
		    $$ = formulas.size();
			formulas.push_back(new CalcFormula(f($1), op_add, f($3)));
		}
        | exp '-' exp {
		    $$ = formulas.size();
			formulas.push_back(new CalcFormula(f($1), op_sub, f($3)));
		}
        | exp '*' exp {
		    $$ = formulas.size();
			formulas.push_back(new CalcFormula(f($1), op_mul, f($3)));
		}
        | exp '/' exp {
		    $$ = formulas.size();
			formulas.push_back(new CalcFormula(f($1), op_div, f($3)));
		}
        | '-' exp  %prec NEG {
		    $$ = $2;
			f($2)->setHeadSub();
		}
        | '(' exp ')' {
		    $$ = $2;
		}
;
%%


#include <ctype.h>
#include <stdio.h>

int yylex ()
{
	int c;

	while ((c = *(yyinStr++)) == ' ' || c == '\t')
		;
	if (c == '.' || isdigit (c))
    {
		yyinStr--;
//senquack - complete conversion to floats:
//		sscanf (yyinStr, "%lf", &yylval);
		sscanf (yyinStr, "%f", &yylval);
		while ((c = *(++yyinStr)) == '.' || isdigit(c)) {}
		return NUM;
    }

	if (c == '$') {
		if (strncmp(yyinStr, "rand", 4) == 0) {
			yyinStr += 4;
			return RAND;
		}
		else if (strncmp(yyinStr, "rank", 4) == 0) {
		    yyinStr += 4;
			return RANK;
        }
		else {
			std::istringstream iss(std::string(yyinStr).substr(0, 1));
			iss >> paramId;
			yyinStr++;
			return PARAM;
		}
	}

	if (c == '\0')
		return 0;
	return c;
}

int yyerror(char* s) {
	printf("yyerror: %s\n", s);
	return 0;
}

std::auto_ptr<CalcFormula> calc(const std::string& str) {
	std::string fml = str + '\n';
	yyinStr = fml.c_str();
	yyparse();
	return std::auto_ptr<CalcFormula>(formula);
}

