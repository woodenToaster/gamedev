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
    
    Count
};

struct Token {
    Token *next;
    char *data;
    u32 size;
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

void AddTokenToList(Arena *arena, TokenList *list, TokenType type) {
    Token *tok = PushClearedStruct<Token>(arena);
    tok->type = type;
    list->head->next = tok;
    list->head = tok;
}

inline bool BeginsComment(char c) {
    bool result = c == '#';
    
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
    bool result = IsWhitespace(c) || c == '=';
    
    return result;
}

inline bool BeginsNumber(char c) {
    bool result = (c >= '0' && c <= '9') || (c == '.');
    
    return result;
}

inline bool EndOfNumber(char c) {
    bool result = IsWhitespace(c) || (c == ',') || (c == ';') || (c == '}');
    
    return result;
}

TokenList Tokenize(Arena *arena, EntireFile entire_file) {
    TokenList result = {};
    Token dummy = {};
    result.head = &dummy;
    
    char *at = (char *)entire_file.contents;
    char *end = at + entire_file.size;
    
    while (at < end) {
        if (IsWhitespace(*at)) {
            ++at;
            continue;
        }
        
        if (BeginsComment(*at)) {
            while (at < end && !EndOfComment(*at)) {
                ++at;
            }
            continue;
        }
        
        if (BeginsIdentifier(*at)) {
            AddTokenToList(arena, &result, TokenType::Identifier);
            result.head->data = at;
            
            while (at && !EndOfIdentifier(*at)) {
                result.head->size++;
                at++;
            }
        } else if (BeginsNumber(*at)) {
            AddTokenToList(arena, &result, TokenType::Number);
            result.head->data = at;
            
            while (at && !EndOfNumber(*at)) {
                result.head->size++;
                at++;
            }
        } else {
            switch (*at) {
                case ';': {
                    AddTokenToList(arena, &result, TokenType::Semicolon);
                    break;
                }
                case '=': {
                    AddTokenToList(arena, &result, TokenType::Equal);
                    break;
                }
                case ',': {
                    AddTokenToList(arena, &result, TokenType::Comma);
                    break;
                }
                case '{': {
                    AddTokenToList(arena, &result, TokenType::OpenCurlyBrace);
                    break;
                }
                case '}': {
                    AddTokenToList(arena, &result, TokenType::CloseCurlyBrace);
                    break;
                }
                case '*':
                {
                    AddTokenToList(arena, &result, TokenType::Star);
                } break;
                case '(':
                {
                    AddTokenToList(arena, &result, TokenType::OpenParen);
                } break;
                case ')':
                {
                    AddTokenToList(arena, &result, TokenType::CloseParen);
                } break;
                case '[':
                {
                    AddTokenToList(arena, &result, TokenType::OpenBracket);
                } break;
                case ']':
                {
                    AddTokenToList(arena, &result, TokenType::CloseBracket);
                } break;
                case '<':
                {
                    AddTokenToList(arena, &result, TokenType::LessThan);
                } break;
                case '>':
                {
                    AddTokenToList(arena, &result, TokenType::GreaterThan);
                } break;
                case '/':
                {
                    AddTokenToList(arena, &result, TokenType::ForwardSlash);
                } break;
                case '@':
                {
                    AddTokenToList(arena, &result, TokenType::At);
                } break;
                case '!':
                {
                    AddTokenToList(arena, &result, TokenType::Exclamation);
                } break;
                case '\'':
                {
                    AddTokenToList(arena, &result, TokenType::SingleQuote);
                } break;
                case '\\':
                {
                    AddTokenToList(arena, &result, TokenType::BackSlash);
                } break;
                case '|':
                {
                    AddTokenToList(arena, &result, TokenType::Pipe);
                } break;
                case '&':
                {
                    AddTokenToList(arena, &result, TokenType::Ampersand);
                } break;
                
                case '"': {
                    AddTokenToList(arena, &result, TokenType::String);
                    at++;
                    result.head->data = at;
                    
                    while (at && *at != '"') {
                        result.head->size++;
                        at++;
                    }
                    break;
                }
                default: {
                    fprintf(stderr, "Unexpected token encountered: %c\n", *at);
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
    
    
    size_t memory_size = 4 * 20 * 1024;
    u8 *memory = (u8 *)malloc(memory_size);
    Arena arena = {};
    initArena(&arena, memory_size, memory);
    
    EntireFile file = ReadEntireFile(&arena, argv[1]);
    TokenList tokens = Tokenize(&arena, file);
    
    for (Token *tok = tokens.head; tok; tok = tok->next)
    {
        printf("%.*s\n", tok->size, tok->data);        
    }
    
    free(memory);
    return 0;
}
