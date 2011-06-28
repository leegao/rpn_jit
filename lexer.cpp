/*
 * lexer.cpp
 *
 *  Created on: Jun 27, 2011
 *      Author: Lee
 */

#include <iostream>
#include <vector>
#include <list>
#include <cstdio>
#include <cstdlib>
#include "lexer.h"
#include "md5.h"

using namespace std;

const string tokenizer::DELIMITERS(" \t\n\r");

tokenizer::tokenizer(const std::string& s) :
    m_string(s),
    m_offset(0),
    m_delimiters(DELIMITERS) {}

tokenizer::tokenizer(const std::string& s, const std::string& delimiters) :
    m_string(s),
    m_offset(0),
    m_delimiters(delimiters) {}

bool tokenizer::next()
{
    return next(m_delimiters);
}

bool tokenizer::next(const std::string& delimiters)
{
    size_t i = m_string.find_first_not_of(delimiters, m_offset);
    if (string::npos == i)
    {
        m_offset = m_string.length();
        return false;
    }

    size_t j = m_string.find_first_of(delimiters, i);
    if (string::npos == j)
    {
        m_token = m_string.substr(i);
        m_offset = m_string.length();
        return true;
    }

    m_token = m_string.substr(i, j - i);
    m_offset = j;
    return true;
}

const std::string tokenizer::token() const{
	return m_token;
}

int lexer::resolve(string id){
	vector<string>::iterator procedure;
	int i = 0;
	for(procedure = proc_map->begin(); procedure < proc_map->end(); procedure++){
		if (*procedure == id){
			return i;
		}
		i++;
	}
	return -1;
}

static unsigned char* digest(char* string, int len){
	MD5_CTX context;
	unsigned char* digest = (unsigned char*)malloc(16);

	MD5Init (&context);
	MD5Update (&context, (unsigned char*)string, len);
	MD5Final (digest, &context);

	return digest;
}

bool lexer::lex(string code){
	tokenizer eng(code);
	if (!eng.next()) { cerr << "Syntax Error: Empty Statement" << endl; return false;}

	procedure* proc = (procedure*)0;
	vector<int>* il = new vector<int>(); string name = eng.token();
	if (name.back() == ':'){
		name.pop_back();
		// setup a new procedure
		proc = (procedure*)malloc(sizeof(procedure));

		if(!eng.next()){cerr << "Syntax Error: Empty Procedure" << endl; return false;}
	}

	do {
		string tok = eng.token();
		int i;
		if (i=atoi(tok.c_str())){
			il->push_back(VAL & i);
		} else if (tok.size() == 1){
			switch(tok.front()){
			case '+':
				il->push_back(OP | PLUS);
				break;
			case '-':
				il->push_back(OP | MINUS);
				break;
			case '*':
				il->push_back(OP | MUL);
				break;
			case '/':
				il->push_back(OP | DIV);
				break;
			case '^':
				il->push_back(OP | EXP);
				break;
			case '%':
				il->push_back(OP | MOD);
				break;
			}
		} else{
			// tok is a procedure name
			int ptr;
			if((ptr = resolve(tok)) == -1){
				// Procedure not found
				cerr << "Syntax Error: Procedure not found." << endl;
				return false;
			}
			il->push_back(OP | (ptr + CALL_PROC));
		}
	} while(eng.next());

	if (proc){
		// This is a procedure
		this->il = 0;
		proc->il = il;
		int* mem = (int*)malloc(sizeof(int) * il->size());
		vector<int>::iterator it;
		int i = 0;
		for (it = il->begin(); it < il->end(); it++){
			mem[i] = *it;
			i++;
		}
		proc->hash = digest((char*)mem, (sizeof(int) * il->size()));
		proc->status = NORMAL;
		procedures->push_back(proc);
		proc_map->push_back(name);
	} else {
		// Part of main
		this->il = il;
	}


	return true;
}


