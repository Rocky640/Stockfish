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
#define SR(mg, eg, rmg, reg) make_score(mg+rmg, eg+reg)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
constexpr Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
  { },
  { },
  { // Knight
   { SR(-169,-105,2,7), SR(-96,-74,2,7), SR(-80,-46,2,7), SR(-79,-18,2,7) },
   { SR( -79, -70,-9,-1), SR(-39,-56,-9,-1), SR(-24,-15,-9,-1), SR( -9,  6,-9,-1) },
   { SR( -64, -38,-1,-6), SR(-20,-33,-1,-6), SR(  4, -5,-1,-6), SR( 19, 27,-1,-6) },
   { SR( -28, -36,1,-1), SR(  5,  0,1,-1), SR( 41, 13,1,-1), SR( 47, 34,1,-1) },
   { SR( -29, -41,3,-3), SR( 13,-20,3,-3), SR( 42,  4,3,-3), SR( 52, 35,3,-3) },
   { SR( -11, -51,3,-6), SR( 28,-38,3,-6), SR( 63,-17,3,-6), SR( 55, 19,3,-6) },
   { SR( -67, -64,-2,2), SR(-21,-45,-2,2), SR(  6,-37,-2,2), SR( 37, 16,-2,2) },
   { SR(-200, -98,-7,11), SR(-80,-89,-7,11), SR(-53,-53,-7,11), SR(-32,-16,-7,11) }
  },
  { // Bishop
   { SR(-44,-63,2,6), SR( -4,-30,2,6), SR(-11,-35,2,6), SR(-28, -8,2,6) },
   { SR(-18,-38,-6,-1), SR(  7,-13,-6,-1), SR( 14,-14,-6,-1), SR(  3,  0,-6,-1) },
   { SR( -8,-18,-1,3), SR( 24,  0,-1,3), SR( -3, -7,-1,3), SR( 15, 13,-1,3) },
   { SR(  1,-26,0,6), SR(  8, -3,0,6), SR( 26,  1,0,6), SR( 37, 16,0,6) },
   { SR( -7,-24,-1,-6), SR( 30, -6,-1,-6), SR( 23,-10,-1,-6), SR( 28, 17,-1,-6) },
   { SR(-17,-26,4,0), SR(  4,  2,4,0), SR( -1,  1,4,0), SR(  8, 16,4,0) },
   { SR(-21,-34,-7,0), SR(-19,-18,-7,0), SR( 10, -7,-7,0), SR( -6,  9,-7,0) },
   { SR(-48,-51,0,-2), SR( -3,-40,0,-2), SR(-12,-39,0,-2), SR(-25,-20,0,-2) }
  },
  { // Rook
   { S(-24, -2), S(-13,-6), S(-7, -3), S( 2,-2) },
   { S(-18,-10), S(-10,-7), S(-5,  1), S( 9, 0) },
   { S(-21, 10), S( -7,-4), S( 3,  2), S(-1,-2) },
   { S(-13, -5), S( -5, 2), S(-4, -8), S(-6, 8) },
   { S(-24, -8), S(-12, 5), S(-1,  4), S( 6,-9) },
   { S(-24,  3), S( -4,-2), S( 4,-10), S(10, 7) },
   { S( -8,  1), S(  6, 2), S(10, 17), S(12,-8) },
   { S(-22, 12), S(-24,-6), S(-6, 13), S( 4, 7) }
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
   { S(  0,-10), S( -5,-3), S( 10, 7), S( 13,-1), S( 21,  7), S( 17,  6), S(  6, 1), S( -3,-20) },
   { S(-11, -6), S(-10,-6), S( 15,-1), S( 22,-1), S( 26, -1), S( 28,  2), S(  4,-2), S(-24, -5) },
   { S( -9,  4), S(-18,-5), S(  8,-4), S( 22,-5), S( 33, -6), S( 25,-13), S( -4,-3), S(-16, -7) },
   { S(  6, 18), S( -3, 2), S(-10, 2), S(  1,-9), S( 12,-13), S(  6, -8), S(-12,11), S(  1,  9) },
   { S( -6, 25), S( -8,17), S(  5,19), S( 11,29), S(-14, 29), S(  0,  8), S(-12, 4), S(-14, 12) },
   { S(-10, -1), S(  6,-6), S( -5,18), S(-11,22), S( -2, 22), S(-14, 17), S( 12, 2), S( -1,  9) }
  };

#undef S
#undef SR

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
          File f = std::min(file_of(s), ~file_of(s));
          psq[ pc][ s] = score + (type_of(pc) == PAWN ? PBonus[rank_of(s)][file_of(s)]
                                                      : Bonus[pc][rank_of(s)][f]);
          psq[~pc][~s] = -psq[pc][s];
      }
  }
}

} // namespace PSQT
