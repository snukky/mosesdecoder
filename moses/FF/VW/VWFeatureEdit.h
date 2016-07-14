#pragma once

#include <string>
#include <boost/foreach.hpp>

#include "VWFeatureBase.h"

#include "moses/InputType.h"
#include "moses/TypeDef.h"
#include "moses/Word.h"
#include "moses/AlignmentInfo.h"
#include "moses/TargetPhrase.h"

namespace Moses
{

// Inherit from this for target-dependent classifier features with source. They
// will automatically register with the classifier class named VW0 or one or
// more names specified by the used-by=name1,name2,... parameter.
//
// The classifier gets a full list by calling
// VWFeatureBase::GetTargetWithSourceFeatures(GetScoreProducerDescription())

class VWFeatureEdit : public VWFeatureBase
{
public:
  VWFeatureEdit(const std::string &line)
    : VWFeatureBase(line, vwft_edit) {
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
                          , const Phrase &contextPhrase
                          , const AlignmentInfo &alignmentInfo
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }

  virtual void operator()(const InputType &input
                          , const TargetPhrase &targetPhrase
                          , Discriminative::Classifier &classifier
                          , Discriminative::FeatureVector &outFeatures) const {
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    VWFeatureBase::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureEdit";
  }

protected:
  inline std::string GetTargetWord(const TargetPhrase &phrase, size_t pos) const {
    return phrase.GetWord(pos).GetString(m_targetFactors, false);
  }

  inline std::string GetSourceWord(const InputType &input, size_t pos) const {
    return input.GetWord(pos).GetString(m_sourceFactors, false);
  }

  // get source words aligned to a particular target word
  std::vector<std::string> GetAlignedSourceWords(const TargetPhrase &phrase
                                                 , const Range &sourceRange
                                                 , const InputType &input
                                                 , size_t idx) const {

    std::set<size_t> alignedToTarget = phrase.GetAlignTerm().GetAlignmentsForTarget(idx);
    std::vector<std::string> out;
    out.reserve(alignedToTarget.size());
    BOOST_FOREACH(size_t srcIdx, alignedToTarget) {
      out.push_back(GetSourceWord(input, srcIdx + sourceRange.GetStartPos()));
    }
    return out;
  }
};

}
