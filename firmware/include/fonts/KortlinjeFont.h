#pragma once

#include "modules/FontModule.h"

#include <array>

class KortlinjeFont final : public FontModule
{
private:
    // U+0025 % PERCENT SIGN
    static constexpr std::array<uint16_t, 9U> percentSign{
        0b011100001U,
        0b010100010U,
        0b011100100U,
        0b000001000U,
        0b000010000U,
        0b000100000U,
        0b001001110U,
        0b010001010U,
        0b100001110U,
    };

    // U+002E . FULL STOP
    static constexpr std::array<uint8_t, 2U> fullStop{
        0b11U,
        0b11U,
    };

    static constexpr std::array<std::array<uint8_t, 9U>, 10U> digitZero_digitNine{{
        {
            // U+0030 0 DIGIT ZERO
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        {
            // U+0031 1 DIGIT ONE
            0b001100U,
            0b011100U,
            0b111100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b111111U,
        },
        {
            // U+0032 2 DIGIT TWO
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b0000110U,
            0b0001100U,
            0b0001100U,
            0b0011000U,
            0b0110000U,
            0b1111111U,
        },
        {
            // U+0033 3 DIGIT THREE
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b0000011U,
            0b0001110U,
            0b0000011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        {
            // U+0034 4 DIGIT FOUR
            0b0000110U,
            0b0001110U,
            0b0011110U,
            0b0110110U,
            0b1100110U,
            0b1100110U,
            0b1111111U,
            0b0000110U,
            0b0000110U,
        },
        {
            // U+0035 5 DIGIT FIVE
            0b1111111U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1111110U,
            0b0000011U,
            0b0000011U,
            0b0000011U,
            0b1111110U,
        },
        {
            // U+0036 6 DIGIT SIX
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100000U,
            0b1111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        {
            // U+0037 7 DIGIT SEVEN
            0b1111111U,
            000000011U,
            0b0000011U,
            0b0000011U,
            0b0000110U,
            0b0000110U,
            0b0001100U,
            0b0001100U,
            0b0001100U,
        },
        {
            // U+0038 8 DIGIT EIGHT
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        {
            // U+0039 9 DIGIT NINE
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111111U,
            0b0000011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
    }};

    // U+003A : COLON
    static constexpr std::array<uint8_t, 6U> colon{
        0b11U,
        0b11U,
        0b00U,
        0b00U,
        0b11U,
        0b11U,
    };

    static constexpr std::array<std::array<uint8_t, 9U>, 10U> latinCapitalLetterA_latinCapitalLetterJ{{
        // U+0041 A LATIN CAPITAL LETTER A
        {
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111111U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
        },
        // U+0042 B LATIN CAPITAL LETTER B
        {
            0b1111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111110U,
        },
        // U+0043 C LATIN CAPITAL LETTER C
        {
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        // U+0044 D LATIN CAPITAL LETTER D
        {
            0b1111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111110U,
        },
        // U+0045 E LATIN CAPITAL LETTER E
        {
            0b1111111U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1111111U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1111111U,
        },
        // U+0046 F LATIN CAPITAL LETTER F
        {
            0b1111111U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1111110U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
        },
        // U+0047 G LATIN CAPITAL LETTER G
        {
            0b0011110U,
            0b1100011U,
            0b1100011U,
            0b1100000U,
            0b1100000U,
            0b1100111U,
            0b1100011U,
            0b1100011U,
            0b0011110U,
        },
        // U+0048 H LATIN CAPITAL LETTER H
        {
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111111U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
        },
        // U+0049 I LATIN CAPITAL LETTER I
        {
            0b111111U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b111111U,
        },
        // U+004A J LATIN CAPITAL LETTER J
        {
            0b0001111U,
            0b0000110U,
            0b0000110U,
            0b0000110U,
            0b0000110U,
            0b0000110U,
            0b1100110U,
            0b1100110U,
            0b0111100U,
        },
    }};

    static constexpr std::array<std::array<uint8_t, 9U>, 12U> latinCapitalLetterL_latinCapitalLetterP{{
        // U+004C L LATIN CAPITAL LETTER L
        {
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
            0b1111111U,
        },
        // U+004D M LATIN CAPITAL LETTER M
        {
            0b1100011U,
            0b1110111U,
            0b1111111U,
            0b1101011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
        },
        // U+004E N LATIN CAPITAL LETTER N
        {
            0b1100011U,
            0b1110011U,
            0b1110011U,
            0b1111011U,
            0b1101011U,
            0b1101111U,
            0b1100111U,
            0b1100111U,
            0b1100011U,
        },
        // U+004F O LATIN CAPITAL LETTER O
        {
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        // U+0050 P LATIN CAPITAL LETTER P
        {
            0b1111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111110U,
            0b1100000U,
            0b1100000U,
            0b1100000U,
        },
    }};

    static constexpr std::array<std::array<uint8_t, 9U>, 12U> latinCapitalLetterR_latinCapitalLetterW{{
        // U+0052 R LATIN CAPITAL LETTER R
        {
            0b1111110U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1111110U,
            0b1111000U,
            0b1101100U,
            0b1100110U,
            0b1100011U,
        },
        // U+0053 S LATIN CAPITAL LETTER S
        {
            0b0111110U,
            0b1100011U,
            0b1100011U,
            0b1100000U,
            0b0111110U,
            0b0000011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        // U+0054 T LATIN CAPITAL LETTER T
        {
            0b111111U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
            0b001100U,
        },
        // U+0055 U LATIN CAPITAL LETTER U
        {
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0111110U,
        },
        // U+0056 V LATIN CAPITAL LETTER V
        {
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b0110110U,
            0b0011100U,
            0b0001000U,
        },
        // U+0057 W LATIN CAPITAL LETTER W
        {
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1100011U,
            0b1101011U,
            0b1111111U,
            0b1110111U,
            0b1100011U,
        },
    }};

    // U+0059 Y LATIN CAPITAL LETTER Y
    static constexpr std::array<uint8_t, 9U> latinCapitalLetterY{
        0b110011U,
        0b110011U,
        0b110011U,
        0b110011U,
        0b110011U,
        0b011110U,
        0b001100U,
        0b001100U,
        0b001100U,
    };

    // U+00B0 ° DEGREE SIGN
    static constexpr std::array<uint8_t, 2U> degreeSign{
        0b11U,
        0b11U,
    };

    // U+1F504 🔄 ANTICLOCKWISE DOWNWARDS AND UPWARDS OPEN CIRCLE ARROWS
    static constexpr std::array<uint16_t, 9U> anticlockwiseDownwardsAndUpwardsOpenCircleArrows{
        0b0000100000000000U,
        0b1001000000000000U,
        0b1011111111111111U,
        0b1001000000000001U,
        0b1000100000010001U,
        0b1000000000001001U,
        0b1111111111111101U,
        0b0000000000001001U,
        0b0000000000010000U,
    };

public:
    static constexpr std::string_view name{"Kortlinje"};

    explicit KortlinjeFont() : FontModule(name) {};

    [[nodiscard]] FontModule::Symbol getChar(char32_t character) const override;
};
