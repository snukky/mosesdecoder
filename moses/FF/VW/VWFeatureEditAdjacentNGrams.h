#pragma once

#include <string>
#include "VWFeatureEdit.h"

namespace Moses
{

class VWFeatureEditAdjacentNGrams : public VWFeatureEdit
{
public:
  VWFeatureEditAdjacentNGrams(const std::string &line)
    : VWFeatureEdit(line), m_windowSize(DEFAULT_WINDOW_SIZE), m_onlyInclusive(false) {
    ReadParameters();

    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , const TargetPhrase &targetPhrase
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // TODO
    // this feature requires non-empty confusion set
    //classifier.GetConfusionSet().ThrowIfEmpty();

    size_t leftContextLastIdx = sourceRange.GetStartPos() - 1;
    size_t rightContextFirstIdx = sourceRange.GetEndPos() + 1;

    for (size_t i = 0; i < targetPhrase.GetSize(); i++) {
      if (classifier.GetConfusionSet().Has(GetTargetWord(targetPhrase, i))) {
        for (size_t j = 0; j < m_windowSize; j++) {
          int startIdx = i - m_windowSize+1 + j;
          size_t endIdx = startIdx + m_windowSize-1;

          // skip creating an n-gram that exceeds the source phrase if the
          // option 'only-inclusive' is set
          if (m_onlyInclusive && (startIdx < 0 || endIdx >= targetPhrase.GetSize()))
            continue;

          std::string ngram;
          for (size_t k = startIdx; k <= endIdx; k++) {
            if (k < 0) {
              ngram += (leftContextLastIdx + k > 0)
                ? ("^" + GetSourceWord(input, leftContextLastIdx + k))
                : "^<s>";
            } else if (k >= targetPhrase.GetSize()) {
              // a naive assumption that the first word from the right source
              // context should follow the last word of the target phrase
              size_t idx = rightContextFirstIdx + (k - targetPhrase.GetSize());
              ngram += (rightContextFirstIdx + idx)
                ? ("^" + GetSourceWord(input, idx))
                : "^</s>";
            } else {
              ngram += "^" + GetTargetWord(targetPhrase, k);
            }
          }
          outFeatures.push_back(classifier.AddLabelDependentFeature("engram" + ngram));
        }
      }
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    if (key == "size") {
      m_windowSize = Scan<size_t>(value);
    } else if (key == "only-inclusive") {
      m_onlyInclusive = Scan<bool>(value);
    } else {
      VWFeatureEdit::SetParameter(key, value);
    }
  }

  const char* GetFFName() const {
    return "VWFeatureEditAdjacentNGrams";
  }

private:
  size_t m_windowSize;
  bool m_onlyInclusive;

  static const int DEFAULT_WINDOW_SIZE = 2;
};

}
