#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureSource.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureSourceCSetIndicator : public VWFeatureSource
{
public:
  VWFeatureSourceCSetIndicator(const std::string &line) : VWFeatureSource(line) {
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

    while (begin < end) {
      if (classifier.GetConfusionSet().Has(GetWord(input, begin++))) {
        outFeatures.push_back(classifier.AddLabelIndependentFeature("scsbool^yes"));
        return;
      }
    }
    outFeatures.push_back(classifier.AddLabelIndependentFeature("scsbool^no"));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureSource::SetParameter(key, value);
  }

  const char* GetFFName() const {
    return "VWFeatureSourceCSetIndicator";
  }
};

}
