/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2014 Marco Costalba, Joona Kiiski, Tord Romstad

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
#include <cassert>
#include <cstdlib>
#include <sstream>

#include "misc.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"
#include "syzygy/tbprobe.h"

using std::string;

UCI::OptionsMap Options; // Global object

namespace UCI {

/// 'On change' actions, triggered by an option's value change
void on_clear_hash(const Option&) { TT.clear(); }
void on_hash_size(const Option& o) { TT.resize(o); }
void on_logger(const Option& o) { start_logger(o); }
void on_threads(const Option&) { Threads.read_uci_options(); }
void on_tb_path(const Option& o) { Tablebases::init(o); }


/// Our case insensitive less() function as required by UCI protocol
bool ci_less(char c1, char c2) { return tolower(c1) < tolower(c2); }

bool CaseInsensitiveLess::operator() (const string& s1, const string& s2) const {
  return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), ci_less);
}


/// init() initializes the UCI options to their hard-coded default values

void init(OptionsMap& o) {

  o["Write Debug Log"]       << Option(false, on_logger);
  o["Contempt"]              << Option(0, -100, 100);
  o["Min Split Depth"]       << Option(0, 0, 12, on_threads);
  o["Threads"]               << Option(1, 1, MAX_THREADS, on_threads);
  o["Hash"]                  << Option(16, 1, 1024 * 1024, on_hash_size);
  o["Clear Hash"]            << Option(on_clear_hash);
  o["Ponder"]                << Option(true);
  o["MultiPV"]               << Option(1, 1, 500);
  o["Skill Level"]           << Option(20, 0, 20);
  o["Move Overhead"]         << Option(30, 0, 5000);
  o["Minimum Thinking Time"] << Option(20, 0, 5000);
  o["Slow Mover"]            << Option(80, 10, 1000);
  o["UCI_Chess960"]          << Option(false);
  o["SyzygyPath"]            << Option("<empty>", on_tb_path);
  o["SyzygyProbeDepth"]      << Option(1, 1, 100);
  o["Syzygy50MoveRule"]      << Option(true);
  o["SyzygyProbeLimit"]      << Option(6, 0, 6);

  //SPSA 

  //formula was numattackers * weightsum
  //weight was P=0, N=6, B=2, R=5, Q=5
  //A5pn was 2*(P+N)=2*(0+6)=12

  o["A1p"]    << Option(7, 0, 64); //was 0
  o["A4n"]    << Option(8, 0, 64); //was 6
  o["A16b"]   << Option(12, 0, 64);//was 2
  o["A64r"]   << Option(9, 0, 64); //was 5
  o["A256q"]  << Option(2, 0, 64); //was 5

  o["A2pp"]   << Option(3, 0, 64); //was 0
  o["A5pn"]   << Option(5, 0, 64); //was 12
  o["A17pb"]  << Option(5, 0, 64); //was 4
  o["A65pr"]  << Option(5, 0, 64); //was 10
  o["A257pq"] << Option(18, 0, 64);//was 10
 
  o["A8nn"]   << Option(29, 0, 64);//was 24
  o["A20nb"]  << Option(14, 0, 64);//was 16
  o["A68nr"]  << Option(26, 0, 64);//was 22
  o["A260nq"] << Option(16, 0, 64);//was 22

  o["A32bb"]  << Option(5, 0, 64); //was 8
  o["A80br"]  << Option(26, 0, 64);//was 14 *** plus 12 !
  o["A272bq"] << Option(9, 0, 64); //was 14

  o["A128rr"] << Option(37, 0, 64);//was 20 *** plus 17 !!
  o["A320rq"] << Option(21, 0, 64);//was 20
  o["A512qq"] << Option(19, 0, 64);//was 20

  o["A3ppp"]  << Option(3, 0, 64); //was 0
  o["A6ppn"]  << Option(14, 0, 64);//was 18
  o["A18ppb"] << Option(11, 0, 64);//was 6
  o["A66ppr"] << Option(17, 0, 64);//was 15
  o["A258ppq"]<< Option(10, 0, 64);//was 15

  o["A9pnn"]  << Option(36, 0, 64);//was 36
  o["A21pnb"] << Option(33, 0, 64);//was 24 *** plus 9 !
  o["A69pnr"] << Option(37, 0, 64);//was 33
  o["A261pnq"]<< Option(39, 0, 64);//was 33

  o["A33pbb"] << Option(17, 0, 64);//was 12
  o["A81pbr"] << Option(29, 0, 64);//was 21
  o["A273pbq"]<< Option(24, 0, 64);//was 21
  o["A129prr"]<< Option(28, 0, 64);//was 30
  o["A321prq"]<< Option(25, 0, 64);//was 30
  o["A513pqq"]<< Option(30, 0, 64);//was 30

  o["A12nnn"] << Option(44, 0, 64);//was 54
  o["A24nnb"] << Option(44, 0, 64);//was 42
  o["A72nnr"] << Option(58, 0, 64);//was 51
  o["A264nnq"]<< Option(51, 0, 64);//was 51
  o["A36nbb"] << Option(23, 0, 64);//was 30
  o["A84nbr"] << Option(38, 0, 64);//was 39
  o["A276nbq"]<< Option(28, 0, 64);//was 39 *** minus 11
  o["A132nrr"]<< Option(45, 0, 64);//was 48
  o["A324nrq"]<< Option(48, 0, 64);//was 48
  o["A516nqq"]<< Option(41, 0, 64);//was 48

  o["A48bbb"] << Option(24, 0, 64);//was 18
  o["A96bbr"] << Option(29, 0, 64);//was 27
  o["A288bbq"]<< Option(21, 0, 64);//was 27
  o["A144brr"]<< Option(33, 0, 64);//was 36
  o["A336brq"]<< Option(41, 0, 64);//was 36
  o["A528bqq"]<< Option(26, 0, 64);//was 36

  o["A192rrr"]<< Option(51, 0, 64);//was 45
  o["A384rrq"]<< Option(30, 0, 64);//was 45 *** minus 15
  o["A576rqq"]<< Option(47, 0, 64);//was 45

  o["A738qqq"]<< Option(45, 0, 64);//was 45

}


/// operator<<() is used to print all the options default values in chronological
/// insertion order (the idx field) and in the format defined by the UCI protocol.

std::ostream& operator<<(std::ostream& os, const OptionsMap& om) {

  for (size_t idx = 0; idx < om.size(); ++idx)
      for (OptionsMap::const_iterator it = om.begin(); it != om.end(); ++it)
          if (it->second.idx == idx)
          {
              const Option& o = it->second;
              os << "\noption name " << it->first << " type " << o.type;

              if (o.type != "button")
                  os << " default " << o.defaultValue;

              if (o.type == "spin")
                  os << " min " << o.min << " max " << o.max;

              break;
          }
  return os;
}


/// Option class constructors and conversion operators

Option::Option(const char* v, OnChange f) : type("string"), min(0), max(0), on_change(f)
{ defaultValue = currentValue = v; }

Option::Option(bool v, OnChange f) : type("check"), min(0), max(0), on_change(f)
{ defaultValue = currentValue = (v ? "true" : "false"); }

Option::Option(OnChange f) : type("button"), min(0), max(0), on_change(f)
{}

Option::Option(int v, int minv, int maxv, OnChange f) : type("spin"), min(minv), max(maxv), on_change(f)
{ std::ostringstream ss; ss << v; defaultValue = currentValue = ss.str(); }


Option::operator int() const {
  assert(type == "check" || type == "spin");
  return (type == "spin" ? atoi(currentValue.c_str()) : currentValue == "true");
}

Option::operator std::string() const {
  assert(type == "string");
  return currentValue;
}


/// operator<<() inits options and assigns idx in the correct printing order

void Option::operator<<(const Option& o) {

  static size_t insert_order = 0;

  *this = o;
  idx = insert_order++;
}


/// operator=() updates currentValue and triggers on_change() action. It's up to
/// the GUI to check for option's limits, but we could receive the new value from
/// the user by console window, so let's check the bounds anyway.

Option& Option::operator=(const string& v) {

  assert(!type.empty());

  if (   (type != "button" && v.empty())
      || (type == "check" && v != "true" && v != "false")
      || (type == "spin" && (atoi(v.c_str()) < min || atoi(v.c_str()) > max)))
      return *this;

  if (type != "button")
      currentValue = v;

  if (on_change)
      on_change(*this);

  return *this;
}

} // namespace UCI
