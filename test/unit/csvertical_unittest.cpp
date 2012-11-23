#include "cue_testfixture"

/**
 * Verifies that the parser correctly parses a "vertical" key, followed by U+003A ':',
 * followed by 'rl' (indicating that the text be positioned vertically, and grows towards the left)
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,RL)
{
	loadVtt( "cue-settings/vertical/rl.vtt", 1 );
	ASSERT_TRUE( getCue( 0 ).isVerticalRightToLeft() );
}

/**
 * Verifies that the parser correctly parses a "vertical" key, followed by U+003A ':',
 * followed by 'lr' (indicating that the text be positioned vertically, and grows towards the right)
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,LR)
{
	loadVtt( "cue-settings/vertical/lr.vtt", 1 );
	ASSERT_TRUE( getCue( 0 ).isVerticalLeftToRight() );
}

/**
 * Test if parser fails when an unknown keyword is used for the otherwise correct 'vertical' cue setting.
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,DISABLED_BadKeyword)
{
	loadVtt( "cue-settings/vertical/bad-keyword.vtt", 1 );
	const Error& err = getError( 0 );
	/**
	 * We're expecting a WEBVTT_INVALID_CUESETTING error on the 25th column of the 3rd line
	 */
	ASSERT_EQ( WEBVTT_INVALID_CUESETTING, err.error() );
	ASSERT_EQ( 3, err.line() );
	ASSERT_EQ( 25, err.column() );
}

/**
 * Test if parser fails when an incorrect setting delimiter is used for the otherwise correct cue setting.
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,DISABLED_BadDelimiter)
{
	loadVtt ( "cue-settings/vertical/bad-delimiter.vtt", 1 );
	const Error& err = getError( 0 );

	/**
	 * We're expecting a WEBVTT_INVALID_CUESETTING error on the 25th column of the 3rd line
	 *
	 * This should really be changed to a different error, like WEBVTT_EXPECTED_CUESETTING_DELIMITER, or something.
	 */
	ASSERT_EQ(WEBVTT_INVALID_CUESETTING,err.error());
	ASSERT_EQ(3,err.line());
	ASSERT_EQ(25,err.column());
}

/**
 * Test if parser fails when an unknown value is used for an otherwise correct 'vertical' cue setting.
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,DISABLED_BadValue)
{
	loadVtt( "cue-settings/vertical/bad-value.vtt", 1 );
	const Error& err = getError( 0 );

	/**
	 * We're expecting a WEBVTT_INVALID_CUESETTING error on the 25th column of the 3rd line
	 *
	 * This should really be changed to a different error, like WEBVTT_EXPECTED_CUESETTING_DELIMITER, or something.
	 */
	ASSERT_EQ( WEBVTT_VERTICAL_BAD_VALUE, err.error() );
	ASSERT_EQ( 3, err.line() );
	ASSERT_EQ( 34, err.column() );
}

/**
 * Test if parser fails when the key/value delimiter is preceded by whitespace.
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,DISABLED_BadWhitespaceBeforeDelimiter)
{
	loadVtt( "cue-settings/vertical/bad-whitespace-before-delimiter.vtt", 1 );
	const Error& err = getError( 0 );

	/**
	 * We're expecting a WEBVTT_INVALID_CUESETTING error on the 34th column of the 3rd line
	 *
	 * This should really be changed to a different error, like WEBVTT_EXPECTED_CUESETTING_DELIMITER, or something.
	 */
	ASSERT_EQ( WEBVTT_INVALID_CUESETTING, err.error() );
	ASSERT_EQ( 3, err.line() );
	ASSERT_EQ( 33, err.column() );
}

/**
 * Test if parser fails when the key/value delimiter is followed by whitespace.
 *
 * From http://dev.w3.org/html5/webvtt/#webvtt-vertical-text-cue-setting (09/28/2012):
 *	1. The string "vertical".
 *	2. A U+003A COLON character (:).
 *	3. One of the following strings: "rl", "lr". 
 */
TEST_F(CueTest,DISABLED_BadWhitespaceAfterDelimiter)
{
	loadVtt( "cue-settings/vertical/bad-whitespace-after-delimiter.vtt", 1 );
	const Error& err = getError( 0 );

	/**
	 * We're expecting a WEBVTT_INVALID_CUESETTING error on the 34th column of the 3rd line
	 *
	 * This should really be changed to a different error, like WEBVTT_EXPECTED_CUESETTING_DELIMITER, or something.
	 */
	ASSERT_EQ( WEBVTT_INVALID_CUESETTING, err.error() );
	ASSERT_EQ( 3, err.line() );
	ASSERT_EQ( 34, err.column() );
}
