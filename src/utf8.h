#ifndef _UTF8_H
#define _UTF8_H


typedef union t_codepoint_tmp
{
  char c[7];
  unsigned char u[7];
} t_codepoint;

/** Creates a UTF-8 representation of a single unicode codepoint.
 *  Will convert UCS2, UCS4 and UTF-32 to UTF-8.
 */
inline void codepointToUTF8(unsigned int cp, t_codepoint * szOut)
{
  szOut->u[0] = szOut->u[1] = szOut->u[2] = szOut->u[3] = szOut->u[4] = szOut->u[5] = szOut->u[6] = 0;

  size_t i = 0;

  if (cp < 0x80)            // ASCII
  {
    szOut->u[i++] = (unsigned char) cp;
  }
  else if (cp < 0x800)      // Many common scripts
  {
    szOut->u[i++] = 0xC0 | ((cp) >> 6          );
    szOut->u[i++] = 0x80 | ( cp          & 0x3F);
  }
  else if (cp < 0x10000)    // Basic Multilingual Plane
  {
    szOut->u[i++] = 0xE0 | ( (cp) >> 12        );
    szOut->u[i++] = 0x80 | (((cp) >>  6) & 0x3F);
    szOut->u[i++] = 0x80 | ( cp          & 0x3F);
  }
  else if (cp < 0x200000)   // All known planes up to Unicode 5.0 (?)
  {
    szOut->u[i++] = 0xF0 | ( (cp) >> 18        );
    szOut->u[i++] = 0x80 | (((cp) >> 12) & 0x3F);
    szOut->u[i++] = 0x80 | (((cp) >> 6)  & 0x3F);
    szOut->u[i++] = 0x80 | (  cp         & 0x3F);
  }
  else if (cp < 0x4000000)  // Hypothetical
  {
    szOut->u[i++] = 0xF8 | ( (cp) >> 24        );
    szOut->u[i++] = 0x80 | (((cp) >> 18) & 0x3F);
    szOut->u[i++] = 0x80 | (((cp) >> 12) & 0x3F);
    szOut->u[i++] = 0x80 | (((cp) >> 6)  & 0x3F);
    szOut->u[i++] = 0x80 | (  cp         & 0x3F);
  }
  else if (cp < 0x80000000) // Hypothetical
  {
    szOut->u[i++] = 0xFC | ( (cp) >> 30        );
    szOut->u[i++] = 0x80 | (((cp) >> 24) & 0x3F);
    szOut->u[i++] = 0x80 | (((cp) >> 18) & 0x3F);
    szOut->u[i++] = 0x80 | (((cp) >> 12) & 0x3F);
    szOut->u[i++] = 0x80 | (((cp) >> 6)  & 0x3F);
    szOut->u[i++] = 0x80 | (  cp         & 0x3F);
  }
  else
  {
    // Error, can only represent codepoints of length 31 bits.
  }
}


/** Extracts one Unicode codepoint from a string in UTF-8 encoding; updates position.
 */
inline unsigned int getOneCodepointFromUTF8(const std::string& str, size_t & position)
{
  if (position >= str.length()) return -1;

  unsigned char data[7];
  data[0] = str[position++];

  if ((data[0] & 0x80) == 0)
  {
    // 1 code unit

    return data[0];
  }

  else if ((data[0] & 0xC0) == 0xC0)
  {
    // 2 code units

    if (position >= str.length()) { position = str.length(); return -1; }

    data[1] = str[position++];

    if ((data[1] & 0xF0) != 0xF0)
    {
      position = str.length();
      return -1;
    }

    return ((data[0] & 0x1F) << 6) | (data[1] & 0x3F);
  }

  else if ((data[0] & 0xE0) == 0xE0)
  {
    // 3 code units

    if (position + 1 >= str.length()) { position = str.length(); return -1; }

    data[1] = str[position++];
    data[2] = str[position++];

    if ((data[1] & 0xF0) != 0xF0 || (data[2] & 0xF0) != 0xF0)
    {
      position = str.length();
      return -1;
    }

    return ((data[0] & 0x0F) << 12) | ((data[1] & 0x3F) << 6) | (data[2] & 0x3F);
  }

  else if ((data[0] & 0xF0) == 0xF0)
  {
    // 4 code units

    if (position + 2 >= str.length()) { position = str.length(); return -1; }

    data[1] = str[position++];
    data[2] = str[position++];
    data[3] = str[position++];

    if ((data[1] & 0xF0) != 0xF0 || (data[2] & 0xF0) != 0xF0 || (data[3] & 0xF0) != 0xF0)
    {
      position = str.length();
      return -1;
    }

    return ((data[0] & 0x07) << 18) | ((data[1] & 0x3F) << 12) | ((data[2] & 0x3F) << 6) | (data[3] & 0x3F);
  }

  else if ((data[0] & 0xF8) == 0xF8)
  {
    // 5 code units

    if (position + 3 >= str.length()) { position = str.length(); return -1; }

    data[1] = str[position++];
    data[2] = str[position++];
    data[3] = str[position++];
    data[4] = str[position++];

    if ((data[1] & 0xF0) != 0xF0 || (data[2] & 0xF0) != 0xF0 || (data[3] & 0xF0) != 0xF0 || (data[4] & 0xF0) != 0xF0)
    {
      position = str.length();
      return -1;
    }

    return ((data[0] & 0x03) << 24) | ((data[1] & 0x3F) << 18) | ((data[2] & 0x3F) << 12) | ((data[3] & 0x3F) << 6) | (data[4] & 0x3F);
  }

  else if ((data[0] & 0xFC) == 0xFC)
  {
    // 6 code units

    if (position + 4 >= str.length()) { position = str.length(); return -1; }

    data[1] = str[position++];
    data[2] = str[position++];
    data[3] = str[position++];
    data[4] = str[position++];
    data[5] = str[position++];

    if ((data[1] & 0xF0) != 0xF0 || (data[2] & 0xF0) != 0xF0 || (data[3] & 0xF0) != 0xF0 || (data[4] & 0xF0) != 0xF0 || (data[5] & 0xF0) != 0xF0)
    {
      position = str.length();
      return -1;
    }

    return ((data[0] & 0x01) << 30) | ((data[1] & 0x3F) << 24) | ((data[2] & 0x3F) << 18) | ((data[3] & 0x3F) << 12) | ((data[4] & 0x3F) << 6) | (data[5] & 0x3F);
  }

  else
  {
    position = str.length();
    return -1;
  }
}


/** This utility function creates a UCS2 "string" (actually a vector of uint16_t's) from
 *  a UTF-8-encoded input string. Non-BMP characters are replaced by 0xFFFD.
 */
inline void makeUCS2MessageFromUTF8(const std::string & str, std::vector<uint16_t> & result)
{
  unsigned int codepoint;
  result.clear();
  result.reserve(str.length() * 2);

  for (size_t position = 0; position < str.length(); )
  {
    codepoint = getOneCodepointFromUTF8(str, position);

    // Break on error
    if (codepoint == (unsigned int)(-1)) break;

    // Check that the codepoint is in the BMP and valid, otherwise insert dummy character.
    if (codepoint < 0xFFFE && (codepoint < 0xD800 || codepoint > 0xDFFF))
    {
      result.push_back(codepoint);
    }
    else
    {
      result.push_back(0xFFFD); // replacement character
    }
  }
}

#endif
