#include <stdio.h>
#include "token.h"

const char* token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_NUMBER:
            return "NUMBER";

        case TOKEN_STRING:
            return "STRING";

        case TOKEN_OPERATOR:
            return "OPERATOR";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_REDIRECT_IN:
            return "REDIRECT_IN";

        case TOKEN_REDIRECT_OUT:
            return "REDIRECT_OUT";

        case TOKEN_REDIRECT_APPEND:
            return "REDIRECT_APPEND";

        case TOKEN_SEMICOLON:
            return "SEMICOLON";

        case TOKEN_EOF:
            return "EOF";

        default:
            return "UNKNOWN";
    }
}
