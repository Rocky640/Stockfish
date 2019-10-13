/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2019 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include "types.h"

Value PieceValue[PHASE_NB][PIECE_NB] = {
  { VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg },
  { VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg }
};

namespace PSQT {

#define S(mg, eg) make_score(mg, eg)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
constexpr Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
  { },
  { },
  { // Knight
   { S(-169+6,-105-2), S(-96+6,-74-2), S(-80+6,-46-2), S(-79+6,-18-2) },
   { S( -79-5, -70-3), S(-39-5,-56-3), S(-24-5,-15-3), S( -9-5,  6-3) },
   { S( -64+3, -38), S(-20+3,-33), S(  4+3, -5), S( 19+3, 27) },
   { S( -28+1, -36-4), S(  5+1,  0-4), S( 41+1, 13-4), S( 47+1, 34-4) },
   { S( -29+2, -41-3), S( 13+2,-20-3), S( 42+2,  4-3), S( 52+2, 35-3) },
   { S( -11-4, -51+6), S( 28-4,-38+6), S( 63-4,-17+6), S( 55-4, 19+6) },
   { S( -67+3, -64-8), S(-21+3,-45-8), S(  6+3,-37-8), S( 37+3, 16-8) },
   { S(-200-3, -98-4), S(-80-3,-89-4), S(-53-3,-53-4), S(-32-3,-16-4) }
  },
  { // Bishop
   { S(-44+4,-63+6), S( -4+4,-30+6), S(-11+4,-35+6), S(-28+4, -8+6) },
   { S(-18-6,-38-4), S(  7-6,-13-4), S( 14-6,-14-4), S(  3-6,  0-4) },
   { S( -8+5,-18-3), S( 24+5,  0-3), S( -3+5, -7-3), S( 15+5, 13-3) },
   { S(  1-7,-26-1), S(  8-7, -3-1), S( 26-7,  1-1), S( 37-7, 16-1) },
   { S( -7-3,-24-3), S( 30-3, -6-3), S( 23-3,-10-3), S( 28-3, 17-3) },
   { S(-17-2,-26-3), S(  4-2,  2-3), S( -1-2,  1-3), S(  8-2, 16-3) },
   { S(-21+1,-34), S(-19+1,-18), S( 10+1, -7), S( -6+1,  9) },
   { S(-48-6,-51+8), S( -3-6,-40+8), S(-12-6,-39+8), S(-25-6,-20+8) }
  },
  { // Rook
   { S(-31, -9), S(-20,-13), S(-14,-10), S(-5, -9) },
   { S(-21,-12), S(-13, -9), S( -8, -1), S( 6, -2) },
   { S(-25,  6), S(-11, -8), S( -1, -2), S( 3, -6) },
   { S(-13, -6), S( -5,  1), S( -4, -9), S(-6,  7) },
   { S(-27, -5), S(-15,  8), S( -4,  7), S( 3, -6) },
   { S(-22,  6), S( -2,  1), S(  6, -7), S(12, 10) },
   { S( -2,  4), S( 12,  5), S( 16, 20), S(18, -5) },
   { S(-17, 18), S(-19,  0), S( -1, 19), S( 9, 13) }
  },
  { // Queen
   { S( 3,-69), S(-5,-57), S(-5,-47), S( 4,-26) },
   { S(-3,-55), S( 5,-31), S( 8,-22), S(12, -4) },
   { S(-3,-39), S( 6,-18), S(13, -9), S( 7,  3) },
   { S( 4,-23), S( 5, -3), S( 9, 13), S( 8, 24) },
   { S( 0,-29), S(14, -6), S(12,  9), S( 5, 21) },
   { S(-4,-38), S(10,-18), S( 6,-12), S( 8,  1) },
   { S(-5,-50), S( 6,-27), S(10,-24), S( 8, -8) },
   { S(-2,-75), S(-2,-52), S( 1,-43), S(-2,-36) }
  },
  { // King
   { S(272,  0), S(325, 41), S(273, 80), S(190, 93) },
   { S(277, 57), S(305, 98), S(241,138), S(183,131) },
   { S(198, 86), S(253,138), S(168,165), S(120,173) },
   { S(169,103), S(191,152), S(136,168), S(108,169) },
   { S(145, 98), S(176,166), S(112,197), S( 69,194) },
   { S(122, 87), S(159,164), S( 85,174), S( 36,189) },
   { S( 87, 40), S(120, 99), S( 64,128), S( 25,141) },
   { S( 64,  5), S( 87, 60), S( 49, 75), S(  0, 75) }
  }
};

constexpr Score PBonus[RANK_NB][FILE_NB] =
  { // Pawn (asymmetric distribution)
   { },
   { S(  3,-10), S(  3, -6), S( 10, 10), S( 19,  0), S( 16, 14), S( 19,  7), S(  7, -5), S( -5,-19) },
   { S( -9,-10), S(-15,-10), S( 11,-10), S( 15,  4), S( 32,  4), S( 22,  3), S(  5, -6), S(-22, -4) },
   { S( -8,  6), S(-23, -2), S(  6, -8), S( 20, -4), S( 40,-13), S( 17,-12), S(  4,-10), S(-12, -9) },
   { S( 13,  9), S(  0,  4), S(-13,  3), S(  1,-12), S( 11,-12), S( -2, -6), S(-13, 13), S(  5,  8) },
   { S( -5, 28), S(-12, 20), S( -7, 21), S( 22, 28), S( -8, 30), S( -5,  7), S(-15,  6), S(-18, 13) },
   { S( -7,  0), S(  7,-11), S( -3, 12), S(-13, 21), S(  5, 25), S(-16, 19), S( 10,  4), S( -8,  7) }
  };

#undef S

Score psq[PIECE_NB][SQUARE_NB];

// init() initializes piece-square tables: the white halves of the tables are
// copied from Bonus[] adding the piece value, then the black halves of the
// tables are initialized by flipping and changing the sign of the white scores.
void init() {

  for (Piece pc = W_PAWN; pc <= W_KING; ++pc)
  {
      PieceValue[MG][~pc] = PieceValue[MG][pc];
      PieceValue[EG][~pc] = PieceValue[EG][pc];

      Score score = make_score(PieceValue[MG][pc], PieceValue[EG][pc]);

      for (Square s = SQ_A1; s <= SQ_H8; ++s)
      {
          File f = map_to_queenside(file_of(s));
          psq[ pc][ s] = score + (type_of(pc) == PAWN ? PBonus[rank_of(s)][file_of(s)]
                                                      : Bonus[pc][rank_of(s)][f]);
          psq[~pc][~s] = -psq[pc][s];
      }
  }
}

} // namespace PSQT
