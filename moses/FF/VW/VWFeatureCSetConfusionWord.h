#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureCSet.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureCSetConfusionWord : public VWFeatureCSet
{
public:
  VWFeatureCSetConfusionWord(const std::string &line) : VWFeatureCSet(line) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , const CWordInfo &cWordInfo
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // nothing to do as no confusion word is found in the input
    if (! cWordInfo.sourcePos.IsSet())
      return;

    const CWordPos& pos = cWordInfo.sourcePos;
    outFeatures.push_back(classifier.AddLabelIndependentFeature("cscw^" +
          (pos.IsNull() ? "<null>" : GetWord(input, sourceRange.GetStartPos() + pos.i))));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureCSet::SetParameter(key, value);
  }

  const char* GetFFName() const {
    return "VWFeatureCSetConfusionWord";
  }
};

}
