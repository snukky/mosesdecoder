#pragma once

#include <string>
#include "VWFeatureEdit.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureEditPosition : public VWFeatureEdit
{
public:
  VWFeatureEditPosition(const std::string &line)
    : VWFeatureEdit(line) {
    ReadParameters();

    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , const CWordPos& sourceCWord
                  , const TargetPhrase &targetPhrase
                  , const CWordPos& targetCWord
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // nothing to do as no confusion word is found in the input
    if (! sourceCWord.IsSet() || ! targetCWord.IsSet())
      return;

    outFeatures.push_back(classifier.AddLabelDependentFeature("epos^" + SPrint(targetCWord.i)));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureEdit::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureEditPosition";
  }
};

}
