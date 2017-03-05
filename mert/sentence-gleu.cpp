#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "GleuScorer.h"
#include "util/exception.hh"

using namespace std;
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
  ScoreStats scoreStats;
  std::string hyp;
  size_t sid = 0;

  while (getline(std::cin, hyp)) {
    const std::vector<NgramCounts>& refs = scorer.GetReference(sid);
    scorer.CalcGleuStats(hyp, refs[0], refs[1], scoreStats);
    ++sid;
  }

  std::vector<float> stats(scoreStats.getArray(), scoreStats.getArray() + scoreStats.size());
  std::cout << smoothedSentenceGleu(stats) << std::endl;

  return 0;
}

