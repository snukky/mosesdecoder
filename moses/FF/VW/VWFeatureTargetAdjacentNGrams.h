#pragma once

#include <string>
#include "VWFeatureTarget.h"

namespace Moses
{

class VWFeatureTargetAdjacentNGrams : public VWFeatureTarget
{
public:
  VWFeatureTargetAdjacentNGrams(const std::string &line)
    : VWFeatureTarget(line), m_windowSize(DEFAULT_WINDOW_SIZE) {
    ReadParameters();

    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const TargetPhrase &targetPhrase
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    for (size_t i = 0; i < targetPhrase.GetSize(); i++) {
      if (classifier.GetConfusionSet().Has(GetWord(targetPhrase, i))) {
        for (size_t j = 0; j < m_windowSize; j++) {
          int startIdx = i - m_windowSize+1 + j;
          size_t endIdx = startIdx + m_windowSize-1;

          if (startIdx >= 0 && endIdx < targetPhrase.GetSize()) {
            std::string ngram;
            for (size_t k = startIdx; k <= endIdx; k++)
              ngram += "^" + GetWord(targetPhrase, k);

            outFeatures.push_back(classifier.AddLabelDependentFeature("tngram" + ngram));
          }
        }
      }
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    if (key == "size") {
      m_windowSize = Scan<size_t>(value);
    } else {
      VWFeatureTarget::SetParameter(key, value);
    }
  }

  const char* GetFFName() const {
    return "VWFeatureTargetAdjacentNGrams";
  }

private:
  size_t m_windowSize;
  static const int DEFAULT_WINDOW_SIZE = 2;
};

}
