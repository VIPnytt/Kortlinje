#include "fonts/KortlinjeFont.h"

FontModule::Symbol KortlinjeFont::getChar(char32_t character) const
{
    if (character >= '0' && character <= '9')
    {
        // U+0030-U+0039
        return toSymbol(digitZero_digitNine[character - '0']);
    }
    if (character >= 'A' && character <= 'J')
    {
        // U+0041-U+004A
        return toSymbol(latinCapitalLetterA_latinCapitalLetterJ[character - 'A']);
    }
    if (character >= 'L' && character <= 'P')
    {
        // U+004C-U+0050
        return toSymbol(latinCapitalLetterL_latinCapitalLetterP[character - 'L']);
    }
    if (character >= 'R' && character <= 'W')
    {
        // U+0052-U+0057
        return toSymbol(latinCapitalLetterR_latinCapitalLetterW[character - 'R']);
    }
    if (character >= 'a' && character <= 'j')
    {
        // U+0041-U+004A
        return toSymbol(latinCapitalLetterA_latinCapitalLetterJ[character - 'a']);
    }
    if (character >= 'a' && character <= 'd')
    {
        // U+0041-U+0044
        return toSymbol(latinCapitalLetterA_latinCapitalLetterJ[character - 'a']);
    }
    if (character >= 'f' && character <= 'i')
    {
        // U+0046-U+0049
        return toSymbol(latinCapitalLetterA_latinCapitalLetterJ[character - 'a']);
    }
    if (character >= 'l' && character <= 'p')
    {
        // U+004C-U+0050
        return toSymbol(latinCapitalLetterL_latinCapitalLetterP[character - 'l']);
    }
    if (character >= 'r' && character <= 'w')
    {
        // U+0052-U+0057
        return toSymbol(latinCapitalLetterR_latinCapitalLetterW[character - 'r']);
    }
    switch (character)
    {
    case ' ': // U+0020 SPACE
        return whitespace(4U);
    case '!': // U+0021 EXCLAMATION MARK
        return toSymbol(exclamationMark);
    case '%': // U+0025 PERCENT SIGN
        return toSymbol(percentSign);
    case '.': // U+002E FULL STOP
        return toSymbol(fullStop);
    case ':': // U+003A COLON
        return toSymbol(colon, 1U);
    case 'Y': // U+0059 LATIN CAPITAL LETTER Y
        return toSymbol(latinCapitalLetterY);
    case 'e': // U+0065 LATIN SMALL LETTER E
        return toSymbol(latinSmallLetterE);
    case 'j': // U+006A LATIN SMALL LETTER J
        return toSymbol(latinSmallLetterJ);
    case 'y': // U+0079 LATIN SMALL LETTER Y
        return toSymbol(latinCapitalLetterY);
    case U'°': // U+00B0 DEGREE SIGN
        return toSymbol(degreeSign, 7U);
    case U'♥': // U+2665 BLACK HEART SUIT
        return toSymbol(blackHeartSuit);
    case U'🔄': // U+1F504 ANTICLOCKWISE DOWNWARDS AND UPWARDS OPEN CIRCLE ARROWS
        return toSymbol(anticlockwiseDownwardsAndUpwardsOpenCircleArrows);
    default:
        return {};
    }
}
