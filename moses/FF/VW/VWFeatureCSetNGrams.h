#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureCSet.h"
#include "moses/Util.h"

namespace Moses
{

// A source-side feature that creates n-grams of specified size adjacent to a
// confusion word.
// Currently, there is no mechanism to handle <null> confusion word.
class VWFeatureCSetNGrams : public VWFeatureCSet
{
public:
  VWFeatureCSetNGrams(const std::string &line)
    : VWFeatureCSet(line), m_windowSize(DEFAULT_WINDOW_SIZE), m_internal(false) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , const CWordInfo &cWordInfo
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // TODO
    // this feature requires non-empty confusion set
    //classifier.GetConfusionSet().ThrowIfEmpty();

    // nothing to do as no confusion word is found in the input
    if (! cWordInfo.sourcePos.IsSet())
      return;

    for (size_t numWordsAfter = 0; numWordsAfter <= m_windowSize; ++numWordsAfter) {
      size_t numWordsBefore = m_windowSize - numWordsAfter;

      // an index of the last word before the confusion word
      int idxBefore = sourceRange.GetStartPos() + cWordInfo.sourcePos.i;
      // an index of the first word after the confusion word
      size_t idxAfter = sourceRange.GetStartPos() + cWordInfo.sourcePos.j + 1;

      // skip creating an n-gram that exceeds the source phrase if the option
      // 'internal' is set
      if (m_internal && (idxBefore - numWordsBefore < sourceRange.GetStartPos()
            || idxAfter > sourceRange.GetEndPos()))
        continue;

      std::string ngram;
      for (int i = idxBefore - numWordsBefore; i < idxBefore; ++i)
        ngram += (i < 0) ? "^<s>" : "^" + GetWord(input, i);
      for (size_t i = idxAfter; i < idxAfter + numWordsAfter; ++i)
        ngram += (i >= input.GetSize()) ? "^</s>" : "^" + GetWord(input, i);

      outFeatures.push_back(classifier.AddLabelIndependentFeature("csngram^-" +
            SPrint(numWordsBefore) + "+" + SPrint(numWordsAfter) + ngram));
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    if (key == "size") {
      m_windowSize = Scan<size_t>(value);
    } else if (key == "internal") {
      m_internal = Scan<bool>(value);
    } else {
      VWFeatureCSet::SetParameter(key, value);
    }
  }

  const char* GetFFName() const {
    return "VWFeatureCSetNGrams";
  }

private:
  size_t m_windowSize;
  bool m_internal;

  static const int DEFAULT_WINDOW_SIZE = 2;
};

}
