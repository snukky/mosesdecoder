#include "GleuScorer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <climits>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include "util/exception.hh"
#include "Ngram.h"
#include "Util.h"
#include "ScoreDataIterator.h"
#include "FeatureDataIterator.h"
#include "Vocabulary.h"

namespace MosesTuning
{


GleuScorer::GleuScorer(const std::string& config)
  : StatisticsBasedScorer("GLEU", config)
  , m_order(Scan<size_t>(getConfig("n", "4")))
  , m_debug(Scan<bool>(getConfig("debug", "false")))
{ }

GleuScorer::~GleuScorer() {}

void GleuScorer::setReferenceFiles(const std::vector<std::string>& referenceFiles)
{
  // make sure reference data is clear
  m_references.reset();
  mert::VocabularyFactory::GetVocabulary()->clear();

  // there should be always a single reference file with tab-separated sentences
  UTIL_THROW_IF2(referenceFiles.size() != 1, "Too many reference files. A file in the tab-separated format is required.");

  std::ifstream ifs(referenceFiles[0].c_str());
  std::string line;
  int checkNum = -1;
  size_t sid = 0;

  while (getline(ifs, line)) {
    std::vector<std::string> columns;
    boost::split(columns, line, boost::is_any_of("\t"));

    // check if there is at least a source sentence and one reference sentence
    UTIL_THROW_IF2(columns.size() < 2, "Too less columns in reference file '" << referenceFiles[0] << "', line " << sid);

    // check if all lines have equal number of sentences
    if (checkNum == -1) {
      checkNum = columns.size();
    }
    UTIL_THROW_IF2((size_t)checkNum != columns.size(), "Different number of sentences in reference file '" << referenceFiles[0] << "', line " << sid);
    checkNum = columns.size();
    sid += 1;

    // read ngram counts
    std::vector<NgramCounts>* counts = new std::vector<NgramCounts>(columns.size());
    for (size_t i = 0; i < columns.size(); ++i) {
      CountNgrams(preprocessSentence(columns[i]), (*counts)[i], m_order);
    }
    m_references.push_back(counts);
  }

  if (m_debug) {
    std::cerr << "Number of references: " << NumberOfReferences() << std::endl;
  }
}

void GleuScorer::prepareStats(size_t sid, const std::string& text, ScoreStats& entry)
{
  CalcGleuStats(text, GetReference(sid), entry);
}

statscore_t GleuScorer::calculateScore(const std::vector<ScoreStatsType>& stats) const
{
  UTIL_THROW_IF(stats.size() != NumberOfScores(), util::Exception, "Incorrect number of statistics");
  return calculateGleu(stats, false);
}

void GleuScorer::CalcGleuStats(const std::string& hypText, const std::vector<NgramCounts>& counts, ScoreStats& entry) const
{
  std::vector<ScoreStatsType> allStats(NumberOfScores());

  NgramCounts hypCounts;
  CountNgrams(preprocessSentence(hypText), hypCounts, m_order, true);

  for (size_t r = 0; r < NumberOfReferences(); ++r) {
    std::vector<ScoreStatsType> stats = CalcGleuStatsForSingleRef(hypCounts, counts[0], counts[r+1]);
    for (size_t s = 0; s < stats.size(); ++s) {
      allStats[r * (m_order * 2 + 1) + s] = stats[s];
    }
  }

  entry.set(allStats);
}

std::vector<ScoreStatsType> GleuScorer::CalcGleuStatsForSingleRef(const NgramCounts& hypCounts,
                                                                  const NgramCounts& srcCounts,
                                                                  const NgramCounts& refCounts) const
{
  // initialize container for statistics
  std::vector<ScoreStatsType> stats(m_order * 2 + 1);
  // length of the reference sentence
  stats[m_order * 2] = refCounts.CountNgrams(1);

  NgramCounts diffCounts;
  CountDiffNgrams(srcCounts, refCounts, diffCounts);
  std::vector<ScoreStatsType> statsDiff(m_order);

  //precision on each ngram type
  for (NgramCounts::const_iterator it = hypCounts.begin(); it != hypCounts.end(); ++it) {
    const NgramCounts::Value guess = it->second;
    const size_t n = it->first.size();
    NgramCounts::Value refCorrect = 0;
    NgramCounts::Value difCorrect = 0;

    NgramCounts::Value v = 0;
    if (refCounts.Lookup(it->first, &v)) {
      refCorrect = std::min(v, guess);
    }
    v = 0;
    if (diffCounts.Lookup(it->first, &v)) {
      difCorrect = std::min(v, guess);
    }
    stats[n * 2 - 2] += refCorrect;
    stats[n * 2 - 1] += guess;
    statsDiff[n - 1] += difCorrect;
  }

  // setup the nominator of p*_n
  for (size_t n = 0; n < m_order; ++n) {
    stats[n * 2] = std::max(stats[n * 2] - statsDiff[n], 0.0f);
  }

  return stats;
}

size_t GleuScorer::CountNgrams(const std::string& line, NgramCounts& counts,
                               unsigned int n, bool is_testing) const
{
  assert(n > 0);
  std::vector<int> encoded_tokens;

  // When performing tokenization of a hypothesis translation, we don't have
  // to update the Scorer's word vocabulary. However, the tokenization of
  // reference translations requires modifying the vocabulary, which means
  // this procedure might be slower than the tokenization the hypothesis
  // translation.
  if (is_testing) {
    TokenizeAndEncodeTesting(line, encoded_tokens);
  } else {
    TokenizeAndEncode(line, encoded_tokens);
  }
  const size_t len = encoded_tokens.size();
  std::vector<int> ngram;

  for (size_t k = 1; k <= n; ++k) {
    // ngram order longer than sentence - no point
    if (k > len) {
      continue;
    }
    for (size_t i = 0; i < len - k + 1; ++i) {
      ngram.clear();
      ngram.reserve(len);
      for (size_t j = i; j < i+k && j < len; ++j) {
        ngram.push_back(encoded_tokens[j]);
      }
      counts.Add(ngram);
    }
  }
  return len;
}

void GleuScorer::CountDiffNgrams(const NgramCounts& countsA,
                                 const NgramCounts& countsB,
                                 NgramCounts& resultCounts) const
{
  for (auto const &it : countsA) {
    NgramCounts::Value v;
    if (!countsB.Lookup(it.first, &v)) {
      resultCounts.Add(it.first, it.second);
    }
  }
}

float GleuScorer::calculateGleu(const std::vector<ScoreStatsType>& comps,
                                bool smooth /* =false */) const
{
  const int shift = m_order * 2 + 1;
  float sumbleu = 0.0f;
  for (size_t r = 0; r < NumberOfReferences(); ++r) {
    std::vector<ScoreStatsType> stats(comps.begin() + (r*shift), comps.begin() + ((r+1)*shift));
    sumbleu += calculateGleuForSingleRef(stats, smooth);
  }
  return sumbleu / (float)NumberOfReferences();
}

float GleuScorer::calculateGleuForSingleRef(const std::vector<ScoreStatsType>& comps,
                                            bool smooth /* =false */) const
{
  // apply smoothing
  std::vector<float> stats(comps);
  if (smooth) {
    for (size_t i = 0; i < stats.size(); ++i) {
      if (stats[i] == 0) {
        stats[i] = 1.0f;
      }
    }
  }

  if (m_debug) {
    std::cerr << "n= " << m_order << std::endl;
    std::cerr << "stats= ";
    for (auto s : stats) {
      std::cerr << s << " ";
    }
    std::cerr << std::endl;
  }

  float logbleu = 0.0f;
  for (size_t n = 0; n < m_order; n++) {
    if (stats[2*n] == 0) {
      return 0.0f;
    }
    logbleu += log(stats[2*n]) - log(stats[2*n+1]);
    //if (m_debug) {
        //std::cerr << "logbleu= " << log(stats[2*n]) - log(stats[2*n+1]) << " ";
    //}
  }
  //if (m_debug) std::cerr << std::endl;

  logbleu /= m_order;

  const float brevity = 1.0 - stats[m_order * 2] / stats[1];
  if (brevity < 0.0f) {
    logbleu += brevity;
  }

  return exp(logbleu);
}

}
