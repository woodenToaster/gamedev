#include <float.h>

enum class TokenType {
    Identifier,
    Number,
    String,
    OpenCurlyBrace,
    CloseCurlyBrace,
    OpenParen,
    CloseParen,
    OpenBracket,
    CloseBracket,
    Comma,
    Equal,
    Semicolon,
    Star,
    GreaterThan,
    LessThan,
    ForwardSlash,
    BackSlash,
    At,
    Exclamation,
    SingleQuote,
    Pipe,
    Ampersand,
    Plus,
    Minus,
    Colon,
    Pound,
    Dot,
    Char,

    Count
};

struct String
{
    char *str;
    size_t size;
};

struct Token
{
    Token *next;
    char *data;
    u32 size;
    u32 line;
    TokenType type;
};

struct TokenList
{
    Token *head;
    int count;
};

struct ObjData
{
    f32 *vertices;
    u32 *faces;
    size_t num_vertices;
    size_t num_faces;
};

void AddTokenToList(Arena *arena, TokenList *list, TokenType type, u32 line_number)
{
    Token *tok = PUSH_CLEARED_STRUCT(arena, Token);
    tok->type = type;
    tok->line = line_number;
    list->head->next = tok;
    list->head = tok;
}

inline bool BeginsComment(char *at)
{
    bool result = false;

    if (at[0] && at[1])
    {
        if (at[0] == '/' && at[1] == '/')
        {
            result = true;
        }
    }

    return result;
}

inline bool EndOfComment(char c)
{
    bool result = (c == '\n') || ( c == '\r' );

    return result;
}

inline bool IsWhitespace(char c)
{
    bool result = c == ' ' || c == '\t' || c == '\n' || c == '\r';

    return result;
}

inline bool BeginsIdentifier(char c)
{
    bool result = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');

    return result;
}

inline bool EndOfIdentifier(char c)
{
    bool result = !((c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c == '_') ||
                    (c >= '0' && c <= '9'));
    return result;
}

inline bool BeginsNumber(char c)
{
    bool result = (c >= '0' && c <= '9');

    return result;
}

inline bool EndOfNumber(char c)
{
    bool result = !BeginsNumber(c) && (c != '.');

    return result;
}

inline bool IsEndOfLine(char **at, char *end)
{
    bool result = (*at)[0] == '\n';

    if (*at + 1 < end && !result)
    {
        result = ((*at)[0] == '\r' && (*at)[1] == '\n');
        if (result)
        {
            (*at)++;
        }
    }

    return result;
}

TokenList tokenizeObjFile(Arena *arena, EntireFile entire_file)
{
    u32 line_number = 1;
    TokenList result = {};
    Token dummy = {};
    result.head = &dummy;

    char *at = (char *)entire_file.contents;
    char *end = at + entire_file.size;

    while (at < end) {
        if (IsWhitespace(*at)) {
            if (IsEndOfLine(&at, end))
            {
                line_number++;
            }

            ++at;
            continue;
        }

        // TODO(chogan): Tokenize comments
        if (BeginsComment(at)) {
            while (at < end) {
                if (IsEndOfLine(&at, end))
                {
                    line_number++;
                    ++at;
                    break;
                }
                ++at;
            }
            continue;
        }

        if (BeginsIdentifier(*at)) {
            AddTokenToList(arena, &result, TokenType::Identifier, line_number);
            result.head->data = at;

            while (at && !EndOfIdentifier(*at)) {
                result.head->size++;
                at++;
            }
        } else if (BeginsNumber(*at)) {
            AddTokenToList(arena, &result, TokenType::Number, line_number);
            result.head->data = at;

            while (at && !EndOfNumber(*at)) {
                result.head->size++;
                at++;
            }
        } else {
            switch (*at) {
                case ';': {
                    AddTokenToList(arena, &result, TokenType::Semicolon, line_number);
                    break;
                }
                case '=': {
                    AddTokenToList(arena, &result, TokenType::Equal, line_number);
                    break;
                }
                case ',': {
                    AddTokenToList(arena, &result, TokenType::Comma, line_number);
                    break;
                }
                case '{': {
                    AddTokenToList(arena, &result, TokenType::OpenCurlyBrace, line_number);
                    break;
                }
                case '}': {
                    AddTokenToList(arena, &result, TokenType::CloseCurlyBrace, line_number);
                    break;
                }
                case '*':
                {
                    AddTokenToList(arena, &result, TokenType::Star, line_number);
                } break;
                case '(':
                {
                    AddTokenToList(arena, &result, TokenType::OpenParen, line_number);
                } break;
                case ')':
                {
                    AddTokenToList(arena, &result, TokenType::CloseParen, line_number);
                } break;
                case '[':
                {
                    AddTokenToList(arena, &result, TokenType::OpenBracket, line_number);
                } break;
                case ']':
                {
                    AddTokenToList(arena, &result, TokenType::CloseBracket, line_number);
                } break;
                case '<':
                {
                    AddTokenToList(arena, &result, TokenType::LessThan, line_number);
                } break;
                case '>':
                {
                    AddTokenToList(arena, &result, TokenType::GreaterThan, line_number);
                } break;
                case '/':
                {
                    AddTokenToList(arena, &result, TokenType::ForwardSlash, line_number);
                } break;
                case '@':
                {
                    AddTokenToList(arena, &result, TokenType::At, line_number);
                } break;
                case '!':
                {
                    AddTokenToList(arena, &result, TokenType::Exclamation, line_number);
                } break;
                case '\'':
                {
                    AddTokenToList(arena, &result, TokenType::Char, line_number);
                    at++;
                    result.head->data = at;
                    result.head->size = 1;
                    at += 2;
                } break;
                case '\\':
                {
                    AddTokenToList(arena, &result, TokenType::BackSlash, line_number);
                } break;
                case '|':
                {
                    AddTokenToList(arena, &result, TokenType::Pipe, line_number);
                } break;
                case '&':
                {
                    AddTokenToList(arena, &result, TokenType::Ampersand, line_number);
                } break;
                case '+':
                {
                    AddTokenToList(arena, &result, TokenType::Plus, line_number);
                } break;
                case '-':
                {
                    AddTokenToList(arena, &result, TokenType::Minus, line_number);
                } break;
                case ':':
                {
                    AddTokenToList(arena, &result, TokenType::Colon, line_number);
                } break;
                case '#':
                {
                    AddTokenToList(arena, &result, TokenType::Pound, line_number);
                } break;
                case '.':
                {
                    AddTokenToList(arena, &result, TokenType::Dot, line_number);
                } break;

                case '"': {
                    AddTokenToList(arena, &result, TokenType::String, line_number);
                    at++;
                    result.head->data = at;

                    eat_string:
                    while (at && *at != '"')
                    {
                        result.head->size++;
                        at++;
                    }
                    if (*(at - 1) == '\\')
                    {
                        result.head->size++;
                        at++;
                        goto eat_string;
                    }
                    break;
                }
                default: {
                    fprintf(stderr, "Unexpected token encountered on line %d: %c\n", line_number,
                            *at);
                    exit(1);
                    break;
                }
            }
            at++;
        }
        result.count++;
    }

    result.head = dummy.next;

    return result;
}

inline bool IsIdentifier(Token *tok)
{
    bool result = tok->type == TokenType::Identifier;

    return result;
}

inline bool IsMinus(Token *tok)
{
    bool result = tok->type == TokenType::Minus;

    return result;
}

inline bool IsNumber(Token *tok)
{
    bool result = tok->type == TokenType::Number;

    return result;
}

inline bool IsString(Token *tok)
{
    bool result = tok->type == TokenType::String;

    return result;
}

inline bool IsOpenCurlyBrace(Token *tok)
{
    bool result = tok->type == TokenType::OpenCurlyBrace;

    return result;
}

inline bool IsCloseCurlyBrace(Token *tok)
{
    bool result = tok->type == TokenType::CloseCurlyBrace;

    return result;
}

inline bool IsComma(Token *tok)
{
    bool result = tok->type == TokenType::Comma;

    return result;
}

inline bool IsEqual(Token *tok)
{
    bool result = tok->type == TokenType::Equal;

    return result;
}

inline bool IsSemicolon(Token *tok)
{
    bool result = tok->type == TokenType::Semicolon;

    return result;
}

void printExpectedAndFail(const char *expected, u32 line=0)
{
    fprintf(stderr, "Configuration parser expected: %s on line %lu\n", expected, line);
    exit(1);
}

f32 parseFloat(Token **tok)
{
    double result = 0;
    if (IsMinus(*tok))
    {
        (*tok) = (*tok)->next;
        (*tok)->data--;
        (*tok)->size++;
    }

    if (*tok && IsNumber(*tok)) {
        result = strtod((*tok)->data, NULL);
        if (errno == ERANGE || result > FLT_MAX) {
            printExpectedAndFail("a floating point number between 1 and FLT_MAX");
        }
    } else {
        printExpectedAndFail("a floating point number", (*tok)->line);
    }

    return (f32)result;
}

Token *parseVertex(Arena *arena, Token *tok)
{
    if (!tok || !IsIdentifier(tok) || tok->data[0] != 'v')
    {
        printExpectedAndFail("v");
    }
    tok = tok->next;
    f32 x = parseFloat(&tok);
    tok = tok->next;
    f32 y = parseFloat(&tok);
    tok = tok->next;
    f32 z = parseFloat(&tok);

    f32 * vertex = PUSH_ARRAY(arena, f32, 3);
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;

    return tok;
}

u32 parseUnsignedInt(Token *tok)
{
    unsigned long result = 0;
    if (tok && IsNumber(tok)) {
        result = strtoul(tok->data, NULL, 0);
        if (errno == ERANGE || result > 0xFFFFFFFF) {
            printExpectedAndFail("an integer between 0 and 0xFFFFFFFF");
        }
    } else {
        printExpectedAndFail("an unsigned number", tok->line);
    }

    return (u32)result;
}

Token *parseFace(Arena *arena, Token *tok)
{
    if (!tok || !IsIdentifier(tok) || tok->data[0] != 'f')
    {
        printExpectedAndFail("f");
    }
    tok = tok->next;
    u32 x = parseUnsignedInt(tok) - 1;
    tok = tok->next;
    u32 y = parseUnsignedInt(tok) - 1;
    tok = tok->next;
    u32 z = parseUnsignedInt(tok) - 1;

    u32 *face = PUSH_ARRAY(arena, u32, 3);
    face[0] = x;
    face[1] = y;
    face[2] = z;

    return tok;
}

ObjData parseObjFile(Arena *arena, TokenList tokens)
{
    ObjData result = {};
    result.vertices = (f32 *)(arena->start + arena->used);
    Token *tok = tokens.head;


    while (tok)
    {
        switch (tok->type)
        {
            case TokenType::Identifier:
            {
                if (tok->data[0] == 'v')
                {
                    tok = parseVertex(arena, tok);
                    result.num_vertices++;
                }
                else if (tok->data[0] == 'f')
                {
                    if (result.faces == 0)
                    {
                        result.faces = (u32 *)(arena->start + arena->used);
                    }

                    tok = parseFace(arena, tok);
                    result.num_faces++;
                }
            } break;
        }
        tok = tok->next;
    }

    return result;
}

ObjData loadObjFromFile(Arena *temp_arena, Arena *arena, char *filename)
{
    EntireFile obj_file = platform.readEntireFile(filename);
    TemporaryMemory temp_mem = beginTemporaryMemory(temp_arena);
    TokenList tokens = tokenizeObjFile(temp_arena, obj_file);
    ObjData result = parseObjFile(arena, tokens);
    endTemporaryMemory(temp_mem);
    platform.freeFileMemory(&obj_file);

    return result;
}
