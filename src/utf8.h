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


#endif
