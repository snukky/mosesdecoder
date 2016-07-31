#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureSource.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureSourceNGrams : public VWFeatureSource
{
public:
  VWFeatureSourceNGrams(const std::string &line)
    : VWFeatureSource(line), m_windowSize(DEFAULT_WINDOW_SIZE), m_onlyInclusive(true) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    size_t beginIdx = m_onlyInclusive
      ? sourceRange.GetStartPos() + (m_windowSize - 1)
      : sourceRange.GetStartPos();
    size_t endIdx = m_onlyInclusive
      ? sourceRange.GetEndPos() + 1
      : sourceRange.GetEndPos() + m_windowSize;

    // no ngram to collect
    if (beginIdx >= endIdx)
      return;

    for (size_t i = beginIdx; i < endIdx; ++i) {
      std::string ngram = "^" + SPrint(i);
      for (int j = i - m_windowSize + 1; j <= (int)i; ++j) {
        if (j < 0)
          ngram += "^<s>";
        else if (j >= (int)input.GetSize())
          ngram += "^</s>";
        else
          ngram += "^" + GetWord(input, j);
      }
      outFeatures.push_back(classifier.AddLabelIndependentFeature("sgram" + ngram));
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
    return "VWFeatureSourceNGrams";
  }

private:
  size_t m_windowSize;
  bool m_onlyInclusive;

  static const int DEFAULT_WINDOW_SIZE = 2;
};

}
