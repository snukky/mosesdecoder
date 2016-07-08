#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureSource.h"
#include "moses/Util.h"

namespace Moses
{

// A source-side feature that creates n-grams of specified size adjacent to a
// confusion word.
// Currently, there is no mechanism to handle <null> confusion word.
class VWFeatureSourceAdjacentNGrams : public VWFeatureSource
{
public:
  VWFeatureSourceAdjacentNGrams(const std::string &line)
    : VWFeatureSource(line), m_windowSize(DEFAULT_WINDOW_SIZE), m_onlyInclusive(false) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // TODO
    // this feature requires non-empty confusion set
    //classifier.GetConfusionSet().ThrowIfEmpty();

    size_t begin = sourceRange.GetStartPos();
    size_t size  = sourceRange.GetEndPos() + 1 - begin;

    for (size_t i = 0; i < size; i++) {
      if (classifier.GetConfusionSet().Has(GetWord(input, begin + i))) {
        for (size_t j = 0; j < m_windowSize; j++) {
          int startIdx = i - m_windowSize+1 + j;
          size_t endIdx = startIdx + m_windowSize-1;

          // skip creating an n-gram that exceeds the source phrase if the
          // option 'only-inclusive' is set
          if (m_onlyInclusive && (startIdx < 0 || endIdx >= size))
            continue;

          std::string ngram;
          for (size_t k = begin + startIdx; k <= begin + endIdx; k++) {
            if (k < 0) {
              ngram += "^<s>";
            } else if (k >= input.GetSize()) {
              ngram += "^</s>";
            } else {
              ngram += "^" + GetWord(input, k);
            }
          }
          outFeatures.push_back(classifier.AddLabelIndependentFeature("sngram" + ngram));
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
      VWFeatureSource::SetParameter(key, value);
    }
  }

  const char* GetFFName() const {
    return "VWFeatureSourceAdjacentNGrams";
  }

private:
  size_t m_windowSize;
  bool m_onlyInclusive;

  static const int DEFAULT_WINDOW_SIZE = 2;
};

}
