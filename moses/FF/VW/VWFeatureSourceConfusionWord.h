#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureSource.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureSourceConfusionWord : public VWFeatureSource
{
public:
  VWFeatureSourceConfusionWord(const std::string &line)
    : VWFeatureSource(line) {
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

    for (size_t i = 0; i < end - begin; i++) {
      std::string word = GetWord(input, begin + i);
      if (classifier.GetConfusionSet().Has(word))
        outFeatures.push_back(classifier.AddLabelIndependentFeature("scw^" + word));
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureSource::SetParameter(key, value);
  }

  const char* GetFFName() const {
    return "VWFeatureSourceConfusionWord";
  }
};

}
