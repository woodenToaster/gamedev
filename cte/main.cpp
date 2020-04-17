#include <float.h>
#include <stdio.h>
#include <string.h>

#include "gamedev_memory.h"

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

struct Token {
    Token *next;
    char *data;
    u32 size;
    u32 line;
    TokenType type;
};

struct TokenList {
    Token *head;
    int count;
};

template<typename T>
T *PushArray(Arena *arena, size_t size)
{
    T *result = pushSize(arena, sizeof(T) * size);
    
    return result;
}

template<typename T>
T *PushClearedStruct(Arena *arena)
{
    T *result = (T *)pushSizeAndClear(arena, sizeof(T));
    
    return result;
}

void printTokenType(TokenType token_type)
{
    switch (token_type)
    {
        case TokenType::Identifier: { printf("Identifier: "); } break;
        case TokenType::Number: { printf("Number: "); } break;
        case TokenType::String: { printf("String: "); } break;
        case TokenType::OpenCurlyBrace: { printf("OpenCurlyBrace: "); } break;
        case TokenType::CloseCurlyBrace: { printf("CloseCurlyBrace: "); } break;
        case TokenType::OpenParen: { printf("OpenParen: "); } break;
        case TokenType::CloseParen: { printf("CloseParen: "); } break;
        case TokenType::OpenBracket: { printf("OpenBracket: "); } break;
        case TokenType::CloseBracket: { printf("CloseBracket: "); } break;
        case TokenType::Comma: { printf("Comma: "); } break;
        case TokenType::Equal: { printf("Equal: "); } break;
        case TokenType::Semicolon: { printf("Semicolon: "); } break;
        case TokenType::Star: { printf("Star: "); } break;
        case TokenType::GreaterThan: { printf("GreaterThan: "); } break;
        case TokenType::LessThan: { printf("LessThan: "); } break;
        case TokenType::ForwardSlash: { printf("ForwardSlash: "); } break;
        case TokenType::BackSlash: { printf("BackSlash: "); } break;
        case TokenType::At: { printf("At: "); } break;
        case TokenType::Exclamation: { printf("Exclamation: "); } break;
        case TokenType::SingleQuote: { printf("SingleQuote: "); } break;
        case TokenType::Pipe: { printf("Pipe: "); } break;
        case TokenType::Ampersand: { printf("Ampersand: "); } break;
        case TokenType::Plus: { printf("Plus: "); } break;
        case TokenType::Minus: { printf("Minus: "); } break;
        case TokenType::Colon: { printf("Colon: "); } break;
        case TokenType::Pound: { printf("Pound: "); } break;
        case TokenType::Dot: { printf("Dot: "); } break;
        case TokenType::Char: { printf("Char: "); } break;
        default: { break; }
    }
}

EntireFile ReadEntireFile(Arena *arena, const char *path) {
    EntireFile result = {};
    FILE *fstream = fopen(path, "r");
    
    if (fstream) {
        int fseek_result = fseek(fstream, 0, SEEK_END);
        
        if (fseek_result == 0) {
            long file_size = ftell(fstream);
            
            if (file_size > 0) {
                
                if ((u32)file_size <= arena->maxCap) {
                    fseek(fstream, 0, SEEK_SET);
                    result.contents = PushArray<u8>(arena, file_size);
                    size_t items_read = fread(result.contents, file_size, 1, fstream);
                    assert(items_read == 1);
                    result.size = file_size;
                } else {
                    fprintf(stderr, "Arena capacity (%zu) too small to read file of size %d\n",
                            arena->maxCap, file_size);
                }
                
            } else {
                // TODO(chogan): @errorhandling
                assert(!"ftell failed");
            }
        } else {
            // TODO(chogan): @errorhandling
            assert(!"fseek failed");
        }
        
        if (fclose(fstream) != 0) {
            // TODO(chogan): @errorhandling
        }
        
    } else {
        // TODO(chogan): @errorhandling
        assert(!"fopen failed");
    }
    
    return result;
}

void AddTokenToList(Arena *arena, TokenList *list, TokenType type, u32 line_number) {
    Token *tok = PushClearedStruct<Token>(arena);
    tok->type = type;
    tok->line = line_number;
    list->head->next = tok;
    list->head = tok;
}

inline bool BeginsComment(char *at) {
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

inline bool EndOfComment(char c) {
    bool result = (c == '\n') || ( c == '\r' );
    
    return result;
}

inline bool IsWhitespace(char c) {
    bool result = c == ' ' || c == '\t' || c == '\n' || c == '\r';
    
    return result;
}

inline bool BeginsIdentifier(char c) {
    bool result = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
    
    return result;
}

inline bool EndOfIdentifier(char c) {
    bool result = !((c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c == '_') ||
                    (c >= '0' && c <= '9'));
    return result;
}

inline bool BeginsNumber(char c) {
    bool result = (c >= '0' && c <= '9');
    
    return result;
}

inline bool EndOfNumber(char c) {
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

TokenList Tokenize(Arena *arena, EntireFile entire_file) {
    u32 line_number = 0;
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
                    
                    while (at && *at != '"') {
                        result.head->size++;
                        at++;
                    }
                    break;
                }
                default: {
                    fprintf(stderr, "Unexpected token encountered on line %d: %c\n", line_number, *at);
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

inline bool IsIdentifier(Token *tok) {
    bool result = tok->type == TokenType::Identifier;
    
    return result;
}

inline bool IsNumber(Token *tok) {
    bool result = tok->type == TokenType::Number;
    
    return result;
}

inline bool IsString(Token *tok) {
    bool result = tok->type == TokenType::String;
    
    return result;
}

inline bool IsOpenCurlyBrace(Token *tok) {
    bool result = tok->type == TokenType::OpenCurlyBrace;
    
    return result;
}

inline bool IsCloseCurlyBrace(Token *tok) {
    bool result = tok->type == TokenType::CloseCurlyBrace;
    
    return result;
}

inline bool IsComma(Token *tok) {
    bool result = tok->type == TokenType::Comma;
    
    return result;
}

inline bool IsEqual(Token *tok) {
    bool result = tok->type == TokenType::Equal;
    
    return result;
}

inline bool IsSemicolon(Token *tok) {
    bool result = tok->type == TokenType::Semicolon;
    
    return result;
}

void PrintExpectedAndFail(const char *expected) {
    fprintf(stderr, "Configuration parser expected: %s\n", expected);
    exit(1);
}

size_t ParseSizet(Token **tok) {
    size_t result = 0;
    if (*tok && IsNumber(*tok)) {
        if (sscanf((*tok)->data, "%zu", &result) == 1) {
            *tok = (*tok)->next;
        } else {
            // LOG(FATAL) << "Could not interpret token data '"
                // << std::string((*tok)->data, (*tok)->size) << "' as a size_t"
                // << std::endl;
        }
    } else {
        PrintExpectedAndFail("a number");
    }
    
    return result;
}

Token *ParseSizetList(Token *tok, size_t *out, int n) {
    if (IsOpenCurlyBrace(tok)) {
        tok = tok->next;
        for (int i = 0; i < n; ++i) {
            out[i] = ParseSizet(&tok);
            if (i != n - 1) {
                if (IsComma(tok)) {
                    tok = tok->next;
                } else {
                    PrintExpectedAndFail(",");
                }
            }
        }
        
        if (IsCloseCurlyBrace(tok)) {
            tok = tok->next;
        } else {
            PrintExpectedAndFail("}");
        }
    } else {
        PrintExpectedAndFail("{");
    }
    
    return tok;
}

int ParseInt(Token **tok) {
    long result = 0;
    if (*tok && IsNumber(*tok)) {
        result = strtol((*tok)->data, NULL, 0);
        if (errno == ERANGE || result <= 0 || result >= INT_MAX) {
            PrintExpectedAndFail("an integer between 1 and INT_MAX");
        }
        *tok = (*tok)->next;
    } else {
        PrintExpectedAndFail("a number");
    }
    
    return (int)result;
}

Token *ParseIntList(Token *tok, int *out, int n) {
    if (IsOpenCurlyBrace(tok)) {
        tok = tok->next;
        for (int i = 0; i < n; ++i) {
            out[i] = ParseInt(&tok);
            if (i != n - 1) {
                if (IsComma(tok)) {
                    tok = tok->next;
                } else {
                    PrintExpectedAndFail(",");
                }
            }
        }
        if (IsCloseCurlyBrace(tok)) {
            tok = tok->next;
        } else {
            PrintExpectedAndFail("}");
        }
    } else {
        PrintExpectedAndFail("{");
    }
    
    return tok;
}

// Token *ParseIntListList(Token *tok, int out[][hermes::kMaxBufferPoolSlabs],
                        // int n, int *m) {
    // if (IsOpenCurlyBrace(tok)) {
        // tok = tok->next;
        // for (int i = 0; i < n; ++i) {
            // tok = ParseIntList(tok, out[i], m[i]);
            // if (i != n - 1) {
                // assert(IsComma(tok));
                // tok = tok->next;
            // } else {
                // Optional final comma
                // if (IsComma(tok)) {
                    // tok = tok->next;
                // }
            // }
        // }
        // if (IsCloseCurlyBrace(tok)) {
            // tok = tok->next;
        // } else {
            // PrintExpectedAndFail("}");
        // }
    // } else {
        // PrintExpectedAndFail("{");
    // }
    // 
    // return tok;
// }

f32 ParseFloat(Token **tok) {
    double result = 0;
    if (*tok && IsNumber(*tok)) {
        result = strtod((*tok)->data, NULL);
        if (result <= 0 || errno == ERANGE || result > FLT_MAX) {
            PrintExpectedAndFail("a floating point number between 1 and FLT_MAX");
        }
        *tok = (*tok)->next;
    } else {
        PrintExpectedAndFail("a number");
    }
    
    return (f32)result;
}

Token *ParseFloatList(Token *tok, f32 *out, int n) {
    if (IsOpenCurlyBrace(tok)) {
        tok = tok->next;
        for (int i = 0; i < n; ++i) {
            out[i] = ParseFloat(&tok);
            if (i != n - 1) {
                if (IsComma(tok)) {
                    tok = tok->next;
                } else {
                    PrintExpectedAndFail(",");
                }
            }
        }
        if (IsCloseCurlyBrace(tok)) {
            tok = tok->next;
        } else {
            PrintExpectedAndFail("}");
        }
    } else {
        PrintExpectedAndFail("{");
    }
    
    return tok;
}

// Token *ParseFloatListList(Token *tok, f32 out[][hermes::kMaxBufferPoolSlabs],
                          // int n, int *m) {
    // if (IsOpenCurlyBrace(tok)) {
        // tok = tok->next;
        // for (int i = 0; i < n; ++i) {
            // tok = ParseFloatList(tok, out[i], m[i]);
            // if (i != n - 1) {
                // if (IsComma(tok)) {
                    // tok = tok->next;
                // } else {
                    // PrintExpectedAndFail(",");
                // }
            // } else {
                // Optional final comma
                // if (IsComma(tok)) {
                    // tok = tok->next;
                // }
            // }
        // }
        // if (IsCloseCurlyBrace(tok)) {
            // tok = tok->next;
        // } else {
            // PrintExpectedAndFail("}");
        // }
    // } else {
        // PrintExpectedAndFail("{");
    // }
    // 
    // return tok;
// }

// Token *ParseStringList(Token *tok, std::string *out, int n) {
    // if (IsOpenCurlyBrace(tok)) {
        // tok = tok->next;
        // for (int i = 0; i < n; ++i) {
            // out[i] = ParseString(&tok);
            // if (i != n - 1) {
                // if (IsComma(tok)) {
                    // tok = tok->next;
                // } else {
                    // PrintExpectedAndFail(",");
                // }
            // }
        // }
        // if (IsCloseCurlyBrace(tok)) {
            // tok = tok->next;
        // } else {
            // PrintExpectedAndFail("}");
        // }
    // } else {
        // PrintExpectedAndFail("{");
    // }
    // 
    // return tok;
// }

Token *ParseCharArrayString(Token *tok, char *arr) {
    if (tok && IsString(tok)) {
        strncpy(arr, tok->data, tok->size);
        arr[tok->size] = '\0';
        tok = tok->next;
    } else {
        PrintExpectedAndFail("a string");
    }
    
    return tok;
}

// void RequireNumTiers(Config *config) {
    // if (config->num_tiers == 0) {
        // LOG(FATAL) << "The configuration variable 'num_tiers' must be defined first"
            // << std::endl;
    // }
// }

// void RequireNumSlabs(Config *config) {
    // if (config->num_slabs == 0) {
        // LOG(FATAL) << "The configuration variable 'num_slabs' must be defined first"
            // << std::endl;
    // }
// }

Token *BeginStatement(Token *tok) {
    if (tok && IsIdentifier(tok)) {
        tok = tok->next;
        if (tok && IsEqual(tok)) {
            tok = tok->next;
        } else {
            PrintExpectedAndFail("=");
        }
    } else {
        PrintExpectedAndFail("an identifier");
    }
    
    return tok;
}

Token *EndStatement(Token *tok) {
    if (tok && IsSemicolon(tok)) {
        tok = tok->next;
    } else {
        PrintExpectedAndFail(";");
    }
    
    return tok;
}

// void ParseTokens(TokenList *tokens, Config *config) {
    // 
    // Token *tok = tokens->head;
    // while (tok) {
        // ConfigVariable var = GetConfigVariable(tok);
        // 
        // if (var == ConfigVariable_Unkown) {
            // tok = tok->next;
            // while (tok && !IsIdentifier(tok)) {
                // tok = tok->next;
            // }
            // continue;
        // }
        // 
        // switch (var) {
            // case ConfigVariable_NumTiers: {
                // tok = BeginStatement(tok);
                // int val = ParseInt(&tok);
                // config->num_tiers = val;
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_Capacities: {
                // RequireNumTiers(config);
                // tok = BeginStatement(tok);
                // tok = ParseSizetList(tok, config->capacities, config->num_tiers);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_BlockSizes: {
                // RequireNumTiers(config);
                // tok = BeginStatement(tok);
                // tok = ParseIntList(tok, config->block_sizes, config->num_tiers);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_NumSlabs: {
                // RequireNumTiers(config);
                // tok = BeginStatement(tok);
                // tok = ParseIntList(tok, config->num_slabs, config->num_tiers);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_SlabUnitSizes: {
                // RequireNumTiers(config);
                // RequireNumSlabs(config);
                // tok = BeginStatement(tok);
                // tok = ParseIntListList(tok, config->slab_unit_sizes, config->num_tiers,
                                       // config->num_slabs);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_DesiredSlabPercentages: {
                // RequireNumTiers(config);
                // RequireNumSlabs(config);
                // tok = BeginStatement(tok);
                // tok = ParseFloatListList(tok, config->desired_slab_percentages,
                                         // config->num_tiers, config->num_slabs);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_BandwidthsMbps: {
                // RequireNumTiers(config);
                // tok = BeginStatement(tok);
                // tok = ParseFloatList(tok, config->bandwidths, config->num_tiers);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_LatenciesUs: {
                // RequireNumTiers(config);
                // tok = BeginStatement(tok);
                // tok = ParseFloatList(tok, config->latencies, config->num_tiers);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_BufferPoolArenaPercentage: {
                // tok = BeginStatement(tok);
                // f32 val = ParseFloat(&tok);
                // config->arena_percentages[hermes::kArenaType_BufferPool] = val;
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_MetadataArenaPercentage: {
                // tok = BeginStatement(tok);
                // f32 val = ParseFloat(&tok);
                // config->arena_percentages[hermes::kArenaType_MetaData] = val;
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_TransferWindowArenaPercentage: {
                // tok = BeginStatement(tok);
                // f32 val = ParseFloat(&tok);
                // config->arena_percentages[hermes::kArenaType_TransferWindow] = val;
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_TransientArenaPercentage: {
                // tok = BeginStatement(tok);
                // f32 val = ParseFloat(&tok);
                // config->arena_percentages[hermes::kArenaType_Transient] = val;
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_MountPoints: {
                // RequireNumTiers(config);
                // tok = BeginStatement(tok);
                // tok = ParseStringList(tok, config->mount_points, config->num_tiers);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_RpcServerName: {
                // tok = BeginStatement(tok);
                // config->rpc_server_name = ParseString(&tok);
                // tok = EndStatement(tok);
                // break;
            // }
            // case ConfigVariable_BufferPoolShmemName: {
                // tok = BeginStatement(tok);
                // tok = ParseCharArrayString(tok, config->buffer_pool_shmem_name);
                // tok = EndStatement(tok);
                // break;
            // }
            // default: {
                // assert(!"Invalid code path\n");
                // break;
            // }
        // }
    // }
// }

// void ParseConfig(Arena *arena, const char *path, Config *config) {
    // ScopedTemporaryMemory scratch(arena);
    // EntireFile config_file = ReadEntireFile(scratch, path);
    // TokenList tokens = Tokenize(scratch, config_file);
    // ParseTokens(&tokens, config);
// }

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Expected a filename\n");
        exit(1);
    }
    
    size_t memory_size = 135 * 1024;
    u8 *memory = (u8 *)malloc(memory_size);
    Arena arena = {};
    initArena(&arena, memory_size, memory);
    
    EntireFile file = ReadEntireFile(&arena, argv[1]);
    TokenList tokens = Tokenize(&arena, file);
    
    for (Token *tok = tokens.head; tok; tok = tok->next)
    {
        if (tok->size)
        {
            printf("%d:", tok->line);
            printTokenType(tok->type);
            printf("%.*s\n", tok->size, tok->data);
        }
    }
    
    free(memory);
    return 0;
}
