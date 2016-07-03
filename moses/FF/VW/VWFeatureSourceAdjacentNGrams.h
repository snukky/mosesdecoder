#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureSource.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureSourceAdjacentNGrams : public VWFeatureSource
{
public:
  VWFeatureSourceAdjacentNGrams(const std::string &line)
    : VWFeatureSource(line), m_windowSize(DEFAULT_WINDOW_SIZE) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {
    size_t begin = sourceRange.GetStartPos();
    size_t end   = sourceRange.GetEndPos() + 1;
    size_t size  = end - begin;

    for (size_t i = 0; i < size; i++) {
      if (classifier.GetConfusionSet().Has(GetWord(input, begin + i))) {
        for (size_t j = 0; j < m_windowSize; j++) {
          int startIdx = i - m_windowSize+1 + j;
          size_t endIdx = startIdx + m_windowSize-1;

          if (startIdx >= 0 && endIdx < size) {
            std::string ngram;
            for (size_t k = startIdx; k <= endIdx; k++)
              ngram += "^" + GetWord(input, begin + k);

            outFeatures.push_back(classifier.AddLabelIndependentFeature("sngram" + ngram));
          }
        }
      }
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    if (key == "size") {
      m_windowSize = Scan<size_t>(value);
    } else {
      VWFeatureSource::SetParameter(key, value);
    }
  }

  const char* GetFFName() const {
    return "VWFeatureSourceAdjacentNGrams";
  }

private:
  size_t m_windowSize;
  static const int DEFAULT_WINDOW_SIZE = 2;
};

}
