#include "Token.hpp"

u16 toi(TokenType ty)
{
    return static_cast<u16>(ty);
}

void printTok(const Token& tok)
{
    if (tok.type == TokenType::Identifier)
    {
        std::cout
            << std::vformat("({}, {}, {}, \"",
                std::make_format_args(toi(tok.type), tok.ln, tok.count))
            << tok.val
            << "\")"
            << '\n';
        return;
    }

    if (tok.type == TokenType::IntLiteral)
    {
        std::cout
            << std::vformat("({}, {}, {}, \"",
                std::make_format_args(toi(tok.type), tok.ln, tok.count))
            << tok.uint
            << "\")"
            << '\n';
    }

    std::cout
        << std::vformat("({}, {}, {})",
            std::make_format_args(toi(tok.type), tok.ln, tok.count))
        << ")"
        << '\n';
}