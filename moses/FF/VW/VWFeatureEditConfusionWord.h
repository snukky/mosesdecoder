#pragma once

#include <string>
#include <algorithm>
#include "VWFeatureEdit.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureEditConfusionWord : public VWFeatureEdit
{
public:
  VWFeatureEditConfusionWord(const std::string &line) : VWFeatureEdit(line) {
    ReadParameters();

    // Call this last
    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , const CWordPos &sourceCWord
                  , const TargetPhrase &targetPhrase
                  , const CWordPos &targetCWord
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // nothing to do as no confusion word is found in the input
    if (! targetCWord.IsSet())
      return;

    outFeatures.push_back(classifier.AddLabelDependentFeature("ecw^" +
          (targetCWord.IsNull() ? "<null>" : GetTargetWord(targetPhrase, targetCWord.i))));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureEdit::SetParameter(key, value);
  }

  const char* GetFFName() const {
    return "VWFeatureEditConfusionWord";
  }
};

}
