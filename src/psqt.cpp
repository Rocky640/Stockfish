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
#define S1(mg, eg) make_score(mg-26, eg-15)
#define S2(mg, eg) make_score(mg+9, eg+30)
#define S3(mg, eg) make_score(mg+14, eg+69)
#define S4(mg, eg) make_score(mg-6, eg-4)
#define S5(mg, eg) make_score(mg-2, eg+18)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
constexpr Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
  { },
  { },
  { // Knight
   { S2(-175, -96), S2(-92,-65), S2(-74,-49), S2(-73,-21) },
   { S2( -77, -67), S2(-41,-54), S2(-27,-18), S2(-15,  8) },
   { S2( -61, -40), S2(-17,-27), S2(  6, -8), S2( 12, 29) },
   { S2( -35, -35), S2(  8, -2), S2( 40, 13), S2( 49, 28) },
   { S2( -34, -45), S2( 13,-16), S2( 44,  9), S2( 51, 39) },
   { S2(  -9, -51), S2( 22,-44), S2( 58,-16), S2( 53, 17) },
   { S2( -67, -69), S2(-27,-50), S2(  4,-51), S2( 37, 12) },
   { S2(-201,-100), S2(-83,-88), S2(-56,-56), S2(-26,-17) }
  },
  { // Bishop
   { S3(-53,-57), S3( -5,-30), S3( -8,-37), S3(-23,-12) },
   { S3(-15,-37), S3(  8,-13), S3( 19,-17), S3(  4,  1) },
   { S3( -7,-16), S3( 21, -1), S3( -5, -2), S3( 17, 10) },
   { S3( -5,-20), S3( 11, -6), S3( 25,  0), S3( 39, 17) },
   { S3(-12,-17), S3( 29, -1), S3( 22,-14), S3( 31, 15) },
   { S3(-16,-30), S3(  6,  6), S3(  1,  4), S3( 11,  6) },
   { S3(-17,-31), S3(-14,-20), S3(  5, -1), S3(  0,  1) },
   { S3(-48,-46), S3(  1,-42), S3(-14,-37), S3(-23,-24) }
  },
  { // Rook
   { S4(-31, -9), S4(-20,-13), S4(-14,-10), S4(-5, -9) },
   { S4(-21,-12), S4(-13, -9), S4( -8, -1), S4( 6, -2) },
   { S4(-25,  6), S4(-11, -8), S4( -1, -2), S4( 3, -6) },
   { S4(-13, -6), S4( -5,  1), S4( -4, -9), S4(-6,  7) },
   { S4(-27, -5), S4(-15,  8), S4( -4,  7), S4( 3, -6) },
   { S4(-22,  6), S4( -2,  1), S4(  6, -7), S4(12, 10) },
   { S4( -2,  4), S4( 12,  5), S4( 16, 20), S4(18, -5) },
   { S4(-17, 18), S4(-19,  0), S4( -1, 19), S4( 9, 13) }
  },
  { // Queen
   { S5( 3,-69), S5(-5,-57), S5(-5,-47), S5( 4,-26) },
   { S5(-3,-55), S5( 5,-31), S5( 8,-22), S5(12, -4) },
   { S5(-3,-39), S5( 6,-18), S5(13, -9), S5( 7,  3) },
   { S5( 4,-23), S5( 5, -3), S5( 9, 13), S5( 8, 24) },
   { S5( 0,-29), S5(14, -6), S5(12,  9), S5( 5, 21) },
   { S5(-4,-38), S5(10,-18), S5( 6,-12), S5( 8,  1) },
   { S5(-5,-50), S5( 6,-27), S5(10,-24), S5( 8, -8) },
   { S5(-2,-75), S5(-2,-52), S5( 1,-43), S5(-2,-36) }
  },
  { // King
   { S(271,  1), S(327, 45), S(270, 85), S(192, 76) },
   { S(278, 53), S(303,100), S(230,133), S(174,135) },
   { S(195, 88), S(258,130), S(169,169), S(120,175) },
   { S(164,103), S(190,156), S(138,172), S( 98,172) },
   { S(154, 96), S(179,166), S(105,199), S( 70,199) },
   { S(123, 92), S(145,172), S( 81,184), S( 31,191) },
   { S( 88, 47), S(120,121), S( 65,116), S( 33,131) },
   { S( 59, 11), S( 89, 59), S( 45, 73), S( -1, 78) }
  }
};

constexpr Score PBonus[RANK_NB][FILE_NB] =
  { // Pawn (asymmetric distribution)
   { },
   { S1(  3,-10), S1(  3, -6), S1( 10, 10), S1( 19,  0), S1( 16, 14), S1( 19,  7), S1(  7, -5), S1( -5,-19) },
   { S1( -9,-10), S1(-15,-10), S1( 11,-10), S1( 15,  4), S1( 32,  4), S1( 22,  3), S1(  5, -6), S1(-22, -4) },
   { S1( -8,  6), S1(-23, -2), S1(  6, -8), S1( 20, -4), S1( 40,-13), S1( 17,-12), S1(  4,-10), S1(-12, -9) },
   { S1( 13,  9), S1(  0,  4), S1(-13,  3), S1(  1,-12), S1( 11,-12), S1( -2, -6), S1(-13, 13), S1(  5,  8) },
   { S1( -5, 28), S1(-12, 20), S1( -7, 21), S1( 22, 28), S1( -8, 30), S1( -5,  7), S1(-15,  6), S1(-18, 13) },
   { S1( -7,  0), S1(  7,-11), S1( -3, 12), S1(-13, 21), S1(  5, 25), S1(-16, 19), S1( 10,  4), S1( -8,  7) }
  };
  #undef S5
  #undef S4
  #undef S3
#undef S2
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
