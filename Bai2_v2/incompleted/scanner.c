/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"

extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

int originalLineNo;
int originalColNo;

/***************************************************************/

void skipBlank() {
  while (currentChar != EOF && charCodes[currentChar] == CHAR_SPACE) {
    readChar();
  }
}

void skipComment() {
  readChar();

  while (currentChar != EOF) {
    if (charCodes[currentChar] == CHAR_TIMES) {
      readChar();
      if (charCodes[currentChar] == CHAR_RPAR) {
        break;
      }
    } else {
      readChar();
    }
  }
}

Token* readIdentKeyword(void) {
  char tmp[MAX_IDENT_LEN + 1];
  int tmpLength = 0;
  char ch;

  tmp[0] = '\0';

  while (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT) {
    if (tmpLength == MAX_IDENT_LEN) {
      error(ERR_IDENTTOOLONG, originalLineNo, originalColNo);
      return makeToken(TK_NONE, originalLineNo, originalColNo);
    }
    ch = (char) currentChar;
    strncat(tmp, &ch, 1);
    tmpLength++;
    readChar();
  }

  Token *token = makeToken(checkKeyword(tmp), originalLineNo, originalColNo);
  if (token->tokenType == TK_NONE) {
    token->tokenType = TK_IDENT;
    strcpy(token->string, tmp);
  }

  return token;
}

Token* readNumber(void) {
  char tmp[MAX_IDENT_LEN + 1];
  char ch;

  tmp[0] = '\0';

  while (charCodes[currentChar] == CHAR_DIGIT) {
    ch = (char) currentChar;
    strncat(tmp, &ch, 1);
    readChar();
  }

  Token *token;

  if (charCodes[currentChar] == CHAR_PERIOD) {
    ch = (char) currentChar;
    strncat(tmp, &ch, 1);
    readChar();

    int checkDigit = 0;
    while (charCodes[currentChar] == CHAR_DIGIT) {
      checkDigit = 1;
      ch = (char) currentChar;
      strncat(tmp, &ch, 1);
      readChar();
    }

    if (checkDigit == 0) {
      ch = '0';
      strncat(tmp, &ch, 1);
    }

    token = makeToken(TK_FLOAT, originalLineNo, originalColNo);
    strcpy(token->string, tmp);
  } else {
    token = makeToken(TK_NUMBER, originalLineNo, originalColNo);
    strcpy(token->string, tmp);
  }

  return token;
}

Token* readConstChar(void) {
  Token *token;
  char ch;

  readChar();
  ch = (char) currentChar;

  readChar();
  if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    token = makeToken(TK_CHAR, originalLineNo, originalColNo);
    strncat(token->string, &ch, 1);
    readChar();
  } else {
    token = makeToken(TK_NONE, originalLineNo, originalColNo);
    error(ERR_INVALIDCHARCONSTANT, originalLineNo, originalColNo);
  }

  return token;
}

Token* readConstString(void) {
  Token *token;
  char tmp[MAX_IDENT_LEN + 1];
  int tmpLength = 0;
  char ch;

  tmp[0] = '\0';

  readChar();

  while (currentChar != EOF && charCodes[currentChar] != CHAR_DOUBLEQUOTE) {
    if (charCodes[currentChar] == CHAR_SLASH) {
      readChar();
    }

    if (tmpLength == MAX_IDENT_LEN) {
      error(ERR_STRINGTOOLONG, originalLineNo, originalColNo);
      return makeToken(TK_NONE, originalLineNo, originalColNo); 
    }

    ch = (char) currentChar;
    strncat(tmp, &ch, 1);
    tmpLength++;
    readChar();
  }

  if (charCodes[currentChar] == CHAR_DOUBLEQUOTE) {
    token = makeToken(TK_STRING, originalLineNo, originalColNo);
    strcpy(token->string, tmp);
    readChar();
  } else {
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_ENDOFSTRING, lineNo, colNo);
  }

  return token;
}

Token* getToken(void) {
  Token *token;
  int ln, cn;

  originalLineNo = lineNo;
  originalColNo = colNo;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: skipBlank(); return getToken();
  case CHAR_LETTER: return readIdentKeyword();
  case CHAR_DIGIT: return readNumber();
  case CHAR_PLUS: 
    token = makeToken(SB_PLUS, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_MINUS:
    token = makeToken(SB_MINUS, lineNo, colNo);
    readChar(); 
    return token;  
  case CHAR_TIMES:
    token = makeToken(SB_TIMES, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_SLASH:
    token = makeToken(SB_SLASH, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_LT:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token = makeToken(SB_LE, originalLineNo, originalColNo);
      readChar();
    } else {
      token = makeToken(SB_LT, originalLineNo, originalColNo);
    }
    return token;
  case CHAR_GT:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token = makeToken(SB_GE, originalLineNo, originalColNo);
      readChar();
    } else {
      token = makeToken(SB_GT, originalLineNo, originalColNo);
    }
    return token;
  case CHAR_EXCLAIMATION:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token = makeToken(SB_NEQ, originalLineNo, originalColNo);
      readChar();
    } else {
      token = makeToken(TK_NONE, originalLineNo, originalColNo);
      error(ERR_INVALIDSYMBOL, originalLineNo, originalColNo);
    }
    return token;
  case CHAR_EQ:
    token = makeToken(SB_EQ, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_COMMA:
    token = makeToken(SB_COMMA, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_PERIOD:
    readChar();
    if (charCodes[currentChar] == CHAR_RPAR) {
      token = makeToken(SB_RSEL, originalLineNo, originalColNo);
      readChar();
    } else {
      token = makeToken(SB_PERIOD, originalLineNo, originalColNo);
    }
    return token;
  case CHAR_COLON:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token = makeToken(SB_ASSIGN, originalLineNo, originalColNo);
      readChar();
    } else {
      token = makeToken(SB_COLON, originalLineNo, originalColNo);
    }
    return token;
  case CHAR_SEMICOLON:
    token = makeToken(SB_SEMICOLON, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_SINGLEQUOTE: return readConstChar();
  case CHAR_LPAR:
    readChar();
    if (charCodes[currentChar] == CHAR_PERIOD) {
      token = makeToken(SB_LSEL, originalLineNo, originalColNo);
      readChar();
      return token;
    } else if (charCodes[currentChar] == CHAR_TIMES) {
      skipComment();

      if (currentChar == EOF) {
        token = makeToken(TK_NONE, lineNo, colNo);
        error(ERR_ENDOFCOMMENT, lineNo, colNo);
        return token;
      } else {
        readChar();
        return getToken();
      }
    } else {
      token = makeToken(SB_LPAR, originalLineNo, originalColNo);
      return token;
    }
  case CHAR_RPAR:
    token = makeToken(SB_RPAR, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_DOUBLEQUOTE: return readConstString();
  default:
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo);
    readChar(); 
    return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
  case TK_FLOAT: printf("TK_FLOAT(%s)\n", token->string); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_STRING: printf("TK_STRING(\"%s\")\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_FLOAT: printf("KW_FLOAT\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}



