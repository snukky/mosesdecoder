#pragma once

#include <set>
#include <cmath>
#include <string>

#include "moses/Util.h"
#include "moses/StaticData.h"
#include "moses/Phrase.h"

namespace Moses
{

/**
 * Calculation of training loss for VW.
 */
class TrainingLoss
{
public:
  virtual float operator()(const TargetPhrase &candidate, const TargetPhrase &correct, const Phrase &original, bool isCorrect) const = 0;
};

/**
 * Basic 1/0 training loss.
 */
class TrainingLossBasic : public TrainingLoss
{
public:
  virtual float operator()(const TargetPhrase &candidate, const TargetPhrase &correct, const Phrase &original, bool isCorrect) const {
    return isCorrect ? 0.0 : 1.0;
  }
};

/**
 * BLEU2+1 training loss.
 */
class TrainingLossBLEU : public TrainingLoss
{
public:
  virtual float operator()(const TargetPhrase &candidate, const TargetPhrase &correct, const Phrase &original, bool isCorrect) const {
    std::multiset<std::string> refNgrams;
    float precision = 1.0;

    for (size_t size = 1; size <= BLEU_N; size++) {
      for (int pos = 0; pos <= (int)correct.GetSize() - (int)size; pos++) {
        refNgrams.insert(MakeNGram(correct, pos, pos + size));
      }

      int confirmed = 1; // we're BLEU+1
      int total = 1;
      for (int pos = 0; pos <= (int)candidate.GetSize() - (int)size; pos++) {
        total++;
        std::string ngram = MakeNGram(candidate, pos, pos + size);
        std::multiset<std::string>::iterator it;
        if ((it = refNgrams.find(ngram)) != refNgrams.end()) {
          confirmed++;
          refNgrams.erase(it);
        }
      }
      precision *= (float)confirmed / total;
    }

    int c = candidate.GetSize();
    int r = correct.GetSize();

    float brevityPenalty = c < r ? exp((float)(1.0 - r) / c) : 1.0;

    return 1.0 - brevityPenalty * pow(precision, (float)1.0 / BLEU_N);
  }

private:
  std::string MakeNGram(const TargetPhrase &phrase, size_t start, size_t end) const {
    std::vector<std::string> words;
    while (start != end) {
      words.push_back(phrase.GetWord(start).GetString(StaticData::Instance().options()->output.factor_order, false));
      start++;
    }
    return Join(" ", words);
  }

  static const size_t BLEU_N = 2;
};

/*
 * Edit type training loss.
 */
class TrainingLossEdits : public TrainingLoss
{
public:
  TrainingLossEdits() : TrainingLoss(), m_costs(5) {
    // Initialize default costs (losses) for edits of the type (original, correct, candidate).
    m_costs[0] = 0.0f;  // a,a,a
    m_costs[1] = 1.0f;  // a,a,b
    m_costs[2] = 0.0f;  // a,b,b
    m_costs[3] = 4.0f;  // a,b,a
    m_costs[4] = 1.0f;  // a,b,c
  }

  virtual float operator()(const TargetPhrase &candidate, const TargetPhrase &correct, const Phrase &original, bool isCorrect) const {
    float cost = 0.0f;
    // a,a,a + a,a,b
    if (Equals(original, correct))
      cost = isCorrect ? m_costs[0] : m_costs[1];
    // a,b,b
    else if (isCorrect)
      cost = m_costs[2];
    // a,b,a + a,b,c
    else
      cost = Equals(original, candidate) ? m_costs[3] : m_costs[4];
    return cost;
  }

private:
  std::vector<float> m_costs;
  std::vector<float> m_factors;

  bool Equals(const Phrase &a, const Phrase &b) const {
    if (a.GetSize() != b.GetSize())
      return false;
    for (size_t i = 0; i < a.GetSize(); ++i)
      if (a.GetWord(i).GetString(0).compare(b.GetWord(i).GetString(0)) != 0)
        return false;
    return true;
  }
};

}

