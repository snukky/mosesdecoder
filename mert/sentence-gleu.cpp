// vim:tabstop=2

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2011- University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "GleuScorer.h"

using namespace MosesTuning;

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "Usage: ./sentence-gleu source+references [config] < candidate > gleu-scores" << std::endl;
    return 1;
  }

  std::vector<std::string> refFiles(argv + 1, argv + 2);

  std::string config(argc >= 3 ? argv[2] : "");
  std::string factors;
  std::string filter;

  GleuScorer scorer(config);
  scorer.setFactors(factors);
  scorer.setFilter(filter);

  scorer.setReferenceFiles(refFiles);

  // load sentences, prepare statistics, score
  ScoreStats sentStats;
  std::vector<float> corpStats;
  std::string hyp;
  size_t sid = 0;

  while (getline(std::cin, hyp)) {
    scorer.CalcGleuStats(hyp, scorer.GetReference(sid), sentStats);

    // sentence-based GLEU uses smoothing
    std::vector<float> tempStats(sentStats.getArray(), sentStats.getArray() + sentStats.size());
    std::cerr << sid << ": " << scorer.calculateGleu(tempStats, true) << std::endl;

    if (corpStats.size() == 0) {
      corpStats = std::vector<float>(sentStats.getArray(), sentStats.getArray() + sentStats.size());
    } else {
      for (size_t i = 0; i < sentStats.size(); ++i) {
        corpStats[i] += sentStats.get(i);
      }
    }

    ++sid;
  }

  std::cout << scorer.calculateGleu(corpStats) << std::endl;

  return 0;
}

