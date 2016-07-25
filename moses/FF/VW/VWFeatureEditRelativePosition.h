#pragma once

#include <string>
#include "VWFeatureEdit.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureEditRelativePosition : public VWFeatureEdit
{
public:
  VWFeatureEditRelativePosition(const std::string &line)
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

    if (targetCWord.i == 0)
      outFeatures.push_back(classifier.AddLabelDependentFeature("erpos^beg"));
    else if (targetCWord.i == (int)targetPhrase.GetSize() - 1)
      outFeatures.push_back(classifier.AddLabelDependentFeature("erpos^end"));
    else
      outFeatures.push_back(classifier.AddLabelDependentFeature("erpos^mid"));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureEdit::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureEditRelativePosition";
  }
};

}
