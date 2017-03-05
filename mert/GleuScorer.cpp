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
  , m_numIters(Scan<size_t>(getConfig("iter", "500")))
  , m_debug(Scan<bool>(getConfig("debug", "false")))
{ }

GleuScorer::~GleuScorer() {}

void GleuScorer::setReferenceFiles(const std::vector<std::string>& referenceFiles)
{
  // make sure reference data is clear
  m_references.reset();
  mert::VocabularyFactory::GetVocabulary()->clear();

  // there should be always a single reference file with tab-separated sentences
  UTIL_THROW_IF2(referenceFiles.size() != 1, "Too many reference files. "
      << "A file in the tab-separated format is required.");

  std::ifstream ifs(referenceFiles[0].c_str());
  std::string line;
  int checkNum = -1;
  size_t sid = 0;

  while (getline(ifs, line)) {
    std::vector<std::string> columns;
    boost::split(columns, line, boost::is_any_of("\t"));

    // check if there is at least a source sentence and one reference sentence
    UTIL_THROW_IF2(columns.size() < 2, "Too less columns in reference file '"
        << referenceFiles[0] << "', line " << sid);

    // check if all lines have equal number of sentences
    if (checkNum == -1) {
      checkNum = columns.size();
    }
    UTIL_THROW_IF2((size_t)checkNum != columns.size(),
        "Different number of sentences in reference file '"
        << referenceFiles[0] << "', line " << sid);
    checkNum = columns.size();
    sid += 1;

    // read ngram counts
    std::vector<NgramCounts>* counts = new std::vector<NgramCounts>(columns.size());
    for (size_t i = 0; i < columns.size(); ++i) {
      CountNgrams(preprocessSentence(columns[i]), (*counts)[i], m_order);
    }
    m_references.push_back(counts);
  }
}

void GleuScorer::prepareStats(size_t sid, const std::string& text, ScoreStats& entry)
{
  // TODO: support more than one reference
  const std::vector<NgramCounts>& refs = GetReference(sid);
  CalcGleuStats(text, refs[0], refs[1], entry);
}

statscore_t GleuScorer::calculateScore(const std::vector<ScoreStatsType>& stats) const
{
  UTIL_THROW_IF(stats.size() != m_order * 2 + 1, util::Exception,
      "Incorrect number of statistics");

  float logbleu = 0.0f;
  for (size_t n = 0; n < m_order; ++n) {
    if (stats[2*n] == 0) {
      return 0.0f;
    }
    logbleu += log(stats[2*n]) - log(stats[2*n+1]);
  }
  logbleu /= m_order;

  // reflength divided by test length
  const float brevity = 1.0 - stats[(m_order * 2)] / static_cast<float>(stats[1]);
  if (brevity < 0.0f) {
    logbleu += brevity;
  }
  return exp(logbleu);
}

void GleuScorer::CalcGleuStats(const std::string& hypText, const NgramCounts& srcCounts, const NgramCounts& refCounts, ScoreStats& entry) const
{
  // there are three statistics for each order n: ngram counts of
  // {Hyp and Ref}, ngram counts of {Hyp and Src}, ngram counts of {Hyp}
  std::vector<ScoreStatsType> stats(m_order * 2);
  // reference length as the last statistic
  stats.push_back(refCounts.CountNgrams(1));

  NgramCounts hypCounts;
  CountNgrams(preprocessSentence(hypText), hypCounts, m_order, true);

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

  // calculate nominator of p*_n
  for (size_t n = 0; n < m_order; ++n) {
    stats[n * 2] = std::max(stats[n * 2] - statsDiff[n], 0.0f);
  }

  // accummulate stats
  for (size_t i = 0; i < stats.size(); ++i) {
    stats[i] += (entry.size() > i ? entry.get(i) : 0.0f);
  }

  entry.set(stats);
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

void GleuScorer::CountDiffNgrams(const NgramCounts& countsA, const NgramCounts& countsB,
                                 NgramCounts& resultCounts) const
{
  for (auto const &it : countsA) {
    NgramCounts::Value v;
    if (!countsB.Lookup(it.first, &v)) {
      resultCounts.Add(it.first, it.second);
    }
  }
}

/********************************************************************/

float smoothedSentenceGleu(const std::vector<float>& comps,
                           bool smooth /* =false */,
                           bool debug /* =true */)
{
  // apply smoothing
  std::vector<float> stats(comps);
  if (smooth) {
    for (size_t i = 0; i < stats.size(); ++i) {
      if (stats[i] == 0) {
        stats[1] = 1.0f;
      }
    }
  }

  // get maximum order of ngrams
  size_t maxOrder = (stats.size() - 1) / 2;

  if (debug) {
    std::cerr << "max order: " << maxOrder << std::endl;
    std::cerr << "stats final: ";
    for (auto s : stats) {
      std::cerr << s << " ";
    }
    std::cerr << std::endl;
  }

  float logbleu = 0.0f;
  for (size_t n = 0; n < maxOrder; n++) {
    if (stats[2*n] == 0) {
      return 0.0f;
    }
    logbleu += log(stats[2*n]) - log(stats[2*n+1]);
    if (debug) {
        std::cerr << " logbleu= " << log(stats[2*n]) - log(stats[2*n+1]);
    }
  }
  if (debug) std::cerr << std::endl;

  logbleu /= maxOrder;
  const float reflen = stats[(maxOrder * 2)];
  const float hyplen = stats[1];
  const float brevity = 1.0 - reflen / hyplen;

  if (brevity < 0.0f) {
    logbleu += brevity;
  }
  return exp(logbleu);
}

}
