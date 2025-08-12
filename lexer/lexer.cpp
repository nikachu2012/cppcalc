#include "lexer.hpp"

char buf[BUFFERSIZ];
size_t bufindex = 0;

FILE *f = NULL;

inline void addc(char c)
{
    buf[bufindex] = c;
    bufindex++;
}

void resetbuf()
{
    bufindex = 0;
}

char read()
{
    return fgetc(f);
}

void pb()
{
    fseek(f, -1, SEEK_CUR);
}

int isdecimal(char c)
{
    return '0' <= c && c <= '9';
}

int ishex(char c)
{
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

// compatible ascii
int isKeyword(char c)
{
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('_' == c) ||
           (0x80 <= (unsigned char)c && (unsigned)c <= 0xff);
}

long int prev_target;

void lexer_pb()
{
    // t = prev_target;
    fseek(f, prev_target, SEEK_SET);
}

LEXER_TYPE lexer(LEXER_RESULT *val)
{
    char c;
    prev_target = ftell(f);
    resetbuf();
    while ((c = read()) != EOF)
    {
        switch (c)
        {
        case ' ':
        case '\t':
        case '\n':
            continue;
            break;
        case '/':
            c = read();
            if (c == '/')
            {
                while (read() != '\n')
                {
                }
            }
            else if (c == '*')
            {
                // multi line comment
                while (true)
                {
                    while (read() != '*')
                    {
                    }
                    if (read() == '/')
                        break;
                    else
                        continue;
                }
            }
            else
            {
                pb();
                addc('/');
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // digit
            addc(c);
            while (isdecimal(c = read()))
            {
                addc(c);
            }
            pb();
            val->text = buf;
            addc(0); // add null char
            return LEXER_TYPE_INTEGER;
            break;
        case '(':
        case '{':
        case '[':
            val->op = c;
            return LEXER_TYPE_LEFT_BRACKET;
            break;
        case ')':
        case '}':
        case ']':
            val->op = c;
            return LEXER_TYPE_RIGHT_BRACKET;
            break;
        case '<':
        case '>': {
            // 次の文字を取得
            char cc = read();

            if (c == cc)
            {
                // 左右シフト演算子
                addc(c);
                addc(cc);

                if ((read()) == '=')
                {
                    // 左右シフト代入演算子
                    addc('=');
                    addc(0);
                    val->text = buf;
                    return LEXER_TYPE_ASSIGN_OPERATOR;
                }
                else
                    pb();
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            else if (cc == '=')
            {
                // 以下、以上
                addc(c);
                addc(cc);
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            else
            {
                // 未満、超過
                pb();
                addc(c);
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
        }
        break;
        case '+':
        case '*':
            addc(c);

            if ((read()) == '=')
            {
                // 代入演算子
                addc('=');
                addc(0);
                val->text = buf;
                return LEXER_TYPE_ASSIGN_OPERATOR;
            }
            else
                pb();

            addc(0);
            val->text = buf;
            return LEXER_TYPE_OPERATOR;
            break;
        case '-':
            if ((read()) == '>')
            {
                // 右向きアロー演算子
                // addc('-');
                // addc('>');
                // addc(0);
                // val->text = buf;
                return LEXER_TYPE_RIGHTARROW;
            }
            addc(c);
            addc(0);
            val->text = buf;
            return LEXER_TYPE_OPERATOR;
            break;
        case '=':
            if ((read()) == '=')
            {
                // ==の時
                // 代入演算子
                addc('=');
                addc('=');
                addc(0);
                val->text = buf;
                return LEXER_TYPE_OPERATOR;
            }
            else
                pb();

            addc(c);
            addc(0);
            val->text = buf;
            return LEXER_TYPE_ASSIGN_OPERATOR;
            break;
        case '%':
            addc(c);
            addc(0);
            val->text = buf;
            return LEXER_TYPE_OPERATOR;
            break;
        case '"':
            // STRING
            while ((c = read()) != '"')
            {
                addc(c);
            }
            // "まで読むのでpushbackは不要
            addc(0);
            val->text = buf;
            return LEXER_TYPE_STRING;
            break;
        case ',':
            return LEXER_TYPE_COMMA;
            break;
        case ';':
            return LEXER_TYPE_SEMICOLON;
            break;
        default:
            // KEYWORD
            addc(c);
            while (isKeyword(c = read()))
            {
                addc(c);
            }
            pb();
            val->text = buf;
            addc(0); // add null char
            return LEXER_TYPE_KEYWORD;
        }
    }

    // ↓ヌル文字を読んだ後は何回呼ばれてもENDを返すようにする
    pb();
    return LEXER_TYPE_END;
}
