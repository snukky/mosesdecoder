#pragma once

#include <string>
#include "VWFeatureBase.h"

namespace Moses
{

// Inherit from this for target-dependent classifier features. They will
// automatically register with the classifier class named VW0 or one or more
// names specified by the used-by=name1,name2,... parameter.
//
// The classifier gets a full list by calling
// VWFeatureBase::GetTargetFeatures(GetScoreProducerDescription())

class VWFeatureTgtSrc : public VWFeatureBase
{
public:
  VWFeatureTgtSrc(const std::string &line)
    : VWFeatureBase(line, vwft_targetSource) {
  }

  // Gets its pure virtual functions from VWFeatureBase

  virtual void operator()(const InputType &input
                          , const Range &sourceRange
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }

  virtual void operator()(const InputType &input
                          , const Range &sourceRange
                          , const CWordInfo &cWordInfo
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }

  virtual void operator()(const InputType &input
                          , const TargetPhrase &targetPhrase
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }


  virtual void operator()(const InputType &input
                          , const Phrase &contextPhrase
                          , const AlignmentInfo &alignmentInfo
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }

  virtual void operator()(const InputType &input
                          , const Range &sourceRange
                          , const CWordPos &sourceCWord
                          , const TargetPhrase &targetPhrase
                          , const CWordPos &targetCWord
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureBase::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureTgtSrc";
  }

protected:
  inline std::string GetWord(const InputType &input, size_t pos) const {
    return input.GetWord(pos).GetString(m_sourceFactors, false);
  }

  inline std::string GetWord(const TargetPhrase &phrase, size_t pos) const {
    return phrase.GetWord(pos).GetString(m_targetFactors, false);
  }
};

}
