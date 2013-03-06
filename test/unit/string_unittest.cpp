#include <gtest/gtest.h>
#include <webvttxx/string>

using namespace WebVTT;

/**
 * an-nyung ha-se-yo encoded in UTF8
 */
const webvtt_byte UTF8AnNyungHaSeYo[] =
{
  0xEC, 0x95, 0x88, /* U+C548 */
  0xEB, 0x85, 0x95, /* U+B155 */
  0xED, 0x95, 0x98, /* U+D558 */
  0xEC, 0x84, 0xB8, /* U+C138 */
  0xEC, 0x9A, 0x94, /* U+C694 */
  0x00              /* NULL */
};

/**
 * an-nyung ha-se-yo encoded in UTF16
 */
const webvtt_uint16 UTF16AnNyungHaSeYo[] =
{
  0xC548, 0xB155, 0xD558, 0xC138, 0xC694, 0x0000
};

TEST(String,CreateWithText)
{
  webvtt_string str;
  webvtt_create_string_with_text( &str,
    (const webvtt_byte *)"Hello World", -1 );
  ASSERT_EQ( 11, webvtt_string_length( &str ) );
  ASSERT_STREQ( "Hello World", (const char *)webvtt_string_text( &str ) );
  webvtt_release_string( &str );
}

/**
 * string_getline boundary condition
 */
TEST(String,GetLine)
{
  WebVTT::uint pos = 0;
  webvtt_string str;
  webvtt_create_string_with_text( &str, (const webvtt_byte *)"Hello Wor", -1 );
  ASSERT_EQ( 9, webvtt_string_length( &str ) );
  webvtt_string_getline( &str, (const webvtt_byte *)"ld! This is a test! Yup\n",
    &pos, -1,  0, 1, 0 );
  ASSERT_EQ( 32, webvtt_string_length( &str ) );
  ASSERT_STREQ( "Hello World! This is a test! Yup",
    (const char *)webvtt_string_text( &str ) );
  webvtt_release_string( &str );
}

/**
 * string_append boundary condition
 */
TEST(String,Append)
{
  WebVTT::uint pos = 0;
  webvtt_string str;
  webvtt_create_string_with_text( &str, (const webvtt_byte *)"Hello Wor", -1 );
  ASSERT_EQ( 9, webvtt_string_length( &str ) );
  webvtt_string_append( &str, (const webvtt_byte *)"ld! This is a test! Yup",
    -1 );
  ASSERT_EQ( 32, webvtt_string_length( &str ) );
  ASSERT_STREQ( "Hello World! This is a test! Yup",
    (const char *)webvtt_string_text( &str ) );
  webvtt_release_string( &str );
}


/**
 * Test the webvtt_utf8_length routine
 */
TEST(String,UTF8CodepointLength)
{
  ASSERT_EQ( 3, webvtt_utf8_length( UTF8AnNyungHaSeYo ) );
}

/**
 * Test the webvtt_utf8_chcount routine
 */
TEST(String,UTF8CodepointCount)
{
  ASSERT_EQ( 5, webvtt_utf8_chcount( UTF8AnNyungHaSeYo, 0 ) )
    << "Incorrectly counted UTF8 codepoints";
}

/**
 * Test the webvtt_utf8_to_utf16 routine
 */
TEST(String,UTF8ToUTF16)
{
  ASSERT_EQ( UTF16AnNyungHaSeYo[0], webvtt_utf8_to_utf16(
    UTF8AnNyungHaSeYo+0, 0, 0 ) );
  ASSERT_EQ( UTF16AnNyungHaSeYo[1], webvtt_utf8_to_utf16(
    UTF8AnNyungHaSeYo+3, 0, 0 ) );
  ASSERT_EQ( UTF16AnNyungHaSeYo[2], webvtt_utf8_to_utf16(
    UTF8AnNyungHaSeYo+6, 0, 0 ) );
  ASSERT_EQ( UTF16AnNyungHaSeYo[3], webvtt_utf8_to_utf16(
    UTF8AnNyungHaSeYo+9, 0, 0 ) );
  ASSERT_EQ( UTF16AnNyungHaSeYo[4], webvtt_utf8_to_utf16(
    UTF8AnNyungHaSeYo+12, 0, 0 ) );
}

/**
 * Test the webvtt_skip_utf8 routine
 */
TEST(String,UTF8Skip)
{
  const webvtt_byte *b = UTF8AnNyungHaSeYo + 0;
  ASSERT_TRUE( webvtt_skip_utf8( &b, 0, 4 ) ) << "Failed to skip"
    << " characters";
  ASSERT_EQ( UTF8AnNyungHaSeYo + 12, b ) << "Didn't skip to correct"
    << " position";
  ASSERT_FALSE( webvtt_skip_utf8( &b, 0, 2 ) ) << "Skipped beyond"
    << " end of string";
}

/**
 * Test the webvtt_skip_utf8 routine starting from non-header
 * character
 */
TEST(String,UTF8SkipTrailing)
{
  const webvtt_byte *b = UTF8AnNyungHaSeYo + 1;
  ASSERT_TRUE( webvtt_skip_utf8( &b, 0, 1 ) ) << "Failed to skip"
    << " character";
  ASSERT_EQ( b, UTF8AnNyungHaSeYo + 3 ) << "Skipped to incorrect"
    << " position";
}

/**
 * Test that we're correctly detecting if a UTF16 code unit is a
 * lead surrogate
 */
TEST(String,UTF16IsLeadSurrogate)
{
  webvtt_uint16 lead = 0xD843;
  ASSERT_TRUE( String::isSurrogateLead( lead ) );
  ASSERT_FALSE( String::isSurrogateTrail( lead ) );
}

/**
 * Test that we're correctly detecting if a UTF16 code unit is a
 * trail surrogate
 */
TEST(String,UTF16IsTrailSurrogate)
{
  webvtt_uint16 trail = 0xDC96;
  ASSERT_TRUE( String::isSurrogateTrail( trail ) );
  ASSERT_FALSE( String::isSurrogateLead( trail ) );
}

/**
 * Test that we can correctly identify codepoints that require
 * multiple UTF16 units.
 */
TEST(String,UTF32RequiresSurrogate)
{
  webvtt_uint32 surrogate = 0xD843DC96; /* U+20C96 */
  webvtt_uint32 single = 0x00000489; /* U+0489 */
  ASSERT_TRUE( String::requiresSurrogate( surrogate ) );
  ASSERT_FALSE( String::requiresSurrogate( single ) );
}

/**
 * Test that we correctly report that a string is empty.
 */
TEST(String,IsEmpty)
{
  const webvtt_byte ne[] = "Not empty!";
  webvtt_string str = { 0 };
  webvtt_init_string( &str );
  ASSERT_TRUE( webvtt_string_is_empty( &str ) );
  webvtt_release_string( &str );
  webvtt_create_string( 0, &str );
  ASSERT_TRUE( webvtt_string_is_empty( &str ) );
  webvtt_string_append( &str, ne, sizeof( ne ) );
  ASSERT_FALSE( webvtt_string_is_empty( &str ) );
  webvtt_release_string( &str );
}

/**
 * Test that we correctly report that a string is empty using the C++
 * bindings.
 */
TEST(String,IsEmptyCXX)
{
  String str;
  ASSERT_TRUE( str.isEmpty() );
  str = String( "" );
  ASSERT_TRUE( str.isEmpty() );
  str.append( "Not Empty!" ); 
  ASSERT_FALSE( str.isEmpty() );
}
