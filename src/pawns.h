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

#ifndef PAWNS_H_INCLUDED
#define PAWNS_H_INCLUDED

#include "misc.h"
#include "position.h"
#include "types.h"

namespace Pawns {

/// Pawns::Entry contains various information about a pawn structure. A lookup
/// to the pawn hash table (performed by calling the probe function) returns a
/// pointer to an Entry object.

struct Entry {

  Score pawn_score(Color c) const { return scores[c]; }
  Bitboard pawn_attacks(Color c) const { return pawnAttacks[c]; }
  Bitboard passed_pawns(Color c) const { return passedPawns[c]; }
  Bitboard pawn_attacks_span(Color c) const { return pawnAttacksSpan[c]; }
  int passed_count() const { return popcount(passedPawns[WHITE] | passedPawns[BLACK]); }

  template<Color Us>
  Score king_safety(const Position& pos, Bitboard attacks) {
    if ((kingSquares[Us] != pos.square<KING>(Us)) || ((castlingRights[Us] & pos.castling_rights(Us)) != pos.castling_rights(Us)))
        do_king_safety<Us>(pos);

    auto compare = [](Score a, Score b) { return mg_value(a) < mg_value(b); };

    Score score = shelter[Us][0];

    if (pos.castling_rights(Us))
    {
    // If we can castle, use the shelter score of the castled king if it is better
    if (pos.castling_safe(Us & KING_SIDE, attacks))
        score = std::max(score, shelter[Us][1], compare);

    if (pos.castling_safe(Us & QUEEN_SIDE, attacks))
        score = std::max(score, shelter[Us][2], compare);
    }

    return score + shelter[Us][3];
  }

  template<Color Us>
  void do_king_safety(const Position& pos);

  template<Color Us>
  Score evaluate_shelter(const Position& pos, Square ksq);

  Key key;
  Score scores[COLOR_NB];
  Bitboard passedPawns[COLOR_NB];
  Bitboard pawnAttacks[COLOR_NB];
  Bitboard pawnAttacksSpan[COLOR_NB];
  Square kingSquares[COLOR_NB];
  int castlingRights[COLOR_NB];
  Score shelter[COLOR_NB][4];
};

typedef HashTable<Entry, 131072> Table;

Entry* probe(const Position& pos);

} // namespace Pawns

#endif // #ifndef PAWNS_H_INCLUDED
