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
                  , const TargetPhrase &targetPhrase
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    // only substitutions and insertions are taken into account
    for (size_t i = 0; i < targetPhrase.GetSize(); i++) {
      std::string tWord = GetTargetWord(targetPhrase, i);
      if (classifier.GetConfusionSet().Has(tWord)) {
        std::vector<std::string> sWords = GetAlignedSourceWords(targetPhrase, sourceRange, input, i);
        for (size_t j = 0; j < sWords.size(); ++j) {
          if (classifier.GetConfusionSet().Has(sWords[j])) {
            outFeatures.push_back(classifier.AddLabelDependentFeature("epair^" + sWords[j] + "->" + tWord));
            // take only the first confusion word and do not process further
            break;
          }
        }
        outFeatures.push_back(classifier.AddLabelDependentFeature("epair^" + CW_NULL + "->" + tWord));
      }
    }
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureEdit::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureEditPair";
  }

private:
  // label for a <null> confusion word
  const static std::string CW_NULL;
};

const std::string VWFeatureEditPair::CW_NULL = "NULL";

}
