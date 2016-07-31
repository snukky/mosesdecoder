#pragma once

#include <string>
#include "VWFeatureTgtSrc.h"
#include "moses/FF/Diffs.h"
#include "moses/Util.h"

namespace Moses
{

class VWFeatureTgtSrcEditOps : public VWFeatureTgtSrc
{
public:
  VWFeatureTgtSrcEditOps(const std::string &line)
    : VWFeatureTgtSrc(line) {
    ReadParameters();

    //if (m_editOps.empty()) {
      //m_editOps.insert('m');
      //m_editOps.insert('d');
      //m_editOps.insert('i');
      //m_editOps.insert('s');
    //}

    VWFeatureBase::UpdateRegister();
  }

  void operator()(const InputType &input
                  , const Range &sourceRange
                  , const TargetPhrase &targetPhrase
                  , Discriminative::Classifier &classifier
                  , Discriminative::FeatureVector &outFeatures) const {

    std::vector<std::string> sourceTokens;
    size_t begin = sourceRange.GetStartPos();
    size_t end   = sourceRange.GetEndPos() + 1;
    while (begin < end) {
      sourceTokens.push_back(GetWord(input, begin++));
    }

    std::vector<std::string> targetTokens;
    for(size_t i = 0; i < targetPhrase.GetSize(); ++i) {
      targetTokens.push_back(GetWord(targetPhrase, i));
    }

    size_t m = 0, d = 0, i = 0, s = 0;
    Diffs diff = CreateDiff(sourceTokens, targetTokens);

    for(int j = 0; j < (int)diff.size(); ++j) {
      if(diff[j] == 'm')
        m++;
      else if(diff[j] == 'd') {
        d++;
        int k = 0;
        while(j - k >= 0 && j + 1 + k < (int)diff.size() &&
              diff[j - k] == 'd' && diff[j + 1 + k] == 'i') {
          d--;
          s++;
          k++;
        }
        j += k;
      } else if(diff[j] == 'i')
        i++;
    }

    //if (m_editOps.find('m') != m_editOps.end())
      outFeatures.push_back(classifier.AddLabelDependentFeature("teop^m^" + SPrint(m)));
    //if (m_editOps.find('d') != m_editOps.end())
      outFeatures.push_back(classifier.AddLabelDependentFeature("teop^d^" + SPrint(d)));
    //if (m_editOps.find('i') != m_editOps.end())
      outFeatures.push_back(classifier.AddLabelDependentFeature("teop^i^" + SPrint(i)));
    //if (m_editOps.find('s') != m_editOps.end())
      outFeatures.push_back(classifier.AddLabelDependentFeature("teop^s^" + SPrint(s)));
  }

  virtual void SetParameter(const std::string& key, const std::string& value) {
    //if (key == "editops") {
      //for (size_t i = 0; i < value.size(); ++i)
        //m_editOps.insert(value[i]);
    //} else {
    VWFeatureTgtSrc::SetParameter(key, value);
  }

  virtual const char* GetFFName() const {
    return "VWFeatureTgtSrcEditOps";
  }

//private:
  //std::set<char> m_editOps;

};

}
