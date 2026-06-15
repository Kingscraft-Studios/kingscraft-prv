////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <NsCore/Error.h>


namespace Noesis
{
namespace UTF8
{

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsTrail(char c)
{
    // Checks if byte is a valid UTF-8 continuation (10xxxxxx)
    return ((uint8_t)c & 0xC0) == 0x80;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint8_t Mask8(char c)
{
    return (uint8_t)(0xff & c);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint16_t Mask16(uint16_t c)
{
    return (uint16_t)(0xffff & c);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
char* Append(uint32_t cp, char* text)
{
    NS_ASSERT(text != 0);

    if (cp > 0x10FFFF) 
    {
        cp = 0xFFFD;
    }

    if (cp < 0x80)
    {
        *(text++) = (char)cp;
    }
    else if (cp < 0x800)
    {
        *(text++) = (char)((cp >> 6) | 0xc0);
        *(text++) = (char)((cp & 0x3f) | 0x80);
    }
    else if (cp < 0x10000)
    {
        *(text++) = (char)((cp >> 12) | 0xe0);
        *(text++) = (char)(((cp >> 6) & 0x3f) | 0x80);
        *(text++) = (char)((cp & 0x3f) | 0x80);
    }
    else 
    {
        *(text++) = (char)((cp >> 18) | 0xf0);
        *(text++) = (char)(((cp >> 12) & 0x3f) | 0x80);
        *(text++) = (char)(((cp >> 6) & 0x3f) | 0x80);
        *(text++) = (char)((cp & 0x3f) | 0x80);
    }

    return text;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint32_t SequenceLength(const char* text)
{
    NS_ASSERT(text != 0);
    uint8_t lead = Mask8(*text);

    if (lead < 0x80) return 1;
    // 0xC0 and 0xC1 are invalid overlong start bytes
    else if (lead < 0xC2) return 1; 
    else if ((lead >> 5) == 0x6) return 2;
    else if ((lead >> 4) == 0xe) return 3;
    // 0xF5 and above are invalid (out of range)
    else if (lead < 0xF5 && (lead >> 3) == 0x1e) return 4;

    // Invalid start byte: treat as length 1 to advance safely
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Next(const char*& text)
{
    NS_ASSERT(text != 0);
    
    const char* start = text;
    uint32_t len = SequenceLength(text);
    uint32_t cp = Mask8(*text);

    // Optimized handling for ASCII or Invalid bytes
    if (len == 1)
    {
        ++text;
        // If ASCII, return it. If it was an invalid start byte (>= 0x80), return replacement.
        return (cp < 0x80) ? cp : 0xFFFD;
    }

    // Validate multi-byte structure
    switch (len) 
    {
        case 2:
        {
            if (!IsTrail(*(++text))) { text = start + 1; return 0xFFFD; }
            cp = ((cp << 6) & 0x7ff) + ((*text) & 0x3f);
            if (cp < 0x80) { text = start + 1; return 0xFFFD; } // Overlong check
            ++text;
            break;
        }
        case 3:
        {
            if (!IsTrail(*(++text))) { text = start + 1; return 0xFFFD; }
            cp = ((cp << 12) & 0xffff) + ((Mask8(*text) << 6) & 0xfff);
            
            if (!IsTrail(*(++text))) { text = start + 1; return 0xFFFD; }
            cp += (*text) & 0x3f;

            // Overlong or Surrogate check
            if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) { text = start + 1; return 0xFFFD; }
            ++text;
            break;
        }
        case 4:
        {
            if (!IsTrail(*(++text))) { text = start + 1; return 0xFFFD; }
            cp = ((cp << 18) & 0x1fffff) + ((Mask8(*text) << 12) & 0x3ffff);
            
            if (!IsTrail(*(++text))) { text = start + 1; return 0xFFFD; }
            cp += (Mask8(*text) << 6) & 0xfff;
            
            if (!IsTrail(*(++text))) { text = start + 1; return 0xFFFD; }
            cp += (*text) & 0x3f;

            if (cp < 0x10000 || cp > 0x10FFFF) { text = start + 1; return 0xFFFD; }
            ++text;
            break;
        }
        default:
            NS_ASSERT_UNREACHABLE;
    }

    return cp; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Next(char*& text)
{
    NS_ASSERT(text != 0);
    const char* text_ = text;
    uint32_t cp = Next(text_);
    text = (char*)text_;
    return cp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Prior(const char*& text)
{
    NS_ASSERT(text != 0);
    // Standard UTF-8 Prior logic: skip trails backwards.
    while ((Mask8(*(--text)) >> 6) == 0x2) {}
    return Get(text);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Prior(char*& text)
{
    NS_ASSERT(text != 0);
    const char* text_ = text;
    uint32_t cp = Prior(text_);
    text = (char*)text_;
    return cp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Get(const char* text)
{
    NS_ASSERT(text != 0);
    return Next(text);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Advance(const char*& text, uint32_t n)
{
    NS_ASSERT(text != 0);
    for (uint32_t i = 0; i < n; i++)
    {
        Next(text);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Advance(char*& text, uint32_t n)
{
    NS_ASSERT(text != 0);
    const char* text_ = text;
    Advance(text_, n);
    text = (char*)text_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Distance(const char* first, const char* last)
{
    NS_ASSERT(first != 0);
    NS_ASSERT(last != 0);
    NS_ASSERT(first <= last);

    uint32_t len = 0;
    while (first < last)
    {
        uint32_t n = SequenceLength(first);

        // Fast Check: If ASCII, simple advance
        if (n == 1)
        {
            first++;
            len++;
            continue;
        }

        // Structural Check: Ensure we don't run past 'last' and trails are valid
        bool valid = (first + n <= last);
        if (valid)
        {
            for (uint32_t i = 1; i < n; ++i)
            {
                if (!IsTrail(first[i])) 
                { 
                    valid = false; 
                    break; 
                }
            }
            
            if (valid && n == 3)
            {
                uint8_t lead = Mask8(*first);
                uint8_t second = Mask8(first[1]);
                
                // Surrogate Check: ED A0..BF is invalid
                if (lead == 0xED && second >= 0xA0) valid = false;
                // Overlong Check: E0 80..9F is invalid (must be >= A0)
                if (lead == 0xE0 && second < 0xA0) valid = false;
            }

            if (valid && n == 4)
            {
                uint8_t lead = Mask8(*first);
                uint8_t second = Mask8(first[1]);

                // Range Check: F4 90..FF is invalid (> 0x10FFFF)
                if (lead == 0xF4 && second >= 0x90) valid = false;
                // Overlong Check: F0 80..8F is invalid (must be >= 90)
                if (lead == 0xF0 && second < 0x90) valid = false;
            }
        }

        if (valid)
        {
            first += n;
        }
        else
        {
            // Invalid sequence structure; treat lead byte as 1 invalid char
            first++; 
        }
        len++;
    }
    return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Length(const char* text)
{
    NS_ASSERT(text != 0);
    uint32_t len = 0;

    while (*text != 0)
    {
        // Fast path for ASCII
        if ((uint8_t)(*text) < 0x80)
        {
            text++;
            len++;
            continue;
        }

        uint32_t n = SequenceLength(text);
        
        // Structural Check: Verify trails exist
        bool valid = true;
        for (uint32_t i = 1; i < n; ++i)
        {
            if (!IsTrail(text[i])) 
            { 
                valid = false; 
                break; 
            }
        }

        if (valid && n == 3)
        {
            uint8_t lead = Mask8(*text);
            uint8_t second = Mask8(text[1]);
            
            // Surrogate Check: ED A0..BF is invalid
            if (lead == 0xED && second >= 0xA0) valid = false;
            // Overlong Check: E0 80..9F is invalid (must be >= A0)
            if (lead == 0xE0 && second < 0xA0) valid = false;
        }

        if (valid && n == 4)
        {
            uint8_t lead = Mask8(*text);
            uint8_t second = Mask8(text[1]);

            // Range Check: F4 90..FF is invalid (> 0x10FFFF)
            if (lead == 0xF4 && second >= 0x90) valid = false;
            // Overlong Check: F0 80..8F is invalid (must be >= 90)
            if (lead == 0xF0 && second < 0x90) valid = false;
        }

        if (valid)
        {
            text += n;
        }
        else
        {
            // Invalid structure (truncated or bad trail); treat as 1 invalid char
            text++;
        }
        len++;
    }

    return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsValid(const char* text)
{
    NS_ASSERT(text != 0);
    while (*text != 0)
    {
        uint8_t lead = Mask8(*text);
        
        if (lead < 0x80)
        {
            text++;
        }
        // Strict checks for IsValid
        else if (lead < 0xC2) // Invalid starts (C0, C1, 80..BF)
        {
            return false;
        }
        else if ((lead >> 5) == 0x6) // 2 bytes
        {
            if (!IsTrail(*(text + 1))) return false;
            text += 2;
        }
        else if ((lead >> 4) == 0xe) // 3 bytes
        {
            if (!IsTrail(*(text + 1)) || !IsTrail(*(text + 2))) return false;
            
            // Surrogate check: ED A0..BF is invalid
            if (lead == 0xED && Mask8(*(text + 1)) >= 0xA0) return false;
            // Overlong check: E0 80..9F is invalid
            if (lead == 0xE0 && Mask8(*(text + 1)) < 0xA0) return false;

            text += 3;
        }
        else if ((lead >> 3) == 0x1e) // 4 bytes
        {
            // Strict range check: F5..FF are invalid start bytes
            if (lead > 0xF4) return false;

            if (!IsTrail(*(text + 1)) || !IsTrail(*(text + 2)) || !IsTrail(*(text + 3))) return false;
            
            // Range check: F4 90..FF is invalid
            if (lead == 0xF4 && Mask8(*(text + 1)) >= 0x90) return false;
            // Overlong check: F0 80..8F is invalid
            if (lead == 0xF0 && Mask8(*(text + 1)) < 0x90) return false;

            text += 4;
        }
        else
        {
            return false;
        }
    }

    return true;
}

#define LEAD_SURROGATE_MIN 0xd800u
#define LEAD_SURROGATE_MAX 0xdbffu
#define TRAIL_SURROGATE_MIN 0xdc00u
#define TRAIL_SURROGATE_MAX 0xdfffu
#define LEAD_OFFSET (LEAD_SURROGATE_MIN - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000u - (LEAD_SURROGATE_MIN << 10) - TRAIL_SURROGATE_MIN)

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t UTF8To16(const char* utf8, uint16_t* utf16, uint32_t numUTF16Chars)
{
    NS_ASSERT(utf8 != 0);
    uint32_t totalChars = 0;
    uint32_t writtenChars = 0;

    for (;;)
    {
        uint32_t c = UTF8::Next(utf8);

        if (c <= 0xffff)
        {
            if (totalChars + 1 < numUTF16Chars)
            {
                utf16[totalChars] = (uint16_t)c;
                writtenChars++;
            }
            totalChars++;
        }
        else
        {
            if (totalChars + 2 < numUTF16Chars)
            {
                utf16[totalChars] = (uint16_t)((c >> 10) + LEAD_OFFSET);
                utf16[totalChars + 1] = (uint16_t)((c & 0x3ff) + TRAIL_SURROGATE_MIN);
                writtenChars += 2;
            }
            totalChars += 2;
        }

        if (c == 0)
        {
            if (writtenChars < numUTF16Chars)
            {
                utf16[writtenChars] = 0;
            }
            return totalChars;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t UTF16To8(const uint16_t* utf16, char* utf8, uint32_t numUTF8Chars)
{
    NS_ASSERT(utf16 != 0);
    uint32_t totalChars = 0;
    uint32_t writtenChars = 0;

    for (;;)
    {
        uint32_t c = Mask16(*utf16++);
        bool isLeadSurrogate = c >= LEAD_SURROGATE_MIN && c <= LEAD_SURROGATE_MAX;
        
        if (isLeadSurrogate)
        {
            uint32_t trail = Mask16(*utf16);
            if (trail >= TRAIL_SURROGATE_MIN && trail <= TRAIL_SURROGATE_MAX)
            {
                utf16++;
                c = (c << 10) + trail + SURROGATE_OFFSET;
            }
            else
            {
                c = 0xFFFD;
            }
        }
        else if (c >= TRAIL_SURROGATE_MIN && c <= TRAIL_SURROGATE_MAX)
        {
            c = 0xFFFD;
        }

        uint32_t numOctets = c < 0x80 ? 1 : c < 0x800 ? 2 : c < 0x10000 ? 3 : 4;
        
        if (totalChars + numOctets + 1 <= numUTF8Chars)
        {
            utf8 = UTF8::Append(c, utf8);
            writtenChars += numOctets;
        }

        totalChars += numOctets;

        if (c == 0)
        {
            if (writtenChars < numUTF8Chars)
            {
                *utf8 = 0;
            }
            return totalChars;
        }
    }
}

#undef LEAD_SURROGATE_MIN
#undef LEAD_SURROGATE_MAX
#undef TRAIL_SURROGATE_MIN
#undef TRAIL_SURROGATE_MAX
#undef LEAD_OFFSET
#undef SURROGATE_OFFSET

}
}
