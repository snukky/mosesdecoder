#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "Ngram.h"
#include "Reference.h"
#include "ScopedVector.h"
#include "ScoreData.h"
#include "StatisticsBasedScorer.h"
#include "Types.h"

namespace MosesTuning
{


/**
 * Gleu scoring
 */
class GleuScorer: public StatisticsBasedScorer
{
public:
  explicit GleuScorer(const std::string& config = "");
  ~GleuScorer();

  virtual void setReferenceFiles(const std::vector<std::string>& referenceFiles);
  virtual void prepareStats(std::size_t sid, const std::string& text, ScoreStats& entry);
  virtual statscore_t calculateScore(const std::vector<ScoreStatsType>& comps) const;

  virtual std::size_t NumberOfScores() const {
    return 2 * m_order + 1;
  }
  virtual float getReferenceLength(const std::vector<ScoreStatsType>& totals) const {
    return totals[m_order*2];
  }

  void CalcGleuStats(const std::string& hypText, const NgramCounts& srcCounts, const NgramCounts& refCounts, ScoreStats& entry) const;

  const std::vector<NgramCounts>& GetReference(size_t sid) const {
    UTIL_THROW_IF2(sid >= m_references.size(), "Sentence id (" << sid << ") not found in reference set.");
    return *(m_references.get())[sid];
  }

private:
  size_t CountNgrams(const std::string& line, NgramCounts& counts, unsigned int n, bool is_testing=false) const;
  void CountDiffNgrams(const NgramCounts& countsA, const NgramCounts& countsB, NgramCounts& resultCounts) const;

  // maximum order of ngrams
  size_t m_order;
  // number of iterations to run
  size_t m_numIters;
  // show debug messages
  bool m_debug;

  // source and multiple reference sentences
  ScopedVector<std::vector<NgramCounts>> m_references;

  // no copying allowed
  GleuScorer(const GleuScorer&);
  GleuScorer& operator=(const GleuScorer&);
};

/** Computes sentence-level BLEU+1 score.
 * This function is used in PRO.
 */
float smoothedSentenceGleu(const std::vector<float>& stats, bool smooth=false, bool debug=true);

}

