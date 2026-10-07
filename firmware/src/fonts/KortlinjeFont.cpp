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
    switch (character)
    {
    case ' ': // U+0020 SPACE
        return whitespace(4U);
    case '%': // U+0025 PERCENT SIGN
        return toSymbol(percentSign);
    case '.': // U+002E FULL STOP
        return toSymbol(fullStop);
    case ':': // U+003A COLON
        return toSymbol(colon, 1U);
    case 'Y': // U+0059 LATIN CAPITAL LETTER Y
        return toSymbol(latinCapitalLetterY);
    case U'°': // U+00B0 DEGREE SIGN
        return toSymbol(degreeSign, 4);
    case U'🔄': // U+1F504 ANTICLOCKWISE DOWNWARDS AND UPWARDS OPEN CIRCLE ARROWS
        return toSymbol(anticlockwiseDownwardsAndUpwardsOpenCircleArrows);
    default:
        return {};
    }
}
