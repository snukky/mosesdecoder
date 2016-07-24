#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureTarget.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureTargetCSetIndicator : public VWFeatureTarget
{
public:
  VWFeatureTargetCSetIndicator(const std::string &line) : VWFeatureTarget(line) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const TargetPhrase &targetPhrase
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    for (size_t i = 0; i < targetPhrase.GetSize(); i++) {
      if (classifier.GetConfusionSet().Has(GetWord(targetPhrase, i))) {
        outFeatures.push_back(classifier.AddLabelDependentFeature("tcsbool^yes"));
        return;
      }
    }
    outFeatures.push_back(classifier.AddLabelDependentFeature("tcsbool^no"));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureTarget::SetParameter(key, value);
  }

  const char* GetFFName() const {
    return "VWFeatureEditBoolIndicator";
  }
};

}
