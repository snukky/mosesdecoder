#pragma once

#include <string>
#include "VWFeatureEdit.h"

namespace Moses
{

class VWFeatureEditPair : public VWFeatureEdit
{
public:
  VWFeatureEditPair(const std::string &line)
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

    outFeatures.push_back(classifier.AddLabelDependentFeature("epair^" +
          (sourceCWord.IsNull()
            ? "<null>"
            : GetSourceWord(input, sourceRange.GetStartPos() + sourceCWord.i)) +
          "->" +
          (targetCWord.IsNull()
            ? "<null>"
            : GetTargetWord(targetPhrase, targetCWord.i))
        ));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureEdit::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureEditPair";
  }
};

}
