/**
 * Copyright (c) 2013 Mozilla Foundation and Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "parser_internal.h"

/**
 * There are probably enough jumps and stack pops here to fill up quite a few
 * caches but it may still
 * be much smaller than a gigantic table-based solution.
 *
 * TODO: Replace all char literals with hex values, just in case compiling on a
 * machine which uses an
 * incompatible character set
 */

#define U_DIGIT case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: \
                case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:
#define U_WHITESPACE case 0x0D: case 0x0A: case 0x20: case 0x09:
#define U_SPACE case 0x20:
#define U_TAB case 0x09:
#define U_CR case 0x0D:
#define U_LF case 0x0A:

#define U_DASH case 0x2D:
#define U_PERIOD case 0x2E:
#define U_GT case 0x3E:
#define U_COLON case 0x3A:
#define U_PERCENT case 0x25:

#define U_0 case 0x30:
#define U_1 case 0x31:
#define U_2 case 0x32:
#define U_3 case 0x33:
#define U_4 case 0x34:
#define U_5 case 0x35:
#define U_6 case 0x36:
#define U_7 case 0x37:
#define U_8 case 0x38:
#define U_9 case 0x39:

#define U_a case 0x61:
#define U_b case 0x62:
#define U_c case 0x63:
#define U_d case 0x64:
#define U_e case 0x65:
#define U_f case 0x66:
#define U_g case 0x67:
#define U_h case 0x68:
#define U_i case 0x69:
#define U_j case 0x6A:
#define U_k case 0x6B:
#define U_l case 0x6C:
#define U_m case 0x6D:
#define U_n case 0x6E:
#define U_o case 0x6F:
#define U_p case 0x70:
#define U_q case 0x71:
#define U_r case 0x72:
#define U_s case 0x73:
#define U_t case 0x74:
#define U_u case 0x75:
#define U_v case 0x76:
#define U_w case 0x77:
#define U_x case 0x78:
#define U_y case 0x79:
#define U_z case 0x7A:

#define U_A case 0x41:
#define U_B case 0x42:
#define U_C case 0x43:
#define U_D case 0x44:
#define U_E case 0x45:
#define U_F case 0x46:
#define U_G case 0x47:
#define U_H case 0x48:
#define U_I case 0x49:
#define U_J case 0x4A:
#define U_K case 0x4B:
#define U_L case 0x4C:
#define U_M case 0x4D:
#define U_N case 0x4E:
#define U_O case 0x4F:
#define U_P case 0x50:
#define U_Q case 0x51:
#define U_R case 0x52:
#define U_S case 0x53:
#define U_T case 0x54:
#define U_U case 0x55:
#define U_V case 0x56:
#define U_W case 0x57:
#define U_X case 0x58:
#define U_Y case 0x59:
#define U_Z case 0x5A:

#define U_BOM0 case 0xEF:
#define U_BOM1 case 0xBB:
#define U_BOM2 case 0xBF:

#define DEFAULT default:

/**
 * Just for semantic clarity
 */
#define OR
#define AND

#define IF_OVERFLOW(X) \
  if( self->token_pos >= (sizeof(self->token) - 1 ) ) \
  { \
    RETURN(X) \
  }

#define BEGIN_STATE(state) case state: { switch(c) {
#define END_STATE DEFAULT BACKUP return BADTOKEN; } } break;
#define END_STATE_EX } } break;
#define SET_STATE(X) self->tstate = X; break;
#define RETURN(X) self->tstate = L_START; return X;
#define SET_NEWLINE self->line++; self->column = 1; RETURN(NEWLINE)
#define CONTINUE continue;
#define BREAK break;

#define BACKUP (*pos)--; \
               --self->column; \
               self->token[--self->token_pos] = 0; \
               self->tstate = L_START;
#define RESET self->column = 1; \
              self->bytes = self->token_pos = 0; \
              self->tstate = L_START;

#define CHECK_BROKEN_TIMESTAMP \
if(self->token_pos == sizeof(self->token) - 1 ) \
{ \
  ERROR(WEBVTT_MALFORMED_TIMESTAMP); \
  return BADTOKEN; \
}

WEBVTT_INTERN webvtt_status
webvtt_lex_word( webvtt_parser self, webvtt_string *str, const char *buffer,
                 webvtt_uint *ppos, webvtt_uint length, webvtt_bool finish )
{
  webvtt_status status = WEBVTT_SUCCESS;
  webvtt_uint pos = *ppos;
  if( !str ) {
    return WEBVTT_INVALID_PARAM;
  }

  webvtt_init_string( str );

# define ASCII_DASH (0x2D)
# define ASCII_GT (0x3E)
  while( pos < length ) {
    webvtt_uint last_bytes = self->bytes;
    webvtt_uint last_line = self->line;
    webvtt_uint last_column = self->column;
    webvtt_uint last_pos = pos;

    webvtt_token token = webvtt_lex(self, buffer, &pos, length, finish );

    if( token == BADTOKEN ) {
      if( WEBVTT_FAILED( status = webvtt_string_putc( str, buffer[pos] ) ) ) {
        webvtt_release_string( str );
        goto _finished;
      }
      ++pos;
    } else {
      pos = last_pos;
      self->bytes = last_bytes;
      self->line = last_line;
      self->column = last_column;
      goto _finished;
    }
  }

_finished:
  *ppos = pos;
  return status;
}

/**
 * webvtt_lex_newline
 *
 * Get newline sequence in re-entrant fashion. self->tstate must be
 * L_START or L_NEWLINE0 for this function to behave correctly.
 */
WEBVTT_INTERN webvtt_token
webvtt_lex_newline( webvtt_parser self, const
  char *buffer, webvtt_uint *pos, webvtt_uint length,
  webvtt_bool finish )
{
  webvtt_uint p = *pos;

  /* Ensure that we've got a valid token-state for this use-case. */
  DIE_IF( self->tstate != L_START && self->tstate != L_NEWLINE0 );

  while( p < length ) {
    unsigned char c = (unsigned char)buffer[ p++ ];
    self->token[ self->token_pos++ ] = c;
    self->token[ self->token_pos ] = 0;
    self->bytes++;

    switch( self->tstate ) {
      case L_START:
        if( c == '\n' ) {
          *pos = p;
          return NEWLINE;
        } else if( c == '\r' ) {
          self->tstate = L_NEWLINE0;
        } else {
          goto backup;
        }
        break;
      case L_NEWLINE0:
        if( c == '\n' ) {
          *pos = p;
          self->tstate = L_START;
          return NEWLINE;
        } else {
          goto backup;
        }
        break;

      default:
        /**
         * This should never happen if the function is called correctly
         * (EG immediately following a successful call to webvtt_string_getline)
         */
       goto backup;
    }
  }

  *pos = p;
  if( finish && ( p >= length ) ) {
    /* If pos >= length, it's and 'finish' is set, it's an automatic EOL */
    self->tstate = L_START;
    return NEWLINE;
  }

  if( self->tstate == L_NEWLINE0 ) {
    return UNFINISHED;
  } else {
    /* This branch should never occur, if the function is used properly. */
    *pos = --p;
    return BADTOKEN;
  }
backup:
  self->token[ --self->token_pos ] = 0;
  --self->bytes;
  *pos = --p;
  if( self->tstate == L_NEWLINE0 ) {
    self->tstate = L_START;
    return NEWLINE;
  }
  return BADTOKEN;
}

WEBVTT_INTERN webvtt_token
webvtt_lex( webvtt_parser self, const char *buffer, webvtt_uint *pos,
            webvtt_uint length, webvtt_bool finish )
{
  while( *pos < length ) {
    unsigned char c = (unsigned char)buffer[(*pos)++];
    self->token[ self->token_pos++ ] = c;
    self->token[ self->token_pos ] = 0;
    self->column++;
    self->bytes++;
    switch( self->tstate ) {
        BEGIN_STATE(L_START)
          U_W  { SET_STATE(L_WEBVTT0) }
          U_BOM0 { SET_STATE(L_BOM0) }
          U_LF { SET_NEWLINE }
          U_CR { SET_STATE(L_NEWLINE0) }
          U_SPACE OR U_TAB { SET_STATE(L_WHITESPACE) }
        END_STATE

        BEGIN_STATE(L_BOM0)
          U_BOM1 { SET_STATE(L_BOM1) }
        END_STATE

        BEGIN_STATE(L_BOM1)
          U_BOM2 {
          if( self->bytes == 3 ) {
            RESET
            BREAK
          }
          RETURN(BOM)
        }
        END_STATE

        BEGIN_STATE(L_WEBVTT0)
          U_E { SET_STATE(L_WEBVTT1) }
        END_STATE

        BEGIN_STATE(L_WEBVTT1)
          U_B { SET_STATE(L_WEBVTT2) }
        END_STATE

        BEGIN_STATE(L_WEBVTT2)
          U_V { SET_STATE(L_WEBVTT3) }
        END_STATE

        BEGIN_STATE(L_WEBVTT3)
          U_T { SET_STATE(L_WEBVTT4) }
        END_STATE

        BEGIN_STATE(L_WEBVTT4)
          U_T { RETURN(WEBVTT) }
        END_STATE

        BEGIN_STATE(L_NEWLINE0)
          U_LF { SET_NEWLINE }
        DEFAULT { BACKUP AND SET_NEWLINE }
        END_STATE_EX

        BEGIN_STATE(L_WHITESPACE)
          U_SPACE OR U_TAB { IF_OVERFLOW(WHITESPACE) SET_STATE(L_WHITESPACE) }
        DEFAULT { BACKUP RETURN(WHITESPACE) }
        END_STATE_EX
    }
  }

  /**
   * If we got here, we've reached the end of the buffer.
   * We therefore can attempt to finish up
   */
  if( finish && self->token_pos ) {
    switch( self->tstate ) {
      case L_WHITESPACE:
        RETURN(WHITESPACE)
      default:
        RESET
        return BADTOKEN;
    }
  }
  return *pos == length || self->token_pos ? UNFINISHED : BADTOKEN;
}
/**
 * token states
L_START    + 'W' = L_WEBVTT0
L_START    + CR  = L_NEWLINE0
L_START    + LF  = *NEWLINE
L_START    + SP  = L_WHITESPACE
L_START    + TB  = L_WHITESPACE
L_WEBVTT0    + 'E' = L_WEBVTT1
L_WEBVTT1    + 'B' = L_WEBVTT2
L_WEBVTT2    + 'V' = L_WEBVTT3
L_WEBVTT3    + 'T' = L_WEBVTT4
L_WEBVTT4    + 'T' = *WEBVTT
L_NEWLINE0   + LF  = *NEWLINE
L_WHITESPACE + TB  = L_WHITESPACE
L_WHITESPACE + SP  = L_WHITESPACE
 */
