/*
 * lexer.h
 *
 *  Created on: Jun 27, 2011
 *      Author: Lee
 */

#ifndef LEXER_H_
#define LEXER_H_

#include <iostream>
#include <string>
#include <list>
#include <vector>

using namespace std;

class tokenizer
{
    public:
        static const std::string DELIMITERS;
        tokenizer(const std::string& str);
        tokenizer(const std::string& str, const std::string& delimiters);
        bool next();
        bool next(const std::string& delimiters);
        const std::string token() const;
    protected:
        size_t m_offset;
        const std::string m_string;
        std::string m_token;
        std::string m_delimiters;
};

typedef int (*jitter) (void*);

struct procedure{
	vector<int>* il;
	unsigned char* hash;
	unsigned char status;
	int calls;
	jitter jit;
};

class lexer{
public:
	vector<struct procedure*>* procedures;
	vector<string>* proc_map;
	vector<int>* il;

	lexer(){
		il = new vector<int>();
		procedures = new vector<struct procedure*>();
		proc_map = new vector<string>();
	}

	lexer(string code){
		il = new vector<int>();
		procedures = new vector<struct procedure*>();
		proc_map = new vector<string>();
		if (!lex(code)) cerr << "Syntax Error." << endl;
	}

	lexer(lexer* l, vector<int>* code){
		procedures = l->procedures;
		proc_map = l->proc_map;
		il = code;
	}

	bool lex(string code);
	int resolve(string id);
};

enum opcodes{
	PLUS, MINUS, MUL, DIV, EXP, MOD,
	CALL_PROC
};

enum procedure_status{
	NORMAL, INLINED, JITTED
};

#define LAST CALL_PROC
#define OP (1<<31)
#define VAL (0x7fffffff)

#endif /* LEXER_H_ */
