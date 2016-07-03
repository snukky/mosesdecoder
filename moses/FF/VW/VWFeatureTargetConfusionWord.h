#pragma once

#include <string>
#include "VWFeatureTarget.h"

namespace Moses
{

class VWFeatureTargetConfusionWord : public VWFeatureTarget
{
public:
  VWFeatureTargetConfusionWord(const std::string &line)
    : VWFeatureTarget(line) {
    ReadParameters();

    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const TargetPhrase &targetPhrase
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    for (size_t i = 0; i < targetPhrase.GetSize(); i++) {
      std::string word = GetWord(targetPhrase, i);
      if (classifier.GetConfusionSet().Has(word))
        outFeatures.push_back(classifier.AddLabelDependentFeature("tcw^" + word));
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureTarget::SetParameter(key, value);
  }

  const char* GetFFName() const {
    return "VWFeatureTargetConfusionWord";
  }
};

}
